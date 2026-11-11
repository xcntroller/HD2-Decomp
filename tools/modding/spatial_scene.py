"""Read-only serialized mission composition, independent of Tk/OpenGL.

Actual Scene2 order and source identities are retained. Unknown parents/cache
paths never become guessed world transforms. This is a cold serialized snapshot,
not actor initialization, script execution or a live engine scene.
"""
from __future__ import annotations
from collections import Counter,defaultdict
from dataclasses import dataclass,field,replace
from types import SimpleNamespace
import json
import math
from pathlib import Path

from actor_fields import ACTOR_NAMES
from model4ds import Geometry
from model_math import IDENTITY,local_matrix,multiply,transform,bounds
from model_resources import ModelResources
from mission import walk_records
from resource_store import virtual_name

MAX_NODES=50000
MAX_ASSETS=192
GEOMETRY_BUDGET=96*1024*1024
TEXTURE_BUDGET=48*1024*1024
EVIDENCE='006CD940/006D1873..006D1987;1001D680;1001E390;SPATIAL.md'


def freeze_session(session):
    """Capture on the GUI thread after the existing Explorer worker is idle.

    Parsed records are immutable inspection products. Reference resolution lists
    are copied because a later Explorer inspection may relink them in place.
    """
    files=tuple(replace(entry,references=[replace(ref,targets=list(ref.targets)) for ref in entry.references]) for entry in session.report.files)
    graph={key:tuple(value) for key,value in session.report.graph.items()}
    return SimpleNamespace(store=session.store,report=SimpleNamespace(files=files,mission_name=session.report.mission_name,graph=graph))


@dataclass
class SpatialNode:
    identity:str
    name:str
    kind:int
    source:str
    offset:int|None
    category:str
    visual_type:int|None=None
    model_frame:str|None=None
    parent:str|None=None
    parent_status:str='root'
    world_status:str='unresolved'
    serialized_name:str|None=None
    automatic_candidates:list[str]=field(default_factory=list)
    sector_transform_changed:bool=False
    requested_parent:str|None=None
    position:tuple=(0.,0.,0.)
    quaternion:tuple=(0.,0.,0.,1.)  # Always engine XYZW; raw WXYZ lives in records.
    scale:tuple=(1.,1.,1.)
    local:tuple=IDENTITY
    world:tuple|None=None
    fallback:tuple|None=None
    box:tuple|None=None
    visible:bool=True
    display_visible:bool=True
    flags:int=0
    status:str='serialized'
    records:list[str]=field(default_factory=list)
    actors:list[dict]=field(default_factory=list)
    links:list[dict]=field(default_factory=list)
    diagnostics:list[str]=field(default_factory=list)
    uninterpreted_records:list[dict]=field(default_factory=list)
    model_resource:str|None=None
    model_resolution:dict|None=None
    model_owner:str|None=None
    asset:str|None=None
    geometry:Geometry|None=field(default=None,repr=False)
    inspection_source:str|None=None
    inspection_file:str|None=None

    def metadata(self):
        value={k:v for k,v in vars(self).items() if k!='geometry'}
        value['geometry_identity']=self.geometry.identity if self.geometry else None
        return value


def scene_quaternion(raw):
    # 006CDE38..006CDEC8 reorders WXYZ to XYZW;0041D180 normalizes it.
    w,x,y,z=raw;n=math.sqrt(x*x+y*y+z*z+w*w)
    if not math.isfinite(n) or n<1e-12:raise ValueError('Unsupported zero/nonfinite Scene2 quaternion')
    return tuple(v/n for v in (x,y,z,w))


def transformed_box(box,matrix):
    if box is None:return None
    return bounds(transform(matrix,tuple(box[(mask>>k)&1][k] for k in range(3))) for mask in range(8))


