#!/usr/bin/env python3
"""Deterministic research export; no ledger writes or inferred call dependencies."""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
SOURCE = '52c568964063207b0d15bea8ef98af09f89d84df'

def read(path):
    return json.loads((ROOT / path).read_text())

def digest(path):
    return hashlib.sha256((ROOT / path).read_bytes()).hexdigest()

def generate():
    manifest_path = 'analysis/future-dependencies/evidence/manifest.json'
    modules_path = 'analysis/future-dependencies/evidence/module-persistence.json'
    manifest, modules = read(manifest_path), read(modules_path)
    ledger = {(r['program'], r['entry']): (i, r) for i, r in enumerate(read('analysis/function-status.json')['functions'])}
    # Catalog associations are scope groupings, not call or initialization proofs.
    specs = [
      ('preparation','P2.4','Application GameData/squad/stores; process lifetime; descriptors borrowed by states',
       '006c0ef0 006b9190 006b9680 00673dd0 00698510', 'analysis/singleplayer-pipeline.md','Existing preparation source; actor consumption/desktop loadout acceptance pending'),
      ('retention','P2.5','Application owns active menu; menu+5B8 borrows retained game until unwrapped',
       '0069e620 0067f350 0067fb00 00680680 0067f550 006c20f0', 'analysis/future-dependencies/phase2-overview.md','Pause/audio/input and in-game Options leaves remain'),
      ('envelope','P2.6','Stack ChunkFile owns handle/stack; Files outlives it; application supplies filename',
       '006c3340 006c39f0 006c47c0 006b03b0 006b0520', 'analysis/future-dependencies/p2.6-save-load.md','Full receiver is not metadata validation; failure semantics need native policy'),
      ('application_persistence','P2.6','Existing application+04/+10/+14/+18/+1C/+20/+28 owners; no parallel save state',
       '006b6ff0 006b71c0 006b72b0 006b6a50 006b6de0 006a2150 006a4940 006a4e00 006a2340 006a1b40 006a66f0 006b4250 006b4400 006b46b0 006b47b0 006a1780 006a57b0 006a5890 00427010 00427050 006b7410',
       'analysis/future-dependencies/p2.6-save-load.md','Unspecified application/stock/GameData/variable leaves block whole preparation format'),
      ('snapshot','P2.6','Same live game/session and missionA8; module registry process lifetime, scene borrowers world lifetime',
       '0069c390 0069c8e0 0069cfc0 0048dc90 0048de60 0049f600 0049f8d0 006df4c0 006df690 006dece0 00523f30 005241a0 0048fc50 0048fdd0 00493390 004933a0 0069c280',
       'analysis/future-dependencies/p2.6-save-load.md','Remaining module/mission payloads, actor leaves and failure cleanup'),
      ('identity','P2.6','Actual actor slot vector and dynamic actor owner; references borrowed; retire borrowers first',
       '00476370 004764d0 006e0890 006e0b70 00476a50 00477b90 00477f50 007ddcb0 00475d50 00475ec0',
       'analysis/future-dependencies/p2.6-save-load.md','Human/other actor virtual48/4C and relationship semantics unresolved'),
      ('script_persistence','P2.6','Registered Script module owns contexts/rotations; contexts borrow actual scene frames and compiled program',
       '00424970 004257b0 004147d0 00414e30 00427730',
       'analysis/future-dependencies/p2.6-context-layout.md','Exact context field meanings, event cardinality and special references remain'),
      ('resume','P2.6','Application owns pending path/command and current state; profile process lifetime',
       '006dcae0 006c3230 006c38f0 006c3ed0 006c4070 0069c340 006b7a00 006b7d30 006b76d0 006b76e0 006b3c90 006b3d00 0069a500',
       'analysis/future-dependencies/p2.6-save-load.md','006B7410/006B7D30 final contracts and failed game activation unwind'),
      ('results','P2.7','Mission result uses same missionA8/Players; debrief borrows application squad/HumanStore',
       '0068d2f0 0068ead0 0068f3d0 0068e0f0 0068d5c0 0068ce00 0068c300 0068c2e0 0068ed90 0068f380 006b12d0 0069ba60 0069bc40 0069beb0 0069bed0',
       'analysis/future-dependencies/p2.7-results.md','Producer handoff, rank/stat/random formulas and repeated-entry gating'),
      ('campaign','P2.7','GameData progression and profile own data; campaign screen owns model; table static process lifetime',
       '0068bd10 0068bdd0 0068bde0 0068c0f0',
       'analysis/menu-definition-resolution.md','Real installed-data progression and result handoff pending'),
    ]
    nodes, edges = {}, {}
    exports = {r['entry']: (i, r) for i, r in enumerate(manifest['exports'])}
    def node(key, kind, **data):
        nodes[key] = {'id':key,'type':kind,'data':data}
    def edge(source, target, role, evidence, blocking=False, condition=None, confidence='confirmed', **extra):
        key = 'relationship:' + hashlib.sha256(json.dumps([source,target,role], separators=(',',':')).encode()).hexdigest()
        edges[key] = {'id':key,'source':source,'target':target,'type':'dependency' if role=='depends_on' else 'relationship',
                      'data':{'relationship':role,'blocking':blocking,'condition':condition,'confidence':confidence,
                              'basis':'explicit' if confidence=='confirmed' else 'inferred','evidence_state':'verified' if confidence=='confirmed' else 'unknown',
                              'provenance':evidence,**extra}}
    def prov(path, **extra): return {'path':path,**extra}
    def function(a, subsystem, milestone, owner, evidence, blocker):
        key = 'function:client:' + a
        if key in nodes:
            if milestone not in nodes[key]['data']['milestones']: nodes[key]['data']['milestones'].append(milestone)
            return key
        indexed = exports.get(a)
        row = ledger.get(('client',a))
        mapping = []
        if row:
            mapping = [p for p in row[1].get('evidence',[]) if p.startswith('recovered/')]
        source_mentions = []
        for file in sorted((ROOT/'recovered').rglob('*')):
            if file.suffix not in ('.h','.cpp'): continue
            for number,line in enumerate(file.read_text(errors='replace').splitlines(),1):
                if a in line.lower(): source_mentions.append({'path':str(file.relative_to(ROOT)), 'line':number,'relationship':'address_mention_only'})
        provenance = [prov(evidence)]
        if indexed: provenance.append(prov(manifest_path,json_pointer=f'/exports/{indexed[0]}'))
        if row: provenance.append(prov('analysis/function-status.json',json_pointer=f'/functions/{row[0]}'))
        status = row[1]['status'] if row else 'no_ledger_claim'
        node(key,'function',label='client:'+a,subsystem=subsystem,milestones=[milestone],original_addresses=[a],
             owner=owner,lifetime='See owner contract; no independent save-only owner',source_mapping=mapping,
             source_mentions=source_mentions,status=status,implementation_status=status,
             implementation_limitations=row[1].get('limitations') if row else 'No ledger claim; absence is not proof of no source',
             research_readiness='partial_contract' if indexed else 'address_or_existing_contract_only',
             implementation_readiness='blocked',confidence='confirmed' if indexed else 'unresolved',
             priority='high' if milestone=='P2.6' else 'normal',priority_reason='Persistence can constrain actor/VM identity and lifecycle' if milestone=='P2.6' else 'Upcoming milestone dependency',
             prerequisites=[],ordering_constraints=[],blockers=[blocker],provenance=provenance,
             inventory_membership='unknown',verification_levels={stage:{'status':'unknown','provenance':[]} for stage in ['source_recovery','integration','native','i386','desktop']})
        # A ledger status is a source claim only; never a fresh verification PASS.
        nodes[key]['data']['verification_levels']['source_recovery']={'status':status,'provenance':provenance[-1:] if row else []}
        return key
    for milestone in ['P2.3','P2.4','P2.5','P2.6','P2.7']:
        node('milestone:'+milestone,'milestone',label=milestone,milestones=[milestone],subsystem=None,original_addresses=[],
             owner='Application',lifetime='Delivery boundary',source_mapping=[],implementation_status='external_prerequisite' if milestone=='P2.3' else 'not_delivered',
             research_readiness='external_prerequisite' if milestone=='P2.3' else 'partial_contract',implementation_readiness='blocked',
             confidence='confirmed',priority='high' if milestone=='P2.6' else 'normal',priority_reason='User-directed forward research',
             blockers=['See subsystem contracts'],prerequisites=[],ordering_constraints=[],provenance=[prov('analysis/phase2-delivery-plan.md')])
    for sid,ms,owner,addresses,evidence,blocker in specs:
        key = 'subsystem:future:'+sid
        node(key,'subsystem',label=sid,milestones=[ms],subsystem=sid,original_addresses=sorted(addresses.split()),owner=owner,
             lifetime='See owner contract',source_mapping=[],implementation_status='mixed_existing_source_and_missing_leaves',
             research_readiness='partial_contract',implementation_readiness='blocked',confidence='confirmed',
             priority='high' if ms=='P2.6' else 'normal',priority_reason='Explicit milestone scope',prerequisites=[],ordering_constraints=[],blockers=[blocker],provenance=[prov(evidence)])
        edge('milestone:'+ms,key,'contains_subsystem',[prov(evidence)])
        for a in addresses.split():
            fk = function(a,sid,ms,owner,evidence,blocker)
            edge(key,fk,'original_function_reference',[prov(evidence)])
    # All original module persistence targets remain visible, even unresearched leaves.
    for i,slot in enumerate(modules['slots']):
        for role in ['save','load']:
            fk=function(slot[role],'snapshot','P2.6','Existing registered module slot'+str(slot['kind']),modules_path,'Persistence body must be recovered; lifecycle recovery is insufficient')
            edge('function:client:'+('0048dc90' if role=='save' else '0048de60'),fk,'virtual_dispatch',
                 [prov(modules_path,json_pointer=f'/slots/{i}')],condition=f'registered slot {slot["kind"]}, vtable+{48 if role=="save" else 52:02x}',slot=slot['kind'])
    for a,(i,r) in exports.items():
        src='function:client:'+a
        if src not in nodes: continue
        calls={c['target']:[] for c in r['direct_calls']}
        for c in r['direct_calls']: calls[c['target']].append(c['site'])
        for target,sites in sorted(calls.items()):
            dst='function:client:'+target
            if dst in nodes: edge(src,dst,'direct_call',[prov(manifest_path,json_pointer=f'/exports/{i}/direct_calls')],call_sites=sorted(sites))
    doc=[prov('analysis/future-dependencies/p2.6-save-load.md')]
    dependencies=[('milestone:P2.3','subsystem:future:preparation','Runtime acceptance requires actual actor consumers'),
      ('milestone:P2.3','subsystem:future:retention','Ordinary pause requires live owner'),
      ('subsystem:future:envelope','subsystem:future:application_persistence','Chunk framing precedes application decoding'),
      ('subsystem:future:application_persistence','subsystem:future:snapshot','Receiver reads application state before game snapshot'),
      ('subsystem:future:identity','subsystem:future:script_persistence','Frame/actor references require original identities'),
      ('subsystem:future:script_persistence','subsystem:future:snapshot','Required registered Script persistence consumer'),
      ('subsystem:future:retention','subsystem:future:snapshot','Required for retained-menu acceptance, not standalone parser'),
      ('subsystem:future:snapshot','subsystem:future:resume','Required for quick/live resume branch'),
      ('subsystem:future:application_persistence','subsystem:future:results','Debrief commits preparation save'),
      ('subsystem:future:results','subsystem:future:campaign','Actual result/progress must feed campaign presentation')]
    for src,dst,reason in dependencies: edge(src,dst,'depends_on',doc,True,condition=reason)
    edge('function:client:00476370','function:client:004764d0','data_dependency',doc,condition='Same actor slot order/population across save and replay')
    edge('subsystem:future:identity','function:client:006dece0','data_dependency',doc,condition='Objective child5000 immediately resolves actor index00475D50')
    edge('subsystem:future:identity','subsystem:future:script_persistence','ownership',doc,condition='Contexts borrow scene frames; scene outlives contexts')
    edge('subsystem:future:script_persistence','subsystem:future:identity','cleanup_order',doc,condition='Retire frame borrowers before actor/scene owner destruction')
    edge('subsystem:future:application_persistence','subsystem:future:snapshot','initialization_order',doc,condition='006C3340 reset/read preparation before0069C390/0069CFC0')
    edge('function:client:0068c300','function:client:006b12d0','initialization_order',[prov('analysis/future-dependencies/p2.7-results.md')],condition='Debrief accept/leave commit before progress before save before queued transition')
    for e in edges.values():
        if e['data']['relationship']=='depends_on': nodes[e['target']]['data']['prerequisites'].append(e['source'])
        if e['data']['relationship'] in ('initialization_order','cleanup_order'):
            nodes[e['target']]['data']['ordering_constraints'].append(e['id'])
    for n in nodes.values():
        for key in ('prerequisites','ordering_constraints','milestones'):
            if key in n['data']: n['data'][key]=sorted(set(n['data'][key]))
    inputs=['analysis/function-status.json','analysis/phase2-delivery-plan.json',manifest_path,modules_path,
            'analysis/future-dependencies/phase2-overview.md','analysis/future-dependencies/p2.6-save-load.md',
            'analysis/future-dependencies/p2.6-context-layout.md','analysis/future-dependencies/p2.7-results.md',
            'analysis/future-dependencies/implementation-guide.md','tools/progress-analyzer.md','tools/progress_graph.py','analysis/future-dependencies/export_graph.py']
    return {'schema_id':'hd2.future-dependencies','schema_version':1,'source_commit_sha':SOURCE,
            'binary':manifest['binary'],'inputs':[{'path':p,'sha256':digest(p)} for p in sorted(inputs)],
            'epistemic_policy':{'call_edge_implies_blocking':False,'dependency_direction':'prerequisite_to_dependent',
                'call_direction':'caller_to_callee','implementation_status_scope':'Pinned source SHA and exact ledger input digest',
                'research_is_implementation':False,'runtime_execution':False,'desktop_acceptance':'unknown',
                'progress_percentages':False,'confidence_values':['confirmed','inferred','unresolved']},
            'graph':{'nodes':sorted(nodes.values(),key=lambda n:n['id']),'edges':sorted(edges.values(),key=lambda e:e['id']),
                     'unknown_relationships':[{'id':'unknown:mission-a8-persistence','source':'subsystem:future:snapshot','target':None,
                       'relationship':'virtual_dispatch','slots':['10','14'],'reason':'Actual mission vtable targets and payload unresolved',
                       'confidence':'unresolved','blocking':True,'provenance':doc}]}}

if __name__=='__main__':
    output=generate()
    (HERE/'phase2-graph.json').write_text(json.dumps(output,indent=2,sort_keys=True)+'\n')
    print(f"{len(output['graph']['nodes'])} nodes, {len(output['graph']['edges'])} edges")
