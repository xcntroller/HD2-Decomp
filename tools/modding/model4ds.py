"""Bounded read-only H&D2 version-0x29 model decoding, with no GUI dependencies.

Only length-established records are traversed. Unknown layouts terminate parsing
at a retained opaque suffix; no heuristic resynchronization is attempted.
"""
from __future__ import annotations
from array import array
from dataclasses import dataclass,field,asdict
import hashlib
import json
import math
from pathlib import Path
import struct
import sys

from model_math import local_matrix,multiply,transform,bounds

MAX_BYTES=64*1024*1024
MAX_FRAMES=6000  # Tool bound, unlike the native first-table limit of 6000.
MAX_VERTICES=1_000_000
MAX_TRIANGLES=2_000_000
EVIDENCE='LS3DF 1005D970/1001EC40/10038410/1005DDD0; MODEL4DS.md'


class ModelError(ValueError):
    def __init__(self,message,offset,status='invalid'):
        super().__init__(message);self.offset=offset;self.status=status


class Cursor:
    def __init__(self,data):self.data=memoryview(data);self.position=0
    def take(self,size):
        if size<0 or size>len(self.data)-self.position:raise ModelError('Truncated model field',self.position)
        start=self.position;self.position+=size;return self.data[start:self.position]
    def unpack(self,fmt):
        values=struct.unpack('<'+fmt,self.take(struct.calcsize('<'+fmt)))
        return values[0] if len(values)==1 else values
    def name(self):return bytes(self.take(self.unpack('B'))).decode('latin-1')
    def floats(self,count):
        values=self.unpack(str(count)+'f')
        if count==1:values=(values,)
        if not all(math.isfinite(v) and abs(v)<=1e20 for v in values):raise ModelError('Nonfinite/excessive float',self.position-count*4)
        return tuple(values)


@dataclass
class Opaque:
    offset:int
    size:int
    reason:str
    sha256:str
    evidence:str
    raw_hex:str|None=None


@dataclass
class Material:
    index:int
    offset:int
    end:int
    flags:int
    payload_hex:str
    parameter_floats:tuple
    extra_texture:str|None
    primary_texture:str
    secondary_texture:str|None
    sequence:dict|None
    evidence:str='LS3DF 1002BBE0/1002BE30'


@dataclass
class Group:
    offset:int
    end:int
    triangle_count:int
    material:int
    indices:array=field(repr=False)
    @property
    def indices_offset(self):return self.offset+2
    def metadata(self):
        return {'offset':self.offset,'end':self.end,'triangle_count':self.triangle_count,
                'indices_offset':self.indices_offset,'material':self.material,'index_type':'uint16_le',
                'triangle_samples':[list(self.indices[i:i+3]) for i in range(0,min(len(self.indices),24),3)]}


@dataclass(frozen=True)
class Vertex:
    index:int
    offset:int
    position:tuple
    normal:tuple
    uv:tuple


@dataclass
class LOD:
    index:int
    offset:int
    end:int
    metric:float
    stream_flags:int
    vertex_count:int
    vertices_offset:int
    vertices:array=field(repr=False)
    groups:list[Group]=field(default_factory=list)
    bounds:tuple|None=None
    def vertex(self,index):
        if not 0<=index<self.vertex_count:raise IndexError(index)
        values=self.vertices[index*8:index*8+8]
        return Vertex(index,self.vertices_offset+index*32,tuple(values[:3]),tuple(values[3:6]),tuple(values[6:8]))
    def metadata(self):
        return {'index':self.index,'offset':self.offset,'end':self.end,'metric':self.metric,
                'stream_flags':self.stream_flags,'vertex_count':self.vertex_count,
                'vertices_offset':self.vertices_offset,'vertex_stride':32,'vertex_fields':['position:float3','normal:float3','uv:float2'],
                'groups':[g.metadata() for g in self.groups],'local_bounds':self.bounds,
                'vertex_samples':[asdict(self.vertex(i)) for i in range(min(self.vertex_count,8))],
                'attribute_evidence':{'stride_position':'native1005DDD0/10030F30',
                    'normal_uv':'corroborated preview interpretation: geometric normals and real texture mapping; original graphics declarations not reconstructed'}}


@dataclass
class Geometry:
    identity:str
    offset:int
    end:int
    lods:list[LOD]=field(default_factory=list)
    def metadata(self):return {'identity':self.identity,'offset':self.offset,'end':self.end,'lods':[l.metadata() for l in self.lods]}


