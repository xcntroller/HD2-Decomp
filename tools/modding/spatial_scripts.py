"""Bounded static script neighborhoods over existing compiler and scene products.

No execution/data-flow inference. A spatial name match never upgrades the
existing mission graph's reference certainty or proves a runtime assignment.
"""
from collections import Counter,defaultdict
from dataclasses import dataclass,asdict,field
from dependency_graph import stable
from mission import walk_records

MAX_ROWS=250
MAX_OVERLAYS=96
MAX_SCAN=12000
EVIDENCE='00428600 bindings;00416E10 opcodes1/5E/66;0042C0B0 compiler observations'


@dataclass(frozen=True)
class Binding:
    identity:str
    object:str|None
    script:str|None
    frame:str
    file:str
    offset:int
    status:str
    profile:str
    graph_edges:tuple
    evidence:str
    candidates:tuple=()
    candidate_count:int=0


@dataclass(frozen=True)
class ScriptReference:
    identity:str
    script:str
    statement:str|None
    command:str
    kind:str
    literal:str|None
    dynamic:bool
    certainty:str
    status:str
    target:str|None
    resources:tuple
    graph_edges:tuple
    locations:tuple
    pc:int
    evidence:str
    original_resolution_status:str=''
    candidates:tuple=()
    candidate_count:int=0


@dataclass(frozen=True)
class Overlay:
    identity:str
    source:str
    target:str
    reference:str
    binding:str
    certainty:str
    label:str
    # Coordinates are read from the unchanged scene, not copied or invented.


@dataclass
class Item:
    identity:str
    kind:str
    label:str
    script:str|None
    certainty:str
    status:str
    direction:str
    source_object:str|None=None
    target_object:str|None=None
    locations:list=field(default_factory=list)
    detail:dict=field(default_factory=dict)
    path:str=''

    def metadata(self):return asdict(self)


