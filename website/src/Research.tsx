import { useEffect, useMemo, useState } from 'react';
import { ReactFlow, Background, Controls, MarkerType, Position, type Edge } from '@xyflow/react';
import '@xyflow/react/dist/style.css';
import { PageHeading, EvidenceLinks } from './components';
import { relationshipLayout } from './graphLayout';
import type { Evidence } from './data';
type Facts = Record<string, unknown> & { label?: string; provenance?: Evidence[]; evidence?: Evidence[] };
type RecordNode = {id:string;type:string;snapshots:Record<string,Facts>};
type Relation = {id:string;source:string;target:string;snapshots:Record<string,Facts>};
type Batch = {id:string;label:string;predecessors:string[];prerequisite_text?:string;conditional_predecessors?:string[];node_ids?:string[];readiness:string;purpose:string;contract:string;addresses:string[];evidence:Evidence[]};
type ResearchData = {available:boolean;research_checkpoint_commit:string;implementation_commit:string;research_implementation_commit:string;research_graph_sha256:string;nodes:RecordNode[];edges:Relation[];batches:Batch[];implementation_groups:{id:string;member_ids:string[]}[];diagnostics:unknown[];unknown_relationships:Facts[];verification:Facts;verification_receipts?:{record:Facts;evidence:Evidence[]}[]};
const facts = (n:RecordNode) => n.snapshots.research ?? n.snapshots.implementation;
const title = (n:RecordNode) => String(facts(n).label ?? n.id);
const blocking = (d:Facts) => d.blocking === true && d.relationship === 'depends_on' && d.evidence_state === 'verified';
const evidenceFields = new Set(['provenance', 'evidence', 'source_evidence']);
function FactValue({ value }: { value: unknown }) {
  if (value === null || value === undefined) return <span className="muted">Unknown</span>;
  if (Array.isArray(value)) {
    return value.length
      ? <ul className="research-value-list">{value.map((item, i) => <li key={i}><FactValue value={item} /></li>)}</ul>
      : <span className="muted">None recorded</span>;
  }
  if (typeof value === 'object') {
    const entries = Object.entries(value);
    return entries.length ? <dl className="research-facts research-nested-facts">{entries.map(([key, item]) =>
      <div key={key}><dt>{key.replaceAll('_', ' ')}</dt><dd>{evidenceFields.has(key) && Array.isArray(item)
        ? <EvidenceLinks items={item as Evidence[]} />
        : <FactValue value={item} />}</dd></div>
    )}</dl> : <span className="muted">No details recorded</span>;
  }
  return <>{typeof value === 'boolean' ? (value ? 'Yes' : 'No') : String(value)}</>;
}
function Details({ value }: { value: Facts }) {
  return <>
    <dl className="research-facts">{Object.entries(value).filter(([key]) => !evidenceFields.has(key)).map(([key, item]) =>
      <div key={key}><dt>{key.replaceAll('_', ' ')}</dt><dd><FactValue value={item} /></dd></div>
    )}</dl>
    <EvidenceLinks items={value.provenance ?? value.evidence ?? []} />
    {!!(value.source_evidence as Evidence[] | undefined)?.length && <EvidenceLinks items={value.source_evidence as Evidence[]} />}
  </>;
}
export default function Research({dark}:{dark:boolean}) {
 const [data,setData]=useState<ResearchData>(); const [error,setError]=useState('');
 const [mode,setMode]=useState('overview'); const [sourceScope,setSourceScope]=useState('research'); const [milestone,setMilestone]=useState('P2.6'); const [subsystem,setSubsystem]=useState(''); const [query,setQuery]=useState(''); const [kind,setKind]=useState(''); const [relation,setRelation]=useState(''); const [evidence,setEvidence]=useState(''); const [status,setStatus]=useState(''); const [statusSource,setStatusSource]=useState('research'); const [onlyBlocking,setOnlyBlocking]=useState(false); const [limit,setLimit]=useState(40); const [selected,setSelected]=useState(''); const [batchId,setBatchId]=useState('');
 useEffect(()=>{const controller=new AbortController(); fetch(`${import.meta.env.BASE_URL}data/research.json`,{signal:controller.signal}).then(r=>{if(!r.ok)throw Error('Research snapshot unavailable');return r.json()}).then(setData).catch(e=>{if(e.name!=='AbortError')setError(String(e))});return ()=>controller.abort()},[]);
 const lookup=useMemo(()=>new Map(data?.nodes.map(n=>[n.id,n])??[]),[data]);
 if(error)return <p role="alert">{error}</p>; if(!data)return <p role="status">Loading intelligence records…</p>; if(!data.available)return <p>No versioned research graph in this source tree.</p>;
 const batch=data.batches.find(b=>b.id===batchId);
 const researchNodes=data.nodes.filter(n=>n.snapshots.research);
 const milestoneIds=researchNodes.filter(n=>n.type==='milestone').map(n=>n.id.replace('milestone:','')).sort();
 const researchedMilestones=milestoneIds.filter(id=>facts(lookup.get('milestone:'+id)!).research_readiness!=='external_prerequisite');
 const implementationMilestoneMembers=new Set(data.implementation_groups.find(g=>g.id===`milestone:${milestone}`)?.member_ids??[]);
 const displayFacts=(n:RecordNode) => sourceScope==='implementation' ? n.snapshots.implementation??facts(n) : facts(n);
 const batchRelatedIds=new Set(batch ? data.nodes.filter(n=>{const d=facts(n);return batch.node_ids?.length ? batch.node_ids.includes(n.id) : ((d.original_addresses as string[]|undefined)??[String(n.snapshots.implementation?.original_entry??'')]).some(a=>batch.addresses.includes(a))}).map(n=>n.id):[]);
 if(batch){batchRelatedIds.add(`milestone:P2.${batch.label.slice(2,3)}`);for(const g of data.implementation_groups)if(g.member_ids.some(id=>batchRelatedIds.has(id)))batchRelatedIds.add(g.id)}
 const matching=data.nodes.filter(n=>sourceScope==='combined'||!!n.snapshots[sourceScope]).filter(n=>{const d=displayFacts(n); return (!milestone || ((d.milestones as string[]|undefined)?.includes(milestone) || n.id===`milestone:${milestone}` || (sourceScope!=='research'&&implementationMilestoneMembers.has(n.id)))) && (!subsystem || d.subsystem===subsystem) && (!kind ? n.type!=='function' || !!query || !!batch : n.type===kind) && (!status || (statusSource==='research' ? n.snapshots.research?.implementation_status : n.snapshots.implementation?.status)===status) && (!evidence || d.confidence===evidence) && (!batch || batchRelatedIds.has(n.id)) && (!query || [n.id,d.label,d.subsystem,...(d.original_addresses as string[]??[]),d.original_entry].join(' ').toLowerCase().includes(query.toLowerCase().replace(/^0x/,'')));});
 const visible=matching.slice(0,limit); const ids=new Set(visible.map(n=>n.id));
 const relations=data.edges.filter(e=>ids.has(e.source)&&ids.has(e.target)&&Object.entries(e.snapshots).filter(([source])=>sourceScope==='combined'||source===sourceScope).some(([,d])=>(!relation||d.relationship===relation)&&(!evidence || d.confidence===evidence || (evidence==='confirmed' && d.evidence_state==='verified'))&&(!onlyBlocking||blocking(d))));
 // Conditional dependencies shape the visual topology without becoming scheduling claims.
 const dependencyLinks=relations.filter(e=>Object.entries(e.snapshots).some(([source,d])=>(sourceScope==='combined'||source===sourceScope)&&d.relationship==='depends_on'));
 const positions=relationshipLayout(visible.map(n=>n.id),dependencyLinks.length?dependencyLinks:relations);
 const nodes=visible.map(n=>({id:n.id,position:positions.get(n.id)!,sourcePosition:Position.Right,targetPosition:Position.Left,data:{label:`${n.type} · ${String(displayFacts(n).label??n.id)}`},selected:selected===n.id,style:{background:'var(--panel)',color:'var(--text)',border:'1px solid #748067',width:265}}));
 const batchLinks=data.batches.flatMap(b=>b.predecessors.map(p=>({id:p+'>'+b.label,source:p,target:b.label,markerEnd:{type:MarkerType.ArrowClosed},type:'smoothstep',style:{strokeDasharray:b.conditional_predecessors?.includes(p)?'5 5':undefined}})));
 const externalMilestones=[...new Set(batchLinks.map(e=>e.source))].filter(id=>!data.batches.some(b=>b.label===id));
 const batchPositions=relationshipLayout([...data.batches.map(b=>b.label),...externalMilestones],batchLinks);
 const batchNodes=[...data.batches.map(b=>({id:b.label,label:b.label+' · '+b.readiness})),...externalMilestones.map(id=>({id,label:id+' · external milestone prerequisite'}))].map(n=>({id:n.id,position:batchPositions.get(n.id)!,sourcePosition:Position.Right,targetPosition:Position.Left,data:{label:n.label},selected:batchId==='batch:'+n.id,style:{background:'var(--panel)',color:'var(--text)',width:265,border:'1px solid #748067'}}));
 const edges:Edge[]=relations.map(e=>{const d=(sourceScope==='implementation'?e.snapshots.implementation:e.snapshots.research)??e.snapshots.implementation;return {id:e.id,source:e.source,target:e.target,type:'smoothstep',label:String(d.relationship),markerEnd:{type:MarkerType.ArrowClosed},style:{stroke:selected===e.source||selected===e.target?'#c8b87c':'#73846c',strokeWidth:selected===e.source||selected===e.target?3:1.3,strokeDasharray:blocking(d)?undefined:'5 5'}}});
 const node=lookup.get(selected); const edge=data.edges.find(e=>e.id===selected);
 const select=(label:string,value:string,change:(v:string)=>void,options:string[]) => <label>{label}<select aria-label={label} value={value} onChange={e=>{change(e.target.value);setLimit(40);setSelected('')}}><option value="">All / default</option>{options.map(v=><option key={v}>{v}</option>)}</select></label>;
 return <><PageHeading eyebrow="Research / intelligence board" title="Future reconstruction research">Original behavior, constraints and proposed implementation batches for the milestones in the current research snapshot. Research readiness does not establish implementation or desktop acceptance.</PageHeading>
 <div className="toolbar">{['overview','graph','batches'].map(v=><button className="button secondary" aria-pressed={mode===v} onClick={()=>setMode(v)} key={v}>{v}</button>)}<a href="#/dependencies">Implementation graph</a><a href="#/roadmap">Main roadmap</a></div>
 <section className="panel research-snapshot"><p>Research checkpoint: <code>{data.research_checkpoint_commit}</code></p><p>Current implementation: <code>{data.implementation_commit}</code></p><p>Research implementation snapshot: <code>{data.research_implementation_commit}</code></p><p>These snapshots are separate. {data.diagnostics.length} stale-input or conflicting-state diagnostics.</p><details><summary>Snapshot diagnostics and validation provenance</summary><pre className="research-json">{JSON.stringify({diagnostics:data.diagnostics,verification:data.verification,graph_digest:data.research_graph_sha256},null,2)}</pre></details></section>
 {mode==='overview'? <><div className="research-grid">{researchedMilestones.map(m=>{const n=lookup.get(`milestone:${m}`);return <article className="panel" key={m}><h2>{m}</h2>{n&&<><h3>{String(n.snapshots.implementation?.label??title(n))}</h3><p>Research: {String(facts(n).research_readiness??'unknown')}</p><p>Implementation readiness: {String(facts(n).implementation_readiness??'unknown')}</p><p>Research snapshot implementation: {String(facts(n).implementation_status??'unknown')}</p><h4>Focus areas</h4><ul>{researchNodes.filter(v=>v.type==='subsystem'&&(facts(v).milestones as string[]|undefined)?.includes(m)).map(v=><li key={v.id}>{title(v)} — {String(facts(v).research_readiness??'unknown')}</li>)}</ul><p>{(facts(n).blockers as string[]|undefined)?.join('; ')}</p><EvidenceLinks items={facts(n).provenance??[]}/></>}<button className="button secondary" onClick={()=>{setMilestone(m);setMode('graph')}}>Explore dependencies</button></article>})}</div><section className="panel"><h2>Unresolved original-code questions</h2>{data.unknown_relationships.map((v,i)=><Details key={i} value={v}/>)}<h2>Research checkpoints</h2>{!!data.verification_receipts?.length&&<EvidenceLinks items={data.verification_receipts.flatMap(r=>r.evidence)}/>}<p>{String(data.verification.scope)}. Implementation readiness: {String(data.verification.implementation_readiness ?? data.verification.readiness ?? 'unknown')}.</p><details><summary>Recorded verification checks</summary><Details value={data.verification}/></details></section></> : <>
 <div className="toolbar graph-toolbar">
 {select('Graph sources',sourceScope,v=>{setSourceScope(v||'research');setMilestone('')},['research','implementation','combined'])}
 {select('Milestone',milestone,setMilestone,milestoneIds)}
 {select('Subsystem',subsystem,setSubsystem,[...new Set(researchNodes.map(n=>String(facts(n).subsystem??'')).filter(Boolean))].sort())}
 {select('Batch',batchId,v=>{setBatchId(v);setMilestone('');setSubsystem('');setKind('');setStatus('');setEvidence('');setQuery('')},data.batches.map(b=>b.id))}
 {select('Node type',kind,setKind,['milestone','subsystem','function'])}
 {select('Relationship',relation,setRelation,[...new Set(data.edges.flatMap(e=>Object.values(e.snapshots).map(d=>String(d.relationship))))].sort())}
 {select('Evidence confidence',evidence,setEvidence,['confirmed','inferred','unresolved'])}
 {select('Implementation status snapshot',statusSource,v=>{setStatusSource(v||'research');setStatus('')},['research','implementation'])}
 {select('Implementation state',status,setStatus,[...new Set(researchNodes.map(n=>String(statusSource==='research' ? facts(n).implementation_status??'' : n.snapshots.implementation?.status??'unknown')))].filter(Boolean).sort())}
 <label>Search address, ID, label or subsystem<input value={query} onChange={e=>{setQuery(e.target.value);setLimit(40);setSelected('')}}/></label>
 <label className="checkbox-control"><input type="checkbox" checked={onlyBlocking} onChange={e=>setOnlyBlocking(e.target.checked)}/> Verified blocking dependencies only</label>
 </div>
 {mode==='batches'&&<section className="panel"><h2>Ordered implementation batches</h2><p>Branches and prerequisite arrows below come from the implementation guide; placement is a visual aid. No batch is credited as implemented.</p><div className="graph-panel" style={{height:420}}><ReactFlow nodes={batchNodes} edges={batchLinks} fitView minZoom={0.08} maxZoom={2} fitViewOptions={{padding:0.16,minZoom:0.08,maxZoom:1}} nodesConnectable={false} nodesDraggable={false} deleteKeyCode={null} colorMode={dark?'dark':'light'} onNodeClick={(_,n)=>{if(n.id.startsWith('P2.')){setMilestone(n.id);setMode('graph');setBatchId('')}else{setBatchId('batch:'+n.id);setMilestone('');setSubsystem('');setKind('');setStatus('');setEvidence('');setQuery('');setSelected('')}}}><Background/><Controls showInteractive={false}/></ReactFlow></div><div className="function-chips">{data.batches.filter(b=>!query||JSON.stringify(b).toLowerCase().includes(query.toLowerCase())).map(b=><button className="tag" key={b.id} onClick={()=>{setBatchId(b.id);setSelected('');setMilestone('');setSubsystem('');setKind('');setStatus('');setEvidence('');setQuery('')}}>{b.label} ← {b.predecessors.join(', ')||'no source-recovery predecessor'}</button>)}</div>{batch&&<><h3>{batch.label}: {batch.purpose}</h3><p>Research: {batch.readiness}</p><p>Prerequisites and owner constraints: {batch.prerequisite_text ?? batch.predecessors.join(", ")}</p><p>Implementation: inspect current function records; no whole-batch delivery claim.</p><p className="research-contract">{batch.contract}</p><EvidenceLinks items={batch.evidence}/></>}</section>}
 <p>{visible.length} / {matching.length} matching nodes · {relations.length} relationships. Functions appear on explicit type selection, search or batch selection. Batch matches use declared research associations when supplied, with guide address mentions as a fallback; they are not a complete implementation inventory. Positions follow visible relationship structure; they are not a schedule or critical path. Conditional dependencies retain their conditions in the inspector. Arrow meanings remain source-specific.</p>
 <div className="dependency-layout"><section className="panel graph-panel" aria-label="Research dependency graph"><ReactFlow key={`${sourceScope}:${milestone}:${subsystem}:${kind}:${query}:${limit}:${batchId}:${relation}:${evidence}:${status}:${onlyBlocking}`} nodes={nodes} edges={edges} fitView minZoom={0.08} maxZoom={2} fitViewOptions={{padding:0.16,minZoom:0.08,maxZoom:1}} nodesDraggable={false} nodesConnectable={false} deleteKeyCode={null} colorMode={dark?'dark':'light'} onNodeClick={(_,n)=>setSelected(n.id)} onEdgeClick={(_,e)=>setSelected(e.id)}><Background/><Controls showInteractive={false}/></ReactFlow></section><aside className="panel inspector graph-inspector"><label>Inspect record<select aria-label="Inspect record" value={selected} onChange={e=>setSelected(e.target.value)}><option value="">Choose node or relationship</option>{visible.map(n=><option key={n.id} value={n.id}>{title(n)}</option>)}{relations.map(e=><option key={e.id} value={e.id}>{e.source} → {e.target}</option>)}</select></label>{node?<><h3>{title(node)}</h3><code>{node.id}</code>{Object.entries(node.snapshots).map(([s,d])=><section key={s}><h4>{s} snapshot</h4><Details value={d}/></section>)}<h4>Related research batches</h4>{data.batches.filter(b=>b.node_ids?.length ? b.node_ids.includes(node.id) : (facts(node).original_addresses as string[]|undefined)?.some(a=>b.addresses.includes(a))).map(b=><button className="tag" key={b.id} onClick={()=>{setBatchId(b.id);setMode('batches')}}>{b.label}</button>)}<h4>Incoming / outgoing relationships</h4>{data.edges.filter(e=>e.source===node.id||e.target===node.id).map(e=><p key={e.id}><button className="text-button" onClick={()=>setSelected(e.id)}>{e.source} → {e.target}: {String((e.snapshots.research??e.snapshots.implementation).relationship)}</button></p>)}</>:edge?<><h3>Directed relationship</h3><p>{edge.source} → {edge.target}</p>{Object.entries(edge.snapshots).map(([s,d])=><section key={s}><h4>{s}</h4><Details value={d}/></section>)}</>:<p>Select a record to inspect original addresses, ownership, blockers, evidence, verification and both implementation snapshots.</p>}</aside></div>
 {matching.length>limit&&<button className="button secondary" disabled={limit>=150} onClick={()=>setLimit(Math.min(150,limit+30))}>Reveal more (maximum 150; narrow filters for further records)</button>}
 <details className="panel"><summary>Explicit unknown relationships ({data.unknown_relationships.length})</summary>{data.unknown_relationships.map((v,i)=><Details key={i} value={v}/>)}</details>
 </> }</>;
}