@dataclass
class Frame:
    index:int
    identity:str
    offset:int
    end:int
    kind:int
    visual_type:int|None
    visual_flags:int|None
    parent:int  # Original one-based frame reference; zero is model root.
    name:str
    properties:str
    position:tuple
    quaternion:tuple
    scale:tuple
    unknown_float:float
    serialized_flags:int
    local_matrix:tuple
    world_matrix:tuple
    geometry:Geometry|None=None
    geometry_reference:int=0
    status:str='decoded'
    unknown:list[Opaque]=field(default_factory=list)
    world_bounds:tuple|None=None
    def metadata(self):
        result={k:v for k,v in vars(self).items() if k not in ('geometry','unknown')}
        result['geometry_identity']=self.geometry.identity if self.geometry else None
        result['unknown']=[asdict(u) for u in self.unknown]
        return result


@dataclass
class Model:
    identity:str
    size:int
    sha256:str
    status:str='unsupported'
    header:dict=field(default_factory=dict)
    materials:list[Material]=field(default_factory=list)
    frames:list[Frame]=field(default_factory=list)
    geometries:list[Geometry]=field(default_factory=list)
    unknown:list[Opaque]=field(default_factory=list)
    diagnostics:list[dict]=field(default_factory=list)
    bounds:tuple|None=None
    provenance:dict|None=None
    limitations:list[str]=field(default_factory=lambda:[
        'Serialized model inspection; no original runtime execution or full format support claim.',
        'Material parameter names/blending/effects, animated sequences and skin deformation are not implemented.',
        'Standalone model view uses LOD0 by default; metrics remain uninterpreted. Mission composition has separate SPATIAL.md coverage.',
        'Unsupported records stop parsing; an opaque suffix is retained without guessed resynchronization.'])
    def metadata(self):
        return {'schema_version':1,'format':'H&D2 4DS','identity':self.identity,'size':self.size,'sha256':self.sha256,
                'status':self.status,'header':self.header,'materials':[asdict(m) for m in self.materials],
                'frames':[f.metadata() for f in self.frames],'geometries':[g.metadata() for g in self.geometries],
                'unknown':[asdict(u) for u in self.unknown],'diagnostics':self.diagnostics,'bounds':self.bounds,
                'bounds_scope':'decoded LOD0 geometry only; partial models omit unknown frames','provenance':self.provenance,'statistics':self.statistics(),'limitations':self.limitations,'evidence':EVIDENCE}
    def statistics(self):
        lods=[l for g in self.geometries for l in g.lods]
        return {'declared_frames':self.header.get('frame_count'),'decoded_frames':len(self.frames),
                'materials':len(self.materials),'unique_geometries':len(self.geometries),'lods':len(lods),
                'stored_vertices':sum(l.vertex_count for l in lods),
                'stored_triangles':sum(g.triangle_count for l in lods for g in l.groups),
                'renderable_instances':sum(bool(f.geometry and any(l.vertex_count and any(g.triangle_count for g in l.groups) for l in f.geometry.lods)) for f in self.frames),
                'lod0_instance_triangles':sum(sum(g.triangle_count for g in f.geometry.lods[0].groups) for f in self.frames if f.geometry and f.geometry.lods)}
    def iter_json(self):
        yield from json.JSONEncoder(sort_keys=True,indent=2,ensure_ascii=True,allow_nan=False).iterencode(self.metadata())
        yield '\n'


def opaque(cursor,start,end,reason,evidence):
    return Opaque(start,end-start,reason,hashlib.sha256(cursor.data[start:end]).hexdigest(),evidence,bytes(cursor.data[start:end]).hex() if end-start<=96 else None)