class SpatialScene:
    def __init__(self,session,progress=None,cancel=None):
        if not hasattr(session,'store'):raise ValueError('Spatial inspection requires an installation-aware mission session')
        self.session=session;self.store=session.store;self.mission=session.report.mission_name
        self.nodes={};self.order=[];self.names=defaultdict(list);self.all_names=defaultdict(list)
        self.assets={};self.asset_failures={};self.texture_cache={};self.geometry_bytes=0
        self.resources=ModelResources(self.store,self.texture_cache,TEXTURE_BUDGET,512)
        self.documents=[];self.diagnostics=[];self.observations=[];self.source_nodes=defaultdict(list)
        self.actor_links=[];self.bounds=None;self.counts=Counter();self.cancel=cancel or (lambda:False)
        self.progress=progress or (lambda text:None)
        self.limitations=[
            'Partial cold serialized spatial rendering, not full mission/world compatibility or runtime actor/script transforms.',
            'Scene2 WXYZ is reordered/normalized;4DS XYZW is retained. Known local transforms compose local*parent.',
            'Ambiguous names, unknown scene paths and missing parents withhold world transforms; no flattening.',
            'Sector identities can remain unresolved even when a translation-only-parent invariant establishes world TRS; fallback positions are checks only.',
            'Original lighting/lightmaps, skin/animation/effects and geometry visibility policies are not simulated.',
            'Default ordered cache-root profile is explicit; descendant enumeration and warm caches remain unresolved. Graph candidates retain their certainty.',
            'Bounds enclose supported LOD0 geometry; volume/portal/occluder payloads are not fabricated terrain.']
        self.load()

    def checkpoint(self):
        if self.cancel():raise RuntimeError('Spatial load cancelled')

    def add(self,node,scratch=True):
        if len(self.nodes)>=MAX_NODES:raise ValueError('Spatial node budget exceeded')
        self.nodes[node.identity]=node;self.order.append(node.identity)
        self.all_names[node.name].append(node.identity)
        if scratch:self.names[node.name].append(node.identity)
        return node

    def asset(self,path):
        self.checkpoint();path=virtual_name(path)
        if path in self.assets:return self.assets[path]
        if path in self.asset_failures:return None
        if len(self.assets)>=MAX_ASSETS:
            self.asset_failures[path]={'status':'limited','message':'Scene unique-asset budget reached'};return None
        try:
            self.progress('Decoding selected '+path)
            asset=self.resources.open(path,textures=False)
            if asset.model.status in ('invalid','unsupported'):
                self.asset_failures[path]={'status':asset.model.status,'diagnostics':asset.model.diagnostics};return None
            size=sum(len(l.vertices)*4+sum(len(g.indices)*2 for g in l.groups) for geo in asset.model.geometries for l in geo.lods)
            if self.geometry_bytes+size>GEOMETRY_BUDGET:
                self.asset_failures[path]={'status':'limited','message':'Scene retained geometry budget96MiB exceeded'};return None
            self.resources.textures(asset)
            self.geometry_bytes+=size;self.assets[path]=asset;return asset
        except (ValueError,OSError) as exc:
            self.asset_failures[path]={'status':'invalid' if getattr(exc,'kind','') in ('invalid','integrity_error','bounds_error') else 'unavailable','message':str(exc)}
            return None

    def lookup(self,name,actor=False):
        values=self.names.get(name,[])
        resolved='resolved_exact_name'
        if not values and actor:
            #10046920→1001DF00 uses the default C-locale stricmp branch.
            #The client scratch map remains case-sensitive. Duplicate engine
            #matches are withheld until traversal/sector scope is established.
            fold=lambda value:value.translate(str.maketrans('ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz'))
            values=[identity for key,ids in self.all_names.items() if fold(key)==fold(name) for identity in ids]
            resolved='resolved_scene_case_insensitive'
        return (values[0],resolved) if len(values)==1 else (None,'ambiguous_name' if values else 'missing_name_or_scene_path')

    def attach_asset(self,node,path):
        asset=self.asset(path);node.asset=path
        if asset is None:
            node.status='model_'+self.asset_failures[path]['status'];node.diagnostics.append(str(self.asset_failures[path]));return
        by_index={}
        for frame in asset.model.frames:
            identity=f'{node.identity}::model@0x{frame.offset:x}'
            child=SpatialNode(identity,node.name+'.'+frame.name,frame.kind,path,frame.offset,'props',
                visual_type=frame.visual_type,model_frame=frame.identity,
                parent=by_index.get(frame.parent,node.identity),parent_status='model_serialized_parent',
                serialized_name=frame.name,
                position=frame.position,quaternion=frame.quaternion,scale=frame.scale,local=frame.local_matrix,
                flags=frame.serialized_flags,status=frame.status,model_owner=node.identity,asset=path,
                geometry=frame.geometry,visible=bool(frame.serialized_flags&1),inspection_source=node.inspection_source,inspection_file=node.inspection_file)
            #10034630/1001D470 names imported members owner.member; successful
            #client model load006CE71D enumerates them through006CC580.
            self.add(child);by_index[frame.index]=identity
        node.status='model_'+asset.model.status

    def record_model(self,node,value):
        node.model_resource=value
        #006CD940→model_resource_key→cache1. Models is the established first
        #default root; subdirectory ordering is not reconstructed or guessed.
        try:
            safe=virtual_name(value)
            key=safe.rsplit('.',1)[0] if '.' in safe else safe
            path=virtual_name('models/'+key+'.4ds')
        except ValueError as exc:node.status='unsafe_model_reference';node.diagnostics.append(str(exc));return
        selection=self.resources.lookup.model(value);node.model_resolution=selection
        if selection['path'] is None:
            node.status='model_missing_directory_profile';self.asset_failures.setdefault(path,{'status':'missing','lookup':selection});return
        path=selection['path']
        self.attach_asset(node,path)

    def base(self):
        for name in ('Primary sector','Backdrop sector','Omni sector'):
            self.add(SpatialNode('engine/'+name,name,5,'engine constructor100462B0',None,'sectors',
                world=IDENTITY,flags=0x80,status='verified_constructor_identity'))
        path=f'missions/{self.mission}/scene.4ds';asset=self.asset(path)
        if asset is None:
            self.diagnostics.append('Original scene unavailable: '+str(self.asset_failures[path]));return
        by_index={}
        for frame in asset.model.frames:
            self.checkpoint()
            category='environment' if frame.geometry else 'sectors' if frame.kind==5 else 'frames'
            node=SpatialNode(frame.identity,frame.name,frame.kind,path,frame.offset,category,
                visual_type=frame.visual_type,model_frame=frame.identity,
                parent=by_index.get(frame.parent),parent_status='base_serialized_parent' if frame.parent else 'base_root',
                position=frame.position,quaternion=frame.quaternion,scale=frame.scale,local=frame.local_matrix,
                flags=frame.serialized_flags,status=frame.status,asset=path,geometry=frame.geometry,
                visible=bool(frame.serialized_flags&1),serialized_name=frame.name,
                inspection_source=frame.identity,inspection_file='resource/'+path)
            self.add(node);self.source_nodes[node.identity].append(node.identity);by_index[frame.index]=node.identity
        self.counts['base_declared_frames']=asset.model.header['frame_count'];self.counts['base_decoded_frames']=len(asset.model.frames)

    def apply_object(self,entry,record):
        frame=None;kind=0;created=False;position_set=False;rotation=None;scale=None;position=None
        parent=None;parent_seen=False;parent_status=None;fallback=None;hidden=False;name=''
        for field in record.children:
            if field.label=='creation_kind':kind=field.values['value']
            elif field.label=='object_name':
                name=field.values['text']
                if not kind:
                    identity,status=self.lookup(name)
                    frame=self.nodes.get(identity)
                    if frame is None:self.observations.append({'record':record.identity,'name':name,'status':status})
                else:
                    if self.names.get(name):
                        self.observations.append({'record':record.identity,'name':name,'status':'duplicate_creation_declined'});continue
                    if kind not in (1,2,3,4,6,8,9,12,14):
                        self.observations.append({'record':record.identity,'name':name,'status':'unsupported_creation_kind'});continue
                    frame=self.add(SpatialNode(record.identity,name,kind,entry.path,record.offset,
                        'lights' if kind==2 else 'props' if kind==9 else 'frames',
                        inspection_file=entry.path,inspection_source=record.identity))
                    created=True
            elif field.label=='position':position=tuple(field.values['serialized']);position_set=True
            elif field.label=='rotation_wxyz':rotation=tuple(field.values['serialized'])
            elif field.label=='scale':scale=tuple(field.values['serialized'])
            elif field.label=='fallback_position':fallback=tuple(field.values['serialized'])
            elif field.label=='model_resource' and frame is not None and frame.kind==9:
                value=field.values['text']
                if not value:
                    frame.status='removed_model';frame.visible=False;frame.parent_status='removed_model'
                    self.names[name]=[];self.all_names[frame.name]=[]
                    frame.records.append(record.identity);self.source_nodes[record.identity].append(frame.identity)
                    return
                elif frame.asset:
                    frame.diagnostics.append('Repeated model replacement is not applied');frame.status='replacement_unresolved'
                else:self.record_model(frame,value)
            elif field.label=='parent reference' and frame is not None:
                if field.children and field.children[0].label=='parent_frame':
                    parent_seen=True;name_ref=field.children[0].values['text']
                    parent,parent_status=self.lookup(name_ref);frame.requested_parent=name_ref
                    # Native falls back only if world_position was already read.
                    if parent is None and fallback is not None:position=fallback
            elif field.label=='hidden':hidden=True
            elif field.label=='visual_override' and frame is not None:
                frame.diagnostics.append('Visual replacement/lightmap rendering is not simulated; serialized mesh positions retained')
        if frame is None:return
        original_placement=(frame.position,frame.quaternion,frame.scale,frame.parent)
        frame.records.append(record.identity);self.source_nodes[record.identity].append(frame.identity)
        applied={'creation_kind','object_name','position','rotation_wxyz','scale','fallback_position','model_resource','parent reference','hidden'}
        frame.uninterpreted_records.extend({'identity':field.identity,'offset':field.offset,'kind':field.kind,
            'label':field.label,'status':'not_applied_to_spatial_snapshot'} for field in record.children if field.label not in applied)
        frame.fallback=fallback if fallback is not None else frame.fallback
        if position_set and not frame.flags&0x80:frame.position=position
        if parent_seen:
            frame.parent=parent;frame.parent_status=parent_status
        elif created:
            frame.parent_status='automatic_sector_unresolved'
        if rotation is not None:
            try:frame.quaternion=scene_quaternion(rotation)
            except ValueError as exc:frame.parent_status='invalid_transform';frame.diagnostics.append(str(exc))
        if scale is not None:frame.scale=scale
        if hidden:frame.visible=False
        frame.local=local_matrix(frame.position,frame.quaternion,frame.scale)
        if frame.kind==5 and original_placement!=(frame.position,frame.quaternion,frame.scale,frame.parent):frame.sector_transform_changed=True

    def actors(self,entry,record):
        identity=None;status='missing_actor_frame';name=None;link=None
        for field in record.children:
            if field.label=='actor_frame':
                name=field.values['text'];identity,status=self.lookup(name,True)
            elif field.label=='actor_type':
                kind=field.values['value']
                link={'source':record.identity,'definition':field.identity,'file':entry.path,
                    'name':name,'type':kind,'type_label':ACTOR_NAMES.get(kind,'Unknown'),
                    'status':status,'certainty':'serialized_frame_name' if identity else 'unresolved','target':identity,
                    'evidence':'006CC760 AE23→AE22; unique exact scratch map then10046920/1001DF00 ASCII-insensitive scene fallback; duplicate traversal/global scope excluded'}
                self.actor_links.append(link)
                if identity:
                    node=self.nodes[identity];node.actors.append(link)
                    if identity not in self.source_nodes[record.identity]:self.source_nodes[record.identity].append(identity)
                    node.category='spawn' if kind==10 else 'actors'
                    node.links.append({'relation':'actor','target':record.identity,'file':entry.path,'certainty':link['certainty']})
            elif field.label=='actor properties' and link:
                link.setdefault('properties',[]).append({'identity':field.identity,**field.values})

    def compose(self):
        # Re-evaluate the invariant on each composition; it never becomes an
        # explicit serialized link or survives newly incomplete sector evidence.
        for node in self.nodes.values():
            if node.parent_status=='automatic_sector_identity_unresolved':
                node.parent_status='automatic_sector_unresolved';node.automatic_candidates=[]
        done=set();active=set()
        def world(identity,depth=0):
            node=self.nodes[identity]
            if identity in done:return node.world
            node.box=None
            if identity in active or depth>256:
                node.parent_status='cycle_or_depth_limit';node.world=None;return None
            active.add(identity)
            if node.status=='replacement_unresolved' or node.parent_status in ('removed_model','missing_name_or_scene_path','ambiguous_name','automatic_sector_unresolved','automatic_sector_identity_unresolved','invalid_transform','replacement_unresolved'):
                node.world=None
            elif node.parent:
                parent=world(node.parent,depth+1)
                node.world=multiply(node.local,parent) if parent is not None else None
            else:node.world=node.local
            node.world_status='composed_serialized' if node.world is not None else 'unresolved'
            node.display_visible=node.visible and (self.nodes[node.parent].display_visible if node.parent else True)
            if node.world is not None:
                if not all(math.isfinite(v) and abs(v)<=1e15 for v in node.world):node.world=None;node.world_status='unresolved';node.diagnostics.append('Unsafe composed transform')
                elif node.geometry and node.geometry.lods:node.box=transformed_box(node.geometry.lods[0].bounds,node.world)
            active.discard(identity);done.add(identity);return node.world
        for identity in self.order:world(identity)
        # LinkTo(...,1) preserves the pre-link world matrix. For newly created
        #unparented frames, if EVERY supported possible sector has identity
        #linear transform and did not change during document composition, a
        #later local rotation/scale override cannot change world TRS. Parent
        #identity remains unknown: do not flatten it into a guessed sector.
        sectors=[n for n in self.nodes.values() if n.kind==5]
        linear=tuple(i for i in range(12) if i not in (3,7,11))
        invariant=sectors and not self.asset_failures and self.counts['base_declared_frames']==self.counts['base_decoded_frames'] and all(
            a.model.header.get('frame_count')==len(a.model.frames) for a in self.assets.values()) and all(
            n.world is not None and all(abs(n.world[i]-IDENTITY[i])<1e-12 for i in linear) and not n.sector_transform_changed for n in sectors)
        if invariant:
            for node in self.nodes.values():
                if node.parent_status=='automatic_sector_unresolved' and all(math.isfinite(v) and abs(v)<=1e15 for v in node.local):
                    node.automatic_candidates=[n.identity for n in sectors]
                    node.parent_status='automatic_sector_identity_unresolved'
                    node.world=node.local;node.world_status='verified_translation_only_sector_invariant'
                    message='World TRS established independently of sector identity; original sector selection/portal order remains unresolved'
                    if message not in node.diagnostics:node.diagnostics.append(message)
            #Resolve children only through an established parent matrix.
            done={i for i,n in self.nodes.items() if n.world is not None}
            for identity in self.order:
                node=self.nodes[identity]
                if identity not in done:world(identity)
        # Category inheritance is display filtering, not actor initialization.
        for identity in self.order:
            node=self.nodes[identity]
            if node.model_owner and self.nodes[node.model_owner].category in ('actors','spawn'):node.category=self.nodes[node.model_owner].category
        self.bounds=bounds(p for node in self.nodes.values() if node.box for p in node.box)
        children=defaultdict(list)
        for node in self.nodes.values():
            if node.parent:children[node.parent].append(node)
        def aggregate(node,depth=0):
            if depth>256 or node.world is None:return None
            boxes=[aggregate(child,depth+1) for child in children[node.identity]]
            if node.geometry:return node.box
            node.box=bounds(p for box in boxes if box for p in box)
            return node.box
        for node in self.nodes.values():
            if not node.parent:aggregate(node)

    def links(self):
        report=self.session.report
        for entry in report.files:
            for record in walk_records(entry.records):
                if record.label!='frame → script':continue
                identity,status=self.lookup(record.values['frame'],True)
                if identity:
                    refs=[r for r in entry.references if r.source==record.identity and r.kind=='script']
                    self.nodes[identity].links.extend({'relation':'script','name':r.value,'source':record.identity,'file':entry.path,
                        'targets':r.targets,'status':r.status,'certainty':'serialized_binding','evidence':r.evidence} for r in refs)
        for entry in report.files:
            references=defaultdict(list)
            for ref in entry.references:references[ref.source].append(ref)
            def visit(records,owners=()):
                for record in records:
                    current=self.source_nodes.get(record.identity,owners)
                    for identity in current:
                        for ref in references.get(record.identity,[]):
                            self.nodes[identity].links.append({'relation':ref.kind,'value':ref.value,'targets':ref.targets,
                                'status':ref.status,'certainty':'existing_graph_reference','evidence':ref.evidence})
                    visit(record.children,current)
            visit(entry.records)

    def load(self):
        self.base()
        for filename in ('scene2.bin','actors.bin','sounds.bin'):
            self.checkpoint();identity=f'resource/missions/{self.mission}/{filename}'
            entry=next((f for f in self.session.report.files if f.path==identity),None)
            if entry is None:continue
            if entry.status!='decoded_subset':self.diagnostics.append(f'{identity}: {entry.status}; not composed');continue
            self.documents.append({'identity':identity,'sha256':entry.sha256,'resolution':entry.resolution})
            for record in walk_records(entry.records):
                if record.label=='object record':self.counts[filename+'_objects']+=1;self.apply_object(entry,record)
                elif record.label=='actor record':self.counts[filename+'_actors']+=1;self.actors(entry,record)
        self.compose();self.links()
        self.counts.update(nodes=len(self.nodes),assets=len(self.assets),geometry_bytes=self.geometry_bytes,
            texture_bytes=sum(i.retained_bytes for i in self.texture_cache.values()),
            mesh_instances=sum(bool(n.geometry and n.world is not None and n.display_visible) for n in self.nodes.values()),
            unresolved_transforms=sum(n.world is None for n in self.nodes.values()),
            unresolved_actors=sum(l['target'] is None for l in self.actor_links))
        self.counts['world_fallback_matches']=sum(n.world is not None and n.fallback is not None and math.dist(n.world[12:15],n.fallback)<.02 for n in self.nodes.values())
        self.counts['world_fallback_comparisons']=sum(n.world is not None and n.fallback is not None for n in self.nodes.values())
        from spatial_coverage import report
        self.coverage=report(self)
        from spatial_scripts import ScriptRelations
        self.script_relations=ScriptRelations(self)
        self.coverage['scripts']=self.script_relations.coverage()

    def metadata(self):
        return {'schema_version':2,'mission':self.mission,'status':'partial_serialized_spatial_snapshot','evidence':EVIDENCE,
            'statistics':dict(self.counts),'documents':self.documents,'bounds':self.bounds,
            'nodes':[self.nodes[i].metadata() for i in self.order],'actor_links':self.actor_links,'observations':self.observations,
            'asset_failures':self.asset_failures,'assets':{k:{'status':a.model.status,'sha256':a.model.sha256,'provenance':a.model.provenance,
                'statistics':a.model.statistics(),'textures':a.textures,'material_preview':a.material_preview} for k,a in sorted(self.assets.items())},
            'diagnostics':self.diagnostics,'limitations':self.limitations,'resolution_profile':self.store.profile(),
            'cache_directory_profile':self.resources.lookup.metadata(),'coverage':self.coverage}

    def export(self,path,coverage_only=False):
        path=Path(path)
        if path.resolve().is_relative_to(self.store.root):raise ValueError('Export outside installation')
        with path.open('x',encoding='utf-8') as stream:
            stream.writelines(json.JSONEncoder(sort_keys=True,indent=2,ensure_ascii=True,allow_nan=False).iterencode(self.coverage if coverage_only else self.metadata()));stream.write('\n')