class ScriptRelations:
    def __init__(self,scene):
        self.scene=scene;self.files={f.path:f for f in scene.session.report.files}
        self.bindings={};self.by_object=defaultdict(list);self.candidate_bindings=defaultdict(list);self.users=defaultdict(list)
        self.references={};self.instructions={};self.by_script=defaultdict(list);self.incoming=defaultdict(list)
        fold=lambda value:value.translate(str.maketrans('ABCDEFGHIJKLMNOPQRSTUVWXYZ','abcdefghijklmnopqrstuvwxyz'))
        folded=defaultdict(list)
        for name,ids in scene.all_names.items():folded[fold(name)].extend(ids)
        self.graph_edges=defaultdict(list)
        for edge in getattr(scene.session.report,'graph',{}).get('edges',()):
            self.graph_edges[edge.source].append(edge.identity)
        for entry in self.files.values():
            for record in walk_records(entry.records):
                if record.label!='frame → script':continue
                frame=record.values['frame'];matches=scene.all_names.get(frame,[])
                obj=matches[0] if len(matches)==1 else None
                ref=next((r for r in entry.references if r.source==record.identity and r.kind=='script'),None)
                script=ref.targets[0] if ref and len(ref.targets)==1 else None
                status='unique_serialized_name_match' if obj else 'ambiguous_frame_name' if matches else 'unresolved_frame_name'
                binding=Binding(record.identity,obj,script,frame,entry.path,record.offset,status,
                    'multiplayer_table' if entry.path.endswith('/mpscripts.dta') else 'singleplayer_table',
                    tuple(self.graph_edges[record.identity]),record.evidence,tuple(matches[:64]),len(matches))
                self.bindings[binding.identity]=binding
                if obj:self.by_object[obj].append(binding.identity)
                elif matches:
                    for candidate in matches[:64]:self.candidate_bindings[candidate].append(binding.identity)
                if script:self.users[script].append(binding.identity)
            report=entry.script
            if not report:continue
            instructions={i['pc']:i for i in report['instructions']}
            for ref in report['references']:
                instruction=instructions[ref['pc']];statement=instruction['statement']
                identity=ref['instruction_identity'];target=None;candidates=[]
                status=ref.get('resolution_status','unresolved')
                if ref['dynamic']:status='dynamic_name_not_evaluated'
                elif ref['kind']=='child_frame':status='requires_runtime_parent'
                elif ref['kind']=='scene_frame':
                    #Conservatively withhold case variants as well as duplicate
                    #names. Native driver exact-case/global-scope precedence is
                    #not reproduced by a cold static scene-name index.
                    candidates=folded.get(fold(ref['literal']),[])
                    target=candidates[0] if len(candidates)==1 else None
                    status='static_unique_name_candidate' if target else 'ambiguous_name' if candidates else 'missing_name_or_scene_path'
                elif ref['kind']=='script':status='possible_assignment_runtime_receiver_unknown'
                view=ScriptReference(identity,entry.path,
                    report['statements'][statement]['identity'] if statement is not None else None,
                    instruction['name'],ref['kind'],ref['literal'],ref['dynamic'],
                    ref.get('relationship_certainty','serialized_unresolved'),status,target,
                    tuple(ref.get('targets',[])),tuple(self.graph_edges[identity]),
                    tuple(ref.get('locations',instruction.get('locations',[]))),ref['pc'],ref['evidence'],ref.get('resolution_status','unresolved'),tuple(candidates[:64]),len(candidates))
                self.references[identity]=view;self.instructions[identity]=instruction;self.by_script[entry.path].append(identity)
                if target:self.incoming[target].append(identity)
                elif status=='ambiguous_name':
                    for candidate in candidates[:64]:self.incoming[candidate].append(identity)

    def scope(self,identity):
        node=self.scene.nodes[identity]
        # Model ownership is serialized containment, not inheritance of a VM.
        return tuple(dict.fromkeys([identity]+([node.model_owner] if node.model_owner else [])))

    def contexts(self,script,pc):
        report=self.files[script].script
        return [dict(event_index=n,name=e['name'],identity=e.get('identity'),symbol=e.get('symbol'),registered=e['registered'],
            statement=report['statements'][e['statement']]['identity'],certainty='compiler_block_containment_not_execution')
            for n,e in enumerate(report.get('events',[])) if e.get('block_open_pc',-1)<=pc<=e.get('block_close_pc',-2)]

    def reference_item(self,ref,direction,source=None):
        report=self.files[ref.script].script;instruction=self.instructions[ref.identity]
        statement=instruction['statement'];events=self.contexts(ref.script,ref.pc)
        args=[report['arguments'][i] for i in report['statements'][statement]['argument_indices']] if statement is not None else []
        detail=asdict(ref)
        detail.update(arguments=args,operands=instruction['operands'],events=events,
            receiver_semantics=('00416E10 ScriptAssign: runtime frame operand0; possible SpawnPoint→spawned Human fallback; neither receiver nor execution is evaluated' if ref.kind=='script' else
                '00416E10 FRM_FindChildFrame: runtime parent operand2 required; aliasing destination/parent clears it' if ref.kind=='child_frame' else
                '00416E10 FRM_FindFrame: driver+88 then scene+7C; unique static scene-name candidate only, global registration scope is not reproduced'),
            runtime_assignment_observed=False,original_graph_certainty=ref.certainty,
            target_provenance=[{'identity':p,'resolution':self.files[p].resolution,'status':self.files[p].status} for p in ref.resources if p in self.files])
        path=' > '.join([ref.script]+[e['name']+(' '+str(e['identity']) if e['identity'] is not None else '') for e in events]+[ref.command])
        return Item(ref.identity,'Reference',f'{ref.command}: {ref.literal if ref.literal is not None else "<dynamic>"}',ref.script,
            ref.certainty,ref.status,direction,source,ref.target,list(ref.locations),detail,path)

    def script_items(self,script,source=None):
        entry=self.files.get(script);report=entry.script if entry else None
        if not report:
            yield Item(script,'Unavailable','Script analysis unavailable',script,'resolver_selected',entry.status if entry else 'missing','Outgoing',source,
                detail={'reason':'Existing inspection did not retain compiler analysis; open Script Inspector explicitly. No products invented.'})
            return
        for kind,key in [('Event','events'),('Label','labels'),('Variable','declarations'),('Control','controls'),('Include','includes'),('Diagnostic','diagnostics')]:
            for n,item in enumerate(report.get(key,[])):
                statement=item.get('statement');identity=report['statements'][statement]['identity'] if statement is not None else script+f'#{key}={n}'
                label=str(item.get('name',item.get('symbol',item.get('message',item.get('kind',kind)))))
                yield Item(identity,kind,label,script,'compiler_product' if kind not in ('Include','Diagnostic') else 'resolver_selected' if kind=='Include' else 'diagnostic',
                    'compiled' if report['syntax_accepted'] else 'partial_syntax_only','Outgoing',source,locations=item.get('locations',[]),
                    detail=item,path=script+' > '+label)
        refs={self.references[i].statement for i in self.by_script[script]}
        for statement in report['statements']:
            if statement['identity'] in refs:continue
            yield Item(statement['identity'],'Command',statement['name'],script,'compiler_product' if report['syntax_accepted'] else 'partial_syntax',
                'compiled_not_execution' if report['syntax_accepted'] else 'partial_syntax_only','Outgoing',source,locations=statement['locations'],
                detail={'statement':statement,'arguments':[report['arguments'][i] for i in statement['argument_indices']],
                    'events':self.contexts(script,statement['pc_begin']) if statement['pc_begin'] is not None else [],
                    'graph_edges':self.graph_edges.get(statement['identity'],[]),'semantics':'Only the three established reference receivers are linked; other arguments are not inferred as object/resource references.'},
                path=script+' > '+statement['name'])
        for identity in self.by_script[script]:yield self.reference_item(self.references[identity],'Outgoing',source)

    def items(self,identity):
        scope=self.scope(identity);scripts={}
        for obj in scope:
            for bid in self.by_object[obj]+self.candidate_bindings[obj]:
                binding=self.bindings[bid];entry=self.files.get(binding.script)
                yield Item(bid,'Binding',binding.frame+' → '+(binding.script or '<unresolved script>'),binding.script,
                    'serialized' if binding.object else 'name_candidate',binding.status,'Outgoing',binding.object,detail={**asdict(binding),
                        'script_provenance':entry.resolution if entry else None,
                        'script_status':entry.status if entry else 'unresolved',
                        'binding_provenance':self.files[binding.file].resolution,
                        'binding_archive_decoding':self.files[binding.file].archive_decoding,
                        'scope':'Selected object' if obj==identity else 'Serialized model owner; does not prove child VM ownership',
                        'runtime_assignment_observed':False,'limitation':'Serialized pair only; original driver+88 filtering, reverse binding order and current runtime context are not evaluated'},path='Serialized binding')
                if binding.script and binding.object:scripts.setdefault(binding.script,obj)
        for script,obj in scripts.items():yield from self.script_items(script,obj)
        seen=set()
        for obj in scope:
            for rid in self.incoming[obj]:
                if rid in seen:continue
                seen.add(rid);ref=self.references[rid]
                users=self.users[ref.script]
                for bid in users[:64] or [None]:
                    binding=self.bindings.get(bid)
                    item=self.reference_item(ref,'Incoming',binding.object if binding else None)
                    item.detail['source_binding']=asdict(binding) if binding else None
                    item.detail['source_binding_total']=len(users)
                    item.detail['source_contexts_limited']=len(users)>64
                    if binding:item.label+=' ← '+binding.frame
                    yield item
        if not scripts:
            yield Item('notice:'+stable(identity),'Notice','No verified serialized script binding for this object/model owner',None,
                'unresolved','no_serialized_binding','Outgoing',identity,detail={'limitation':'Runtime ScriptAssign, spawned Human inheritance and lexical mentions do not establish ownership. Incoming references can still be inspected.'})

    def query(self,identity,search='',direction='All',kind='All',offset=0,limit=MAX_ROWS):
        if identity not in self.scene.nodes:raise ValueError('Unknown spatial identity')
        offset=max(0,int(offset));limit=max(1,min(MAX_ROWS,int(limit)));terms=search.casefold().split();rows=[];total=0;limited=False
        for scanned,item in enumerate(self.items(identity)):
            if scanned>=MAX_SCAN:limited=True;break
            if direction!='All' and item.direction!=direction:continue
            if kind!='All' and item.kind!=kind:continue
            text=self.search_text(item)
            if not all(t in text for t in terms):continue
            if offset<=total<offset+limit:rows.append(item)
            total+=1
        scripts=sorted({r.script for r in rows if r.script})
        return {'identity':identity,'scope':list(self.scope(identity)),'filters':{'search':search,'direction':direction,'kind':kind},'total':total,'total_is_exact':not limited,'scan_limit':MAX_SCAN,'offset':offset,'limit':limit,'items':[r.metadata() for r in rows],
            'scripts':{s:{'status':self.files[s].status,'sha256':self.files[s].sha256,'resolution':self.files[s].resolution,'archive_decoding':self.files[s].archive_decoding,
                'sources':self.files[s].script['sources'] if self.files[s].script else []} for s in scripts if s in self.files},
            'objects':[{'identity':n.identity,'name':n.name,'actor_types':[a['type_label'] for a in n.actors],
                'source':n.source,'offset':n.offset,'world_status':n.world_status,'parent_status':n.parent_status,
                'world':n.world,'source_resolution':self.files[n.inspection_file].resolution if n.inspection_file in self.files else None}
                for n in (self.scene.nodes[i] for i in self.scope(identity))],
            'interpretation':'Static serialized/compiler relationships, never execution or observed assignment'}

    def overlays(self,identity,search='',direction='All',limit=MAX_OVERLAYS):
        terms=search.casefold().split();rows=[];total=0;seen=set();scope=self.scope(identity)
        def add(ref,binding,direction):
            nonlocal total
            source=binding.object;target=ref.target
            if not source or not target or source==target:return
            if self.scene.nodes[source].world is None or self.scene.nodes[target].world is None:return
            label=f'{ref.command}: {ref.literal}'
            if terms:
                item=self.reference_item(ref,direction,source)
                if direction=='Incoming':item.label+=' ← '+binding.frame
                if not all(t in self.search_text(item) for t in terms):return
            key=(ref.identity,binding.identity)
            if key in seen:return
            seen.add(key);total+=1
            if len(rows)<min(MAX_OVERLAYS,max(0,limit)):
                rows.append(Overlay('spatial-edge:'+stable(*key),source,target,ref.identity,binding.identity,
                    'candidate',label)) # Name matching/global lookup is not a proven runtime resolution.
        def pairs():
            if direction in ('All','Outgoing'):
                for obj in scope:
                    for bid in self.by_object[obj]:
                        b=self.bindings[bid]
                        for rid in self.by_script.get(b.script,[]):yield self.references[rid],b,'Outgoing'
            if direction in ('All','Incoming'):
                for obj in scope:
                    for rid in self.incoming[obj]:
                        ref=self.references[rid]
                        for bid in self.users[ref.script]:yield ref,self.bindings[bid],'Incoming'
        limited=False
        for scanned,(ref,binding,pair_direction) in enumerate(pairs()):
            if scanned>=MAX_SCAN:limited=True;break
            add(ref,binding,pair_direction)
        return {'total':total,'total_is_exact':not limited,'scan_limit':MAX_SCAN,'limit':MAX_OVERLAYS,'edges':rows,'omitted':total-len(rows),
            'legend':'Dashed candidate lines are static references; arrowheads indicate source-to-mentioned target, never movement or execution'}

    def coverage(self):
        statuses=Counter(r.status for r in self.references.values());bound=Counter(b.status for b in self.bindings.values())
        visualizable=0;nonspatial=0;sources=defaultdict(set)
        for binding in self.bindings.values():
            if binding.object and self.scene.nodes[binding.object].world is not None:sources[binding.script].add(binding.object)
        for ref in self.references.values():
            users=sources.get(ref.script,set())
            if ref.target and self.scene.nodes[ref.target].world is not None and (len(users)>1 or users and ref.target not in users):visualizable+=1
            else:nonspatial+=1
        return {'schema_version':1,'binding_records':len(self.bindings),'binding_states':dict(sorted(bound.items())),
            'analyzed_scripts':sum(f.script is not None for f in self.files.values()),'compiled_reference_records':len(self.references),
            'reference_states':dict(sorted(statuses.items())),'references_with_spatial_context':visualizable,'nonspatial_or_unresolved_references':nonspatial,
            'runtime_assignments_observed':0,'scope':'Only existing retained compiler references for ScriptAssign/FRM_FindFrame/FRM_FindChildFrame; no lexical or data-flow inference',
            'query_limit':MAX_ROWS,'overlay_limit':MAX_OVERLAYS,'neighborhood_scan_limit':MAX_SCAN}

    @staticmethod
    def search_text(item):
        positions=' '.join(f'{loc["source"]}:{loc["begin"]["line"]}:{loc["begin"]["column"]}' for loc in item.locations)
        return f'{item.identity} {item.kind} {item.path} {item.label} {item.certainty} {item.status} {item.direction} {positions}'.casefold()

    def evidence(self,item):
        value=dict(item);entry=self.files.get(item.get('script'))
        value['script_provenance']=entry.resolution if entry else None
        value['script_archive_decoding']=entry.archive_decoding if entry else None
        value['script_sha256']=entry.sha256 if entry else None
        value['spatial_endpoints']={}
        for role in ('source_object','target_object'):
            node=self.scene.nodes.get(item.get(role))
            if node:
                asset=self.scene.assets.get(node.asset)
                value['spatial_endpoints'][role]={'identity':node.identity,'frame_name':node.name,'source':node.source,'offset':node.offset,
                    'actor_types':[a['type_label'] for a in node.actors],'world_status':node.world_status,'parent_status':node.parent_status,
                    'world_position':node.world[12:15] if node.world else None,'asset_provenance':asset.model.provenance if asset else None}
        if entry and entry.script:
            value['syntax_accepted']=entry.script['syntax_accepted'];value['limitations']=entry.script['limitations']
            value['diagnostics']=entry.script['diagnostics'];value['sources']=entry.script['sources'];value['include_context']=entry.script['include_context']
        return value

    def source(self,item):
        script=item.get('script');entry=self.files.get(script)
        if not entry or not entry.script:raise ValueError('No retained compiler source map for this script')
        locations=item.get('locations',[])
        location=locations[max(0,min(len(locations)-1,item.get('location_index',0)))] if locations else None
        identity=location['source'] if location else script
        metadata=next((s for s in entry.script['sources'] if s['identity']==identity),None)
        if not metadata:raise ValueError('Source is not part of the retained compiler source map')
        if metadata['size']>65535:raise ValueError('Source exceeds the bounded source panel; compiler diagnostics remain available')
        data=self.scene.store.read(identity[9:])
        import hashlib
        if hashlib.sha256(data).hexdigest()!=metadata['sha256']:raise ValueError('Source changed since analysis; reload the mission snapshot')
        return data,entry.script,identity,location

    def snapshot(self,identity,search='',direction='All',kind='All',offset=0):
        overlays=self.overlays(identity,search,direction) if kind in ('All','Reference') else {'edges':[],'total':0,'total_is_exact':True,'omitted':0}
        return {'schema_version':1,'mission':self.scene.mission,
            'query':self.query(identity,search,direction,kind,offset),
            'overlays':{**overlays,'edges':[asdict(e) for e in overlays['edges']]},
            'coverage':self.coverage(),'resolution_profile':self.scene.store.profile()}

    def export(self,path,value):
        from pathlib import Path
        import json
        path=Path(path)
        if path.resolve().is_relative_to(self.scene.store.root):raise ValueError('Export outside installation')
        with path.open('x',encoding='utf-8') as stream:
            stream.writelines(json.JSONEncoder(sort_keys=True,indent=2,ensure_ascii=True,allow_nan=False).iterencode(value));stream.write('\n')