def decode(data:bytes,identity='model.4ds',provenance=None):
    model=Model(str(identity),len(data),hashlib.sha256(data).hexdigest(),provenance=provenance)
    c=Cursor(data);vertices=triangles=0
    def geometry():
        nonlocal vertices,triangles
        offset=c.position;reference=c.unpack('H')
        if reference:
            if reference>len(model.frames) or model.frames[reference-1].geometry is None or not model.frames[reference-1].geometry.lods:
                raise ModelError('Invalid/non-mesh shared geometry reference',offset)
            return model.frames[reference-1].geometry,reference
        result=Geometry(f'{identity}@0x{offset:x}',offset,offset);count=c.unpack('B')
        if count>10:raise ModelError('LOD count exceeds native maximum10',c.position-1)
        for index in range(count):
            start=c.position;metric=c.floats(1)[0];flags=c.unpack('I');nv=c.unpack('H')
            if nv==65535:raise ModelError('Special reused-vertex UV stream is not decoded',c.position-2,'unsupported')
            if flags:raise ModelError('Additional vertex stream layout is unsupported',start+4,'unsupported')
            vertices+=nv
            if vertices>MAX_VERTICES:raise ModelError('Tool vertex budget exceeded',c.position,'unsupported')
            at=c.position;raw=c.take(nv*32);buffer=array('f');buffer.frombytes(raw)
            if sys.byteorder!='little':buffer.byteswap()
            if not all(math.isfinite(v) and abs(v)<=1e20 for v in buffer):raise ModelError('Nonfinite/excessive vertex field',at)
            lod=LOD(index,start,c.position,metric,flags,nv,at,buffer)
            lod.bounds=bounds(tuple(buffer[v:v+3]) for v in range(0,len(buffer),8))
            for _ in range(c.unpack('B')):
                group_at=c.position;nt=c.unpack('H');triangles+=nt
                if triangles>MAX_TRIANGLES:raise ModelError('Tool triangle budget exceeded',group_at,'unsupported')
                indices=array('H');indices.frombytes(c.take(nt*6))
                if sys.byteorder!='little':indices.byteswap()
                if any(i>=nv for i in indices):raise ModelError('Triangle index exceeds vertex count',group_at)
                material=c.unpack('H')
                if material>len(model.materials):raise ModelError('Material reference exceeds first table',c.position-2)
                lod.groups.append(Group(group_at,c.position,nt,material,indices))
            lod.end=c.position;result.lods.append(lod)
        result.end=c.position;model.geometries.append(result)
        return result,0
    record_start=0
    try:
        if len(data)>MAX_BYTES:raise ModelError('Model exceeds64MiB inspection limit',0,'unsupported')
        if bytes(c.take(4))!=b'4DS\0':raise ModelError('Invalid 4DS signature',0)
        version=c.unpack('H');model.header['version']=version
        if version!=0x29:raise ModelError('Unsupported 4DS version (requires0x29)',4,'unsupported')
        model.header['opaque_metadata_hex']=bytes(c.take(8)).hex();count=c.unpack('H')
        model.header['first_table_count']=count
        if count>6000:raise ModelError('First table exceeds native6000 limit',14)
        for index in range(count):
            record_start=c.position;flags=c.unpack('I');payload=bytes(c.take(76 if flags&0x80000 else 72))
            # Preserve float bit patterns in payload_hex; nonfinite opaque material
            # parameters are not converted to nonstandard JSON numbers.
            values=struct.unpack('<'+str(len(payload)//4)+'f',payload)
            values=tuple(v if math.isfinite(v) else None for v in values)
            extra=c.name() if flags&0x80000 else None;primary=c.name();secondary=None
            if primary and flags&0x40000000 and not flags&0xa1000000:secondary=c.name()
            sequence=None
            if flags&0x4000000:sequence={'counts':list(c.take(2)),'opaque_parameters_hex':bytes(c.take(16)).hex()}
            model.materials.append(Material(index+1,record_start,c.position,flags,payload.hex(),values,extra,primary,secondary,sequence))
        model.header['first_table_end']=c.position;record_start=c.position;count=c.unpack('H');model.header['frame_count']=count
        if count>MAX_FRAMES:raise ModelError('Tool frame count limit exceeded',c.position-2,'unsupported')
        for index in range(count):
            record_start=c.position;kind=c.unpack('B');sub=flags=None
            if kind==1:
                sub=c.unpack('B');flags=c.unpack('H')
                if sub not in (0,1,2):raise ModelError(f'Unsupported visual subtype{sub}',record_start,'unsupported')
            elif kind not in (5,6,10,11,12):raise ModelError(f'Unsupported frame kind{kind}',record_start,'unsupported')
            parent=c.unpack('H')
            if parent>index:raise ModelError('Parent must refer to an earlier one-based frame',c.position-2)
            position=c.floats(3);quat=c.floats(4);scale=c.floats(3);unknown=c.floats(1)[0];bits=c.unpack('B')
            name=c.name();properties=c.name();local=local_matrix(position,quat,scale)
            world=multiply(local,model.frames[parent-1].world_matrix) if parent else local
            if not all(math.isfinite(v) and abs(v)<=1e20 for v in world):raise ModelError('Unsafe hierarchical transform',record_start)
            frame=Frame(index+1,f'{identity}@0x{record_start:x}',record_start,c.position,kind,sub,flags,parent,name,properties,position,quat,scale,unknown,bits,local,world)
            if kind==1:
                frame.geometry,frame.geometry_reference=geometry()
                if sub==1:frame.status='partial';model.diagnostics.append({'severity':'warning','offset':record_start,'code':'visual1_preview','message':'Visual subtype1 rendered as serialized mesh; subtype-specific runtime behavior is unknown'})
                if sub==2 and frame.geometry and frame.geometry.lods:
                    # 10053900 consumes common mesh, byte bone count, opaque32,
                    # per-bone byte and96-byte records, then per-LOD counted words.
                    start=c.position;bones=c.unpack('B')
                    if bones:
                        c.take(32+bones+bones*96)
                        for lod in frame.geometry.lods if frame.geometry else []:
                            used=c.unpack('I')
                            if used>lod.vertex_count:raise ModelError('Skin count exceeds LOD vertices',c.position-4)
                            c.take(used*2)
                    frame.unknown.append(opaque(c,start,c.position,'Skin binding records; undeformed serialized geometry only','LS3DF 10053900'))
                    frame.status='partial'
                    model.diagnostics.append({'severity':'warning','offset':start,'code':'skin_undeformed','message':'Skin bindings retained opaque; no skin deformation or posed-character claim'})
            elif kind==5:
                # 1004E250/100431A0: exact sector/portal extents. Volumes are
                # not environment meshes; retaining them unlocks later visuals.
                start=c.position;header=bytes(c.take(48))
                nv,nt=struct.unpack_from('<II',header,8)
                if nv>MAX_VERTICES or nt>MAX_TRIANGLES:raise ModelError('Tool sector count budget exceeded',start,'unsupported')
                c.take(nv*16)
                at=c.position;raw=c.take(nt*6)
                if any(i[0]>=nv for i in struct.iter_unpack('<H',raw)):raise ModelError('Sector index exceeds vertex count',at)
                portals=c.unpack('B')
                for _ in range(portals):
                    count=c.unpack('B')
                    if count>23:raise ModelError('Tool portal point buffer limit exceeded',c.position-1,'unsupported')
                    c.take((count+2)*16)
                frame.unknown.append(opaque(c,start,c.position,f'Sector header/volume: {nv} vertices,{nt} triangles,{portals} portals; not rendered','LS3DF 1004E250/100431A0'))
                frame.status='partial'
            elif kind==12:
                # 1003A960: opaque occlusion volume, not a visible terrain mesh.
                start=c.position;nv,nt=c.unpack('II')
                if nv>MAX_VERTICES or nt>MAX_TRIANGLES:raise ModelError('Tool occluder count budget exceeded',start,'unsupported')
                c.take(nv*16);at=c.position;raw=c.take(nt*6)
                if any(i[0]>=nv for i in struct.iter_unpack('<H',raw)):raise ModelError('Occluder index exceeds vertex count',at)
                frame.unknown.append(opaque(c,start,c.position,'Occluder volume retained; not rendered','LS3DF 1003A960'));frame.status='partial'
            elif kind==6:
                start=c.position;c.take(32);frame.unknown.append(opaque(c,start,c.position,'Dummy payload,8 raw floats','LS3DF 1001C060'));frame.status='partial'
            elif kind==10:
                start=c.position;c.take(4);frame.unknown.append(opaque(c,start,c.position,'Joint payload DWORD','LS3DF 10020BA0'));frame.status='partial'
            # kind11 uses the same common reader with no extra serialized body.
            if frame.geometry and frame.geometry.lods:
                buffer=frame.geometry.lods[0].vertices
                frame.world_bounds=bounds(transform(world,tuple(buffer[v:v+3])) for v in range(0,len(buffer),8))
            frame.end=c.position;model.frames.append(frame)
        record_start=c.position;model.header['trailer_byte']=c.unpack('B');model.header['parsed_end']=c.position
        if c.position<len(data):
            model.unknown.append(opaque(c,c.position,len(data),'Unconsumed model suffix','LS3DF 1005D970'));model.status='partial'
        else:model.status='partial' if any(f.status=='partial' for f in model.frames) or any(m.sequence for m in model.materials) else 'decoded'
    except (ModelError,ValueError) as exc:
        offset=getattr(exc,'offset',c.position);status=getattr(exc,'status','invalid')
        model.status='partial' if status=='unsupported' and model.frames else status
        model.diagnostics.append({'severity':'error' if status=='invalid' else 'warning','offset':offset,'code':status,'message':str(exc),'evidence':EVIDENCE})
        model.unknown.append(opaque(c,record_start,len(data),'Parsing stopped: '+str(exc),EVIDENCE))
        # Malformed input never publishes drawable products; safely parsed
        # metadata remains available for inspection.
        if status=='invalid':
            for frame in model.frames:frame.geometry=None;frame.world_bounds=None
            model.geometries=[]
    model.bounds=bounds(point for frame in model.frames if frame.world_bounds for point in frame.world_bounds)
    return model


def read_file(path):
    path=Path(path)
    with path.open('rb') as stream:data=stream.read(MAX_BYTES+1)
    return decode(data,str(path.resolve()))


def main():
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('model',type=Path);args=parser.parse_args()
    try:
        model=read_file(args.model);sys.stdout.writelines(model.iter_json())
        return 1 if model.status in ('invalid','unsupported') else 0
    except OSError as exc:parser.exit(2,f'{exc}\n')


if __name__=='__main__':raise SystemExit(main())
