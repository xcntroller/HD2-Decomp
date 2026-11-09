"""Evidence-labelled mission entities, relationships and validation, without Tk."""
from __future__ import annotations
from dataclasses import dataclass
import hashlib
import json


@dataclass
class Entity:
    identity: str
    file: str
    category: str
    label: str
    offset: int
    type_id: int | None
    type_label: str
    property_records: list[str]


@dataclass
class Node:
    identity: str
    file: str
    category: str
    label: str
    offset: int | None = None
    type_label: str = ""


@dataclass
class Edge:
    identity: str
    source: str
    target: str
    relation: str
    label: str
    certainty: str
    status: str
    evidence: str


@dataclass
class Issue:
    identity: str
    severity: str
    code: str
    file: str
    source: str
    message: str
    evidence: str


def walk(nodes):
    for node in nodes:
        yield node
        yield from walk(node.children)


def stable(*values):
    return hashlib.sha256(json.dumps(values,ensure_ascii=True,separators=(",",":")).encode()).hexdigest()[:24]


def enrich(report):
    nodes={};edges=[];entities=[];issues=[];owners={}
    def issue(severity,code,file,source,message,evidence):
        issues.append(Issue("issue:"+stable(code,source,message),severity,code,file,source,message,evidence))
    def edge(source,target,relation,label,certainty,status,evidence):
        edges.append(Edge("edge:"+stable(source,target,relation,label),source,target,relation,label,certainty,status,evidence))
    for entry in report.files:
        nodes[entry.path]=Node(entry.path,entry.path,"file",entry.path)
        if entry.status in ("malformed_or_unsupported","inspection_error","resolution_blocked","limited"):
            for diagnostic in entry.diagnostics:issue("error","input_unavailable",entry.path,entry.path,diagnostic,entry.format)
        elif entry.status=="syntax_unavailable":
            for diagnostic in entry.diagnostics:issue("warning","script_syntax_unavailable",entry.path,entry.path,diagnostic,"0042C850")
        elif entry.status=="unsupported":issue("info","unsupported_file",entry.path,entry.path,"Payload format not decoded",entry.format)
        def visit(records,owner=None):
            for record in records:
                current=owner
                if record.label in ("object record","actor record","frame → script"):
                    category={"object record":"object","actor record":"actor","frame → script":"binding"}[record.label]
                    if category=="actor":
                        names=record.values.get("frame_names",[]);types=record.values.get("serialized_types",[])
                        labels=record.values.get("type_labels",[])
                    elif category=="object":
                        names=record.values.get("names",[]);types=record.values.get("creation_kinds",[])
                        from actor_fields import FRAME_NAMES
                        labels=[FRAME_NAMES.get(t,"Unknown") for t in types]
                    else:names=[record.values.get("frame","")];types=[];labels=[]
                    label=" / ".join(names) or f"unnamed {category} at 0x{record.offset:x}"
                    properties=[c.identity for c in record.children if c.label=="actor properties"]
                    entity=Entity(record.identity,entry.path,category,label,record.offset,
                                  types[0] if len(types)==1 else None,
                                  labels[0] if len(labels)==1 else "Multiple serialized types" if labels else "Unspecified",properties)
                    entities.append(entity);nodes[record.identity]=Node(record.identity,entry.path,category,label,record.offset,entity.type_label)
                    current=record.identity
                    edge(entry.path,current,"contains",label,"serialized","documented",record.evidence)
                    if len(types)>1:
                        issue("warning","multiple_serialized_types",entry.path,current,"Record has multiple type declarations; no single runtime type inferred",record.evidence)
                    if category=="actor" and entity.type_label=="Unknown":
                        issue("warning","unsupported_actor_type",entry.path,current,f"Actor type {entity.type_id} has no known label/decoder",record.evidence)
                owners[record.identity]=current or entry.path
                if record.label=="actor properties":
                    status=record.values.get("decode_status")
                    if status in ("malformed","unsupported_actor_type","unsupported_property_decoder","unsupported_version","partial_unknown_chunks"):
                        issue("error" if status=="malformed" else "warning",status,entry.path,record.identity,
                              record.values.get("diagnostic",status),record.values.get("evidence",record.evidence))
                    if record.values.get("unconsumed_bytes"):
                        issue("info","unconsumed_property_bytes",entry.path,record.identity,
                              f"{record.values['unconsumed_bytes']} bytes remain uninterpreted",record.evidence)
                elif "sha256" in record.values:
                    issue("info","opaque_record",entry.path,record.identity,
                          f"{record.kind}: {record.size} byte span remains opaque",record.evidence)
                visit(record.children,current)
        visit(entry.records)
    for entry in report.files:
        for index,ref in enumerate(entry.references):
            source=owners.get(ref.source,ref.source if "#pc=" in ref.source else entry.path)
            if source not in nodes:
                nodes[source]=Node(source,entry.path,"script_instruction",source.rsplit("#",1)[-1])
                edge(entry.path,source,"contains",nodes[source].label,"serialized","documented",ref.evidence)
            targets=ref.targets
            if not targets:
                target="reference:"+stable(ref.source,index,ref.kind,ref.value)
                nodes[target]=Node(target,entry.path,"unresolved_reference",ref.value or "<dynamic>")
                targets=[target]
            for target in targets:
                mapped=owners.get(target,target)
                if mapped not in nodes:nodes[mapped]=Node(mapped,mapped,"resource",mapped)
                certainty="resolver_selected" if ref.status in ("resolved_resource","selected_payload_unsupported","selected_resource_unavailable") else "name_candidate" if ref.targets else "serialized_unresolved"
                edge(source,mapped,ref.kind,ref.value or "<dynamic>",certainty,ref.status,ref.evidence)
            if ref.status not in ("candidate","resolved_resource"):
                severity="info" if ref.status in ("dynamic","requires_runtime_parent","empty_or_unsafe","empty_reference") else "warning"
                issue(severity,"reference_"+ref.status,entry.path,ref.source,
                      f"{ref.kind}: {ref.value if ref.value is not None else '<dynamic>'}",ref.evidence)
    for diagnostic in report.diagnostics:
        issue("warning","session_diagnostic","","mission:"+report.mission_name,diagnostic,"inspection limits")
    if report.resolution_profile:
        for diagnostic in report.resolution_profile.get("diagnostics",[]):
            issue("error","resolution_profile_incomplete","","mission:"+report.mission_name,diagnostic,"10004B8C/1000C110")
    report.entities=sorted(entities,key=lambda e:e.identity)
    report.graph={"nodes":sorted(nodes.values(),key=lambda n:n.identity),"edges":sorted({e.identity:e for e in edges}.values(),key=lambda e:e.identity)}
    report.validation=sorted({i.identity:i for i in issues}.values(),key=lambda i:(i.file,i.source,i.code,i.identity))
    from collections import Counter
    report.coverage={"complete_format_support":False,"file_statuses":dict(sorted(Counter(f.status for f in report.files).items())),
                     "validation_severities":dict(sorted(Counter(i.severity for i in report.validation).items())),
                     "opaque_records":sum(i.code=="opaque_record" for i in report.validation)}


def search_graph(report, query="", limit=300, offset=0):
    """Search node/edge labels, IDs, kinds, provenance and validation status."""
    terms=query.casefold().split()
    lookup={node.identity:node for node in report.graph.get("nodes",[])}
    matched=[];total=0
    for edge in report.graph.get("edges",[]):
        haystack=" ".join((edge.source,edge.target,edge.relation,edge.label,edge.certainty,edge.status,edge.evidence,
                           lookup[edge.source].label,lookup[edge.target].label,lookup[edge.source].category,lookup[edge.target].category,
                           lookup[edge.source].type_label,lookup[edge.target].type_label)).casefold()
        if all(term in haystack for term in terms):
            if offset<=total<offset+limit:matched.append(edge)
            total+=1
    return total,matched
