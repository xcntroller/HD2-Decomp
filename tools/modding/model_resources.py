"""Read-only model/texture assets over the established resource abstraction.

A selected invalid resource is never replaced. Explicit ordered directory
profiles remain separate from warm runtime caches and exact archive selection.
"""
from dataclasses import dataclass,field,asdict
import hashlib
from pathlib import Path
import struct

from model4ds import decode,read_file,Model
from resource_lookup import ResourceLookup
from material_preview import describe,rgba_preview

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
    channels:int=3
    indices:bytes|None=field(default=None,repr=False)
    native_alpha:bytes|None=field(default=None,repr=False)
    @property
    def retained_bytes(self):return len(self.pixels)+len(self.indices or b'')+len(self.native_alpha or b'')


def decode_bmp(data,keep_aux=False):
    """Verified local BMP inputs: BITMAPINFOHEADER, BI_RGB,8/24/32-bit.

    Source RGB plus optional indices/32-bit fourth bytes. Palette reserved bytes
    are not alpha. A separate established material policy decides alpha usage.
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
    pixels=bytearray();indices=bytearray();alpha=bytearray()
    for row in range(rows):
        at=offset+(row if height<0 else rows-1-row)*stride
        raw=data[at:at+width*(bpp//8)]
        if keep_aux and bpp==8:indices.extend(raw)
        if keep_aux and bpp==32:alpha.extend(raw[3::4])
        if bpp==8:
            if any(i>=len(palette) for i in raw):raise TextureError('BMP index exceeds palette')
            pixels.extend(b''.join(palette[i] for i in raw))
        else:
            channels=bpp//8;rgb=bytearray(width*3)
            rgb[0::3]=raw[2::channels];rgb[1::3]=raw[1::channels];rgb[2::3]=raw[0::channels];pixels.extend(rgb)
    return Image(width,rows,bytes(pixels),{'format':'BMP BI_RGB','dib_size':dib,'bits_per_pixel':bpp,
        'pixel_offset':offset,'row_stride':stride,'top_down_serialized':height<0,'palette_entries':len(palette),
        'alpha':'not decoded; opaque RGB preview','sha256':hashlib.sha256(data).hexdigest(),
        'file_bytes':len(data),'unconsumed_suffix':len(data)-end},indices=bytes(indices) if indices else None,native_alpha=bytes(alpha) if alpha else None)


@dataclass
class ModelAsset:
    model:Model
    textures:list[dict]=field(default_factory=list)
    images:dict[int,Image]=field(default_factory=dict,repr=False)
    protected_roots:list[Path]=field(default_factory=list,repr=False)
    material_preview:dict[int,dict]=field(default_factory=dict)
    def metadata(self):
        value=self.model.metadata();value['textures']=self.textures;value['material_preview']=self.material_preview
        value['rendering']={'mode':'serialized geometry / supported RGB/RGBA preview',
            'original_renderer_reproduction':False,'fallback_material':'explicit neutral gray',
            'textures_applied_to_materials':sorted(self.images),
            'limitations':['Only established alpha sources; preview lighting/blending/cutoff differ from original. No lightmaps, animation or skin deformation.',
                           'Only verified mesh layouts are displayed; unsupported data is never replaced with generated geometry.',
                           'Texture cache-directory matching is labelled separately from exact archive selection.',
                           'All supported frames are shown; original visibility/LOD/effect policy is not simulated.']}
        return value


class ModelResources:
    def __init__(self,store,texture_cache=None,texture_limit=MAX_TEXTURE_BYTES,texture_dependencies=MAX_TEXTURES,lookup=None):
        self.store=store;self.texture_cache=texture_cache;self.texture_limit=texture_limit
        self.texture_dependencies=texture_dependencies
        self.lookup=lookup or ResourceLookup(store)
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
        decoded=self.texture_cache if self.texture_cache is not None else {}
        def admit(key,image):
            if sum(i.retained_bytes for i in decoded.values())+image.retained_bytes>self.texture_limit:
                raise TextureError('Shared decoded texture/alpha budget exceeded','unsupported')
            decoded[key]=image;return image
        def source(path):
            if path not in decoded:
                result=self.store.read_result(path)
                image=decode_bmp(result.data,keep_aux=True)
                image.metadata['archive_decoding']=result.decoding
                admit(path,image)
            return decoded[path]
        for material in asset.model.materials:
            policy=describe(material);state=policy.metadata();asset.material_preview[material.index]=state
            sources={};deps={}
            #1002BBE0 reads/requests extra first, then primary and optional opacity.
            for role,name in (('extra',material.extra_texture),('primary',material.primary_texture),('secondary',material.secondary_texture)):
                if not name:continue
                dependency={'material':material.index,'role':role,'name':name,'status':'unsupported',
                    'rendering_status':'fallback','evidence':policy.evidence}
                asset.textures.append(dependency);deps[role]=dependency
                if len(asset.textures)>self.texture_dependencies:
                    dependency['message']='Tool texture dependency/decode budget reached';continue
                try:
                    selection=self.lookup.texture(name);dependency.update(lookup=selection,relationship=selection['certainty'])
                    path=selection['path']
                    if path is None:dependency.update(status='missing',message=selection['message']);continue
                    dependency.update(selected='resource/'+path,resolution=selection['resolution'])
                    if material.sequence or role=='extra' or role=='secondary' and not policy.opacity_map:
                        dependency.update(status='resolved_uninterpreted',message='Sequence/extra/unsupported secondary interpretation is not rendered');continue
                    image=source(path);sources[role]=(path,image)
                    dependency.update(status='decoded',rendering_status='RGB_source' if role=='primary' else 'alpha_source',
                        image=image.metadata,width=image.width,height=image.height)
                except (ValueError,OSError) as exc:
                    status=getattr(exc,'status',None)
                    if status is None:status='invalid' if getattr(exc,'kind','') in ('invalid','integrity_error','bounds_error') else 'unavailable'
                    dependency.update(status=status,message=str(exc))
            if 'primary' not in sources:
                state.update(rendering_status='color_only_preview' if not material.primary_texture and policy.color is not None else 'fallback_material',
                    alpha_status='not_requested' if not material.primary_texture else 'unavailable_primary');continue
            path,image=sources['primary'];secondary=sources.get('secondary')
            try:
                if policy.opacity_map and not secondary:raise TextureError('Required opacity map unavailable; preview retains RGB only','unsupported')
                key=(path,policy.color_key,policy.intrinsic_alpha,secondary[0] if secondary else None)
                if key in decoded:preview=decoded[key]
                else:
                    pixels,channels,mode=rgba_preview(image,policy,secondary[1] if secondary else None)
                    preview=image if channels==3 else admit(key,Image(image.width,image.height,pixels,{**image.metadata,'alpha':mode},channels))
                asset.images[material.index]=preview
                state.update(rendering_status='RGBA_preview' if preview.channels==4 else 'RGB_preview',alpha_status=preview.metadata['alpha'] if preview.channels==4 else 'opaque')
                deps['primary']['rendering_status']=state['rendering_status']
            except (ValueError,OSError) as exc:
                asset.images[material.index]=image
                state.update(rendering_status='RGB_fallback_alpha_unsupported',alpha_status='unsupported',diagnostic=str(exc))
                deps['primary']['rendering_status']='RGB_fallback_alpha_unsupported'
        return asset


def open_local(path):
    path=Path(path).resolve();asset=ModelAsset(read_file(path),protected_roots=[path.parent])
    # A local file has no implicit installation profile or texture search path.
    for material in asset.model.materials:
        if material.primary_texture:asset.textures.append({'material':material.index,'role':'primary','name':material.primary_texture,
            'status':'unavailable','rendering_status':'fallback','message':'Open through an installation to resolve textures'})
    return asset
