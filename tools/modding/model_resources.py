"""Read-only model/texture assets over the established resource abstraction.

A selected invalid resource is never replaced. Engine cache lookup remains
separate from exact virtual paths and registered-root name candidates.
"""
from dataclasses import dataclass,field,asdict
import hashlib
from pathlib import Path
import struct

from model4ds import decode,read_file,Model

MAX_TEXTURE_BYTES=32*1024*1024
MAX_TEXTURE_PIXELS=8_000_000
MAX_TEXTURES=64


class TextureError(ValueError):
    def __init__(self,message,status='invalid'):
        super().__init__(message);self.status=status


@dataclass
class Image:
    width:int
    height:int
    pixels:bytes=field(repr=False)
    metadata:dict=field(default_factory=dict)


def decode_bmp(data):
    """Verified local BMP inputs: BITMAPINFOHEADER, BI_RGB,8/24/32-bit.

    RGB preview only. Palette reserved bytes/BGRx padding are not inferred alpha.
    The returned first row is the top row, matching serialized UV v=0 addressing.
    """
    if len(data)<2:raise TextureError('Truncated image signature')
    if data[:2]!=b'BM':raise TextureError('Only BMP RGB preview is currently supported','unsupported')
    if len(data)<54:raise TextureError('Truncated BMP header')
    file_size,_,_,offset=struct.unpack_from('<IHHI',data,2)
    dib,width,height,planes,bpp,compression,image_size,xppm,yppm,used,important=struct.unpack_from('<IiiHHIIiiII',data,14)
    if dib!=40 or compression!=0 or bpp not in (8,24,32):raise TextureError('Unsupported BMP header/compression/pixel format','unsupported')
    if planes!=1 or width<=0 or not height:raise TextureError('Invalid BMP dimensions or planes')
    if width>4096 or abs(height)>4096:raise TextureError('Texture dimension limit4096 exceeded','unsupported')
    rows=abs(height)
    if width*rows>MAX_TEXTURE_PIXELS:raise TextureError('Texture pixel budget exceeded','unsupported')
    stride=((width*bpp+31)//32)*4;end=offset+stride*rows
    palette=[]
    if bpp==8:
        count=used or 256
        if count>256 or 54+4*count>offset:raise TextureError('Invalid BMP palette bounds')
        for n in range(count):
            if 54+4*n+4>len(data):raise TextureError('Truncated BMP palette')
            b,g,r,_=data[54+4*n:58+4*n];palette.append(bytes((r,g,b)))
    if offset<54 or end>len(data) or file_size and (file_size>end and file_size>len(data) or file_size<end):raise TextureError('BMP pixel/file bounds invalid')
    pixels=bytearray()
    for row in range(rows):
        at=offset+(row if height<0 else rows-1-row)*stride
        raw=data[at:at+width*(bpp//8)]
        if bpp==8:
            if any(i>=len(palette) for i in raw):raise TextureError('BMP index exceeds palette')
            pixels.extend(b''.join(palette[i] for i in raw))
        else:
            channels=bpp//8;rgb=bytearray(width*3)
            rgb[0::3]=raw[2::channels];rgb[1::3]=raw[1::channels];rgb[2::3]=raw[0::channels];pixels.extend(rgb)
    return Image(width,rows,bytes(pixels),{'format':'BMP BI_RGB','dib_size':dib,'bits_per_pixel':bpp,
        'pixel_offset':offset,'row_stride':stride,'top_down_serialized':height<0,'palette_entries':len(palette),
        'alpha':'not decoded; opaque RGB preview','sha256':hashlib.sha256(data).hexdigest(),
        'file_bytes':len(data),'unconsumed_suffix':len(data)-end})


@dataclass
class ModelAsset:
    model:Model
    textures:list[dict]=field(default_factory=list)
    images:dict[int,Image]=field(default_factory=dict,repr=False)
    protected_roots:list[Path]=field(default_factory=list,repr=False)
    def metadata(self):
        value=self.model.metadata();value['textures']=self.textures
        value['rendering']={'mode':'serialized geometry / RGB texture preview',
            'original_renderer_reproduction':False,'fallback_material':'explicit neutral gray',
            'textures_applied_to_materials':sorted(self.images),
            'limitations':['No original lighting/blending/alpha/shaders/animation or skin deformation.',
                           'Only verified mesh layouts are displayed; unsupported data is never replaced with generated geometry.',
                           'Texture cache-directory matching is labelled separately from exact archive selection.',
                           'All supported frames are shown; original visibility/LOD/effect policy is not simulated.']}
        return value


class ModelResources:
    def __init__(self,store,texture_cache=None,texture_limit=MAX_TEXTURE_BYTES,texture_dependencies=MAX_TEXTURES):
        self.store=store;self.texture_cache=texture_cache;self.texture_limit=texture_limit
        self.texture_dependencies=texture_dependencies
    def open(self,name,textures=True):
        from resource_store import virtual_name
        name=virtual_name(name)
        if not name.endswith('.4ds'):raise ValueError('Select an exact .4ds resource')
        resolution=self.store.resolution(name)
        result=self.store.read_result(name)
        asset=ModelAsset(decode(result.data,'resource/'+name,{'resolution':asdict(resolution),'decoding':result.decoding}),protected_roots=[self.store.root])
        if textures and asset.model.status!='invalid':self.textures(asset)
        return asset

    def textures(self,asset):
        from resource_store import virtual_name
        decoded=self.texture_cache if self.texture_cache is not None else {}
        pixel_bytes=sum(len(image.pixels) for image in decoded.values())
        for material in asset.model.materials:
            for role,name in (('primary',material.primary_texture),('secondary',material.secondary_texture),('extra',material.extra_texture)):
                if not name:continue
                dependency={'material':material.index,'role':role,'name':name,'status':'unsupported',
                            'rendering_status':'fallback','evidence':'LS3DF 1002BBE0/1002BE30;006DBE60 registered graphics roots'}
                asset.textures.append(dependency)
                if len(asset.textures)>self.texture_dependencies:
                    dependency['message']='Tool texture dependency/decode budget reached';continue
                try:
                    key=virtual_name(name)
                    if '/' in key:
                        candidates=[key] if key.startswith(('maps/','maps_c/','maps_u/')) else []
                        dependency['relationship']='serialized_virtual_path'
                    else:
                        candidates=[root+'/'+key for root in ('maps','maps_c','maps_u')]
                        dependency['relationship']='registered_root_name_candidate'
                    matches=[p for p in candidates if self.store.resolution(p).status!='missing']
                    dependency['candidates']=matches
                    if len(matches)!=1:
                        dependency.update(status='missing' if not matches else 'ambiguous',message='No unique texture resource; engine cache search is not guessed');continue
                    path=matches[0];resolution=self.store.resolution(path)
                    dependency.update(selected='resource/'+path,resolution=asdict(resolution))
                    # Secondary/extra maps and sequence playback have no verified
                    # preview interpretation, so do not silently compose them.
                    if role!='primary' or material.sequence:
                        dependency.update(status='resolved_uninterpreted',message='Secondary/extra/sequence mapping is not rendered');continue
                    if path not in decoded:
                        result=self.store.read_result(path);image=decode_bmp(result.data)
                        if pixel_bytes+len(image.pixels)>self.texture_limit:
                            raise TextureError('Total decoded texture budget exceeded','unsupported')
                        decoded[path]=image;pixel_bytes+=len(image.pixels)
                        dependency['archive_decoding']=result.decoding
                    image=decoded[path]
                    dependency.update(status='decoded',rendering_status='RGB_preview',image=image.metadata,width=image.width,height=image.height)
                    asset.images[material.index]=image
                except (ValueError,OSError) as exc:
                    status=getattr(exc,'status',None)
                    if status is None:
                        kind=getattr(exc,'kind','unavailable')
                        status='invalid' if kind in ('invalid','integrity_error','bounds_error') else 'unavailable'
                    dependency.update(status=status,message=str(exc))
        return asset


def open_local(path):
    path=Path(path).resolve();asset=ModelAsset(read_file(path),protected_roots=[path.parent])
    # A local file has no implicit installation profile or texture search path.
    for material in asset.model.materials:
        if material.primary_texture:asset.textures.append({'material':material.index,'role':'primary','name':material.primary_texture,
            'status':'unavailable','rendering_status':'fallback','message':'Open through an installation to resolve textures'})
    return asset
