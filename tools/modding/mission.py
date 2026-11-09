"""Bounded, read-only mission models and parsers. No GUI or game runtime imports."""
from __future__ import annotations
import argparse
from dataclasses import dataclass, field, fields, is_dataclass
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import struct

from script_client import inspect_bytes
from dependency_graph import enrich
from actor_fields import decode_properties, ACTOR_NAMES, FRAME_NAMES

MAX_BYTES = 64 * 1024 * 1024
MAX_RECORDS = 200_000
MAX_FILES = 20_000
SCENE_EVIDENCE = "recovered/game/mission_scene_reader.h:006CC760/006CD940"
TABLE_EVIDENCE = "recovered/game/mission_table_reader.h:00523C00/0051DE80/0051C7B0/0051C870/0051C910"
BINDING_EVIDENCE = "recovered/game/mission_script_bindings.h:00428600"
SCRIPT_EVIDENCE = "analysis/mission-script-receivers.md:00416E10 opcodes1/5E/66"
LIMITATIONS = [
    "Read-only serialized inspection; not mission loading or runtime compatibility validation.",
    "Unknown payloads remain opaque; decoded records do not imply instantiated actors or world transforms.",
    "Reference matches are local candidates only; no archive overlays, engine lookup or runtime hierarchy is reproduced.",
    "Scripts in reference folders are indexed and analyzed only on request; includes are not expanded.",
    "Byte names use Latin-1 for reversible display; original encoding is not inferred.",
]


@dataclass
class Record:
    identity: str
    offset: int
    size: int
    kind: str
    label: str
    evidence: str
    values: dict = field(default_factory=dict)
    children: list[Record] = field(default_factory=list)


@dataclass
class Reference:
    source: str
    kind: str
    value: str | None
    evidence: str
    status: str = "unresolved"
    targets: list[str] = field(default_factory=list)


@dataclass
class FileReport:
    path: str
    size: int
    format: str
    status: str = "indexed"
    sha256: str | None = None
    records: list[Record] = field(default_factory=list)
    references: list[Reference] = field(default_factory=list)
    diagnostics: list[str] = field(default_factory=list)
    script: dict | None = None
    resolution: dict | None = None
    archive_decoding: dict | None = None


@dataclass
class MissionReport:
    mission_name: str
    files: list[FileReport]
    diagnostics: list[str] = field(default_factory=list)
    limitations: list[str] = field(default_factory=lambda: list(LIMITATIONS))
    schema_version: int = 2
    entities: list = field(default_factory=list)
    graph: dict = field(default_factory=dict)
    validation: list = field(default_factory=list)
    resolution_profile: dict | None = None
    coverage: dict = field(default_factory=dict)

    def iter_json(self):
        # Shallow dataclass views avoid an additional deep copy of large documents.
        class Encoder(json.JSONEncoder):
            def default(self,value):
                if is_dataclass(value):return {f.name:getattr(value,f.name) for f in fields(value)}
                return super().default(value)
        yield from Encoder(indent=2,sort_keys=True,ensure_ascii=True,allow_nan=False).iterencode(self)
        yield "\n"

    def to_json(self):
        return "".join(self.iter_json())


class FormatError(ValueError):
    pass


class Cursor:
    def __init__(self, data, start=0, end=None):
        self.data, self.position, self.end = data, start, len(data) if end is None else end
        if not 0 <= start <= self.end <= len(data):
            raise FormatError("Invalid document bounds")

    def take(self, size):
        if size < 0 or size > self.end - self.position:
            raise FormatError(f"Truncated data at 0x{self.position:x}, need {size} bytes")
        start = self.position
        self.position += size
        return self.data[start:self.position]

    def unpack(self, fmt):
        return struct.unpack(fmt, self.take(struct.calcsize(fmt)))

    def text(self):
        end = self.data.find(b"\0", self.position, self.end)
        if end < 0:
            raise FormatError(f"Unterminated name at 0x{self.position:x}")
        return self.take(end - self.position + 1)[:-1].decode("latin-1")

    def chunk(self):
        offset = self.position
        kind, size = self.unpack("<HI")
        if size < 6:
            raise FormatError(f"Chunk smaller than header at 0x{offset:x}")
        self.take(size - 6)
        return offset, size, kind, Cursor(self.data, offset + 6, offset + size)


class Parser:
    def __init__(self, data, path):
        self.data, self.path, self.count = data, path, 0
        self.references = []

    def record(self, offset, size, kind, label, evidence, values=None):
        self.count += 1
        if self.count > MAX_RECORDS:
            raise FormatError("Record budget exceeded")
        return Record(f"{self.path}@0x{offset:x}", offset, size, kind, label, evidence, values or {})

    def ref(self, node, kind, value, evidence):
        self.references.append(Reference(node.identity, kind, value, evidence))

    def opaque(self, offset, size, kind, label="opaque payload", evidence=SCENE_EVIDENCE):
        return self.record(offset, size, kind, label, evidence,
                           {"sha256": hashlib.sha256(memoryview(self.data)[offset:offset+size]).hexdigest()})

    def scene(self):
        cursor = Cursor(self.data)
        offset, size, kind, payload = cursor.chunk()
        if kind != 0x4c53:
            raise FormatError("Expected Scene2 0x4c53 root")
        root = self.record(offset, size, "0x4c53", "scene document", SCENE_EVIDENCE)
        root.children = self.scene_children(payload, "root")
        roots = [root]
        if cursor.position < cursor.end:
            roots.append(self.opaque(cursor.position, cursor.end-cursor.position, "trailing", "trailing bytes ignored by loader"))
        return roots

    def scene_children(self, cursor, context):
        nodes = []
        actor_type = None
        containers = {("root",0x4000):("objects","object container"),
                      ("objects",0x4010):("object","object record"),
                      ("root",0xae20):("actors","actor container"),
                      ("actors",0xae21):("actor","actor record"),
                      ("object",0x4020):("parent","parent reference"),
                      ("object",0x4060):("sound","sound properties")}
        texts = {("object",0x10):"object_name", ("object",0x2012):"model_resource",
                 ("parent",0x10):"parent_frame", ("actor",0xae23):"actor_frame",
                 ("sound",0x10):"sound_resource", ("sound",0xb200):"sound_sector",
                 ("sound",0xb300):"sound_area"}
        floats = {0x20:("position",3),0x22:("rotation_wxyz",4),
                  0x2c:("fallback_position",3),0x2d:("scale",3)}
        while cursor.position < cursor.end:
            offset, size, kind, payload = cursor.chunk()
            key = context, kind
            node = self.opaque(offset, size, f"0x{kind:04x}")
            if key in containers:
                next_context, node.label = containers[key]
                node.values = {}
                node.children = self.scene_children(payload, next_context)
                if next_context=="actor":
                    names=[c.values["text"] for c in node.children if c.label=="actor_frame"]
                    types=[c.values["value"] for c in node.children if c.label=="actor_type"]
                    node.values={"frame_names":names,"serialized_types":types,
                                 "type_labels":[ACTOR_NAMES.get(t,"Unknown") for t in types]}
                elif next_context=="object":
                    node.values={"names":[c.values["text"] for c in node.children if c.label=="object_name"],
                                 "creation_kinds":[c.values["value"] for c in node.children if c.label=="creation_kind"]}
            elif key in texts:
                node.label = texts[key]
                value = payload.text()
                node.values = {"text":value, "unconsumed_bytes":payload.end-payload.position}
                if node.label != "object_name":
                    self.ref(node, node.label, value, SCENE_EVIDENCE)
            elif context == "object" and kind in (0x4011,0x4012):
                node.label = "creation_kind" if kind == 0x4011 else "creation_subtype"
                node.values = {"value":payload.unpack("<I")[0], "unconsumed_bytes":payload.end-payload.position}
                if kind==0x4011:node.values["label"]=FRAME_NAMES.get(node.values["value"],"Unknown")
            elif context == "object" and kind in floats:
                node.label, count = floats[kind]
                values = payload.unpack(f"<{count}f")
                if not all(math.isfinite(v) for v in values):
                    raise FormatError(f"Nonfinite {node.label} at 0x{offset:x}")
                node.values = {"serialized":list(values),"unconsumed_bytes":payload.end-payload.position}
            elif context == "actor" and kind == 0xae22:
                node.label = "actor_type"
                actor_type=payload.unpack("<I")[0]
                node.values = {"value":actor_type,"type_label":ACTOR_NAMES.get(actor_type,"Unknown"),"unconsumed_bytes":payload.end-payload.position}
            elif context == "actor" and kind == 0xae24:
                node.label = "actor properties"
                node.values.update(decode_properties(payload,actor_type))
            elif context == "root" and kind == 1:
                node.label = "scene version"
                version, revision = payload.unpack("<HH")
                node.values = {"version":version,"revision":revision}
                if version != 7:
                    raise FormatError(f"Unsupported Scene2 version {version}")
            elif context == "root" and kind == 0xae40:
                node.label = "mission tables"
                node.values = {}
                node.children = self.tables(payload)
            nodes.append(node)
        return nodes

    def bindings(self):
        cursor = Cursor(self.data)
        offset, size, kind, payload = cursor.chunk()
        root = self.record(offset,size,f"0x{kind:04x}","script binding pairs",BINDING_EVIDENCE)
        while payload.position < payload.end:
            offset, size, kind, name = payload.chunk()
            # Native formatted read bounds names to at most 999 bytes.
            name.end = min(name.end, name.position + 999)
            frame_name = name.text()
            end_offset, end_size, script_kind, script = payload.chunk()
            script.end = min(script.end, script.position + 999)
            script_name = script.text()
            node = self.record(offset,end_offset+end_size-offset,"binding","frame → script",BINDING_EVIDENCE,
                               {"frame":frame_name,"script":script_name,
                                "frame_chunk_id":kind,"script_chunk_id":script_kind,"script_chunk_offset":end_offset})
            self.ref(node,"actor_frame",frame_name,BINDING_EVIDENCE)
            self.ref(node,"script",script_name,BINDING_EVIDENCE)
            root.children.append(node)
        records = [root]
        if cursor.position < cursor.end:
            records.append(self.opaque(cursor.position,cursor.end-cursor.position,"trailing","trailing binding bytes ignored by loader",BINDING_EVIDENCE))
        return records

    def tables(self, cursor):
        nodes=[]
        for family in range(2):
            offset=cursor.position
            header=cursor.unpack("<7I")
            version,size,count,capacity,vectors,frames,actors=header
            if version != 0x7ed00109:
                raise FormatError(f"Unsupported mission table version 0x{version:x} at 0x{offset:x}")
            if size < 28 or size-28 > cursor.end-cursor.position:
                raise FormatError(f"Invalid mission table size at 0x{offset:x}")
            end=offset+size
            data=Cursor(self.data,cursor.position,end)
            cursor.take(size-28)
            if any(c>65535 for c in (count,vectors,frames,actors)) or capacity>count*64:
                raise FormatError("Mission table counts exceed recovered bounds")
            if count*32+vectors*18+(frames+actors)*3 > end-data.position:
                raise FormatError("Mission table counts exceed payload")
            block=self.record(offset,size,"table","mission table set",TABLE_EVIDENCE,
                              {"set":family,"version":version,"record_count":count,"link_capacity":capacity,
                               "vector_count":vectors,"frame_reference_count":frames,"actor_reference_count":actors})
            used=0
            for index in range(count):
                start=data.position
                raw=data.take(32)
                flags=struct.unpack_from("<H",raw,20)[0]
                links=flags&63
                used+=links+bool(flags&64)
                if used>capacity:
                    raise FormatError("Mission table link capacity exceeded")
                pairs=[list(data.unpack("<HH")) for _ in range(links)]
                block.children.append(self.record(start,data.position-start,"table_record","serialized table record",TABLE_EVIDENCE,
                    {"index":index,"prefix_hex":raw[:20].hex(),"flags":flags,"suffix_hex":raw[22:].hex(),
                     "links":pairs,"synthesized_sentinel":bool(flags&64)}))
            for index in range(vectors):
                start=data.position
                value,*words=data.unpack("<H4I")
                block.children.append(self.record(start,18,"vector_record","18-byte record",TABLE_EVIDENCE,
                                                  {"index":index,"value":value,"raw_words":words}))
            for kind,total in (("table_frame",frames),("table_actor",actors)):
                for index in range(total):
                    start=data.position
                    value=data.unpack("<H")[0]
                    name=data.text()
                    node=self.record(start,data.position-start,kind,"table name reference",TABLE_EVIDENCE,
                                     {"index":index,"value":value,"name":name})
                    block.children.append(node)
                    self.ref(node,kind,name,TABLE_EVIDENCE)
            if data.position != end:
                raise FormatError(f"Mission table did not consume its block at 0x{data.position:x}")
            nodes.append(block)
        if cursor.position < cursor.end:
            nodes.append(self.opaque(cursor.position,cursor.end-cursor.position,"trailing","unconsumed table suffix",TABLE_EVIDENCE))
            cursor.position=cursor.end
        return nodes


def identify(path):
    name=Path(path).name.lower()
    if name in ("scene2.bin","actors.bin","sounds.bin"): return "scene"
    if name in ("scripts.dta","mpscripts.dta"): return "bindings"
    if name == "check2.bin": return "tables"
    if name.endswith(".scr"): return "script"
    return "opaque"


def parse_file(path, identity, inspector=None):
    size=path.stat().st_size
    if identify(identity)=="opaque":return FileReport(identity,size,"opaque",status="unsupported")
    if size>MAX_BYTES:
        return FileReport(identity,size,identify(identity),status="limited",diagnostics=["File exceeds 64 MiB inspection limit"])
    with path.open("rb") as stream:data=stream.read(MAX_BYTES+1)
    return parse_data(data,identity,inspector)


def parse_data(data,identity,inspector=None):
    size=len(data)
    report=FileReport(identity,size,identify(identity))
    if report.format == "opaque":
        report.status="unsupported"
        return report
    if size>MAX_BYTES:
        report.status="limited"
        report.diagnostics=["File exceeds 64 MiB inspection limit"]
        return report
    try:
        if len(data)>MAX_BYTES: raise FormatError("File exceeds inspection limit")
        report.size=len(data)
        report.sha256=hashlib.sha256(data).hexdigest()
        parser=Parser(data,identity)
        if report.format == "script":
            if inspector is None:
                report.status="indexed"
                report.diagnostics=["Source indexed; select to run the standalone inspector"]
                return report
            script=inspect_bytes(inspector,data,identity)
            report.script=script
            report.status="syntax_accepted" if script["syntax_accepted"] else "syntax_unavailable"
            report.diagnostics=[script["diagnostic"]] if script["diagnostic"] else []
            for ref in script["references"]:
                report.references.append(Reference(f"{identity}#pc={ref['pc']}",ref["kind"],ref["literal"],SCRIPT_EVIDENCE))
            return report
        if report.format == "scene":report.records=parser.scene()
        elif report.format == "bindings":report.records=parser.bindings()
        elif report.format == "tables":report.records=parser.tables(Cursor(data))
        report.references=parser.references
        report.status="decoded_subset"
    except (OSError,ValueError,RuntimeError,struct.error,OverflowError) as exc:
        report.records=[];report.references=[];report.script=None
        report.status="inspection_error" if report.format=="script" else "malformed_or_unsupported"
        report.diagnostics=[str(exc)]
    return report


def walk_records(nodes):
    for node in nodes:
        yield node
        yield from walk_records(node.children)


class RetentionBudget:
    """Bound retained document models across a session, not just individual reads."""
    def __init__(self,records=200_000,instructions=100_000):
        self.record_limit=records;self.instruction_limit=instructions
        self.entries={};self.records=0;self.instructions=0

    def admit(self,entry):
        old_records,old_instructions=self.entries.get(entry.path,(0,0))
        records=sum(1 for _ in walk_records(entry.records))
        instructions=len(entry.script.get("instructions",[])) if entry.script else 0
        if (self.records-old_records+records>self.record_limit
                or self.instructions-old_instructions+instructions>self.instruction_limit):
            entry.records=[];entry.references=[];entry.script=None
            entry.status="limited";entry.diagnostics=["Session retained-record/instruction budget exceeded"]
            records=instructions=0
        self.records+=records-old_records;self.instructions+=instructions-old_instructions
        self.entries[entry.path]=(records,instructions)
        return entry


def safe_name(value):
    if not value or "\0" in value:return None
    text=value.replace("\\","/")
    p=PurePosixPath(text)
    if p.is_absolute() or ".." in p.parts or ":" in text:return None
    return str(p)


def link_references(report):
    frames={}
    files={}
    for entry in report.files:
        for node in walk_records(entry.records):
            if node.label=="object_name":frames.setdefault(node.values["text"],[]).append(node.identity)
        files.setdefault(entry.path.split("/",1)[-1].lower(),[]).append(entry.path)
    for entry in report.files:
        for ref in entry.references:
            ref.targets=[]
            if ref.value is None:ref.status="dynamic";continue
            # Engine frame names are identifiers, not filesystem paths. Do not
            # impose host path syntax on a verbatim serialized name candidate.
            if ref.kind in ("actor_frame","parent_frame","scene_frame","table_frame","table_actor","sound_sector","sound_area"):
                ref.targets=list(frames.get(ref.value,[]))
                ref.status="candidate" if len(ref.targets)==1 else "ambiguous_candidates" if ref.targets else "empty_reference" if not ref.value else "unresolved_in_supplied_folders"
                continue
            if ref.kind=="child_frame":
                ref.status="requires_runtime_parent";continue
            value=safe_name(ref.value)
            if value is None:ref.status="empty_reference" if not ref.value else "empty_or_unsafe";continue
            if ref.kind=="script":
                #00427730 appends only in absence of exact .scr or .SCR substring.
                script=value if ".scr" in value or ".SCR" in value else value+".scr"
                expected=[script.lower(),f"scripts/{report.mission_name}/{script}".lower(),
                          f"{report.mission_name}/{script}".lower()]
                ref.targets=sorted({p for key in expected for p in files.get(key,[])})
            elif ref.kind in ("model_resource","sound_resource"):
                # Explicit suffix/name candidates, never the engine's cache selection.
                for path,items in files.items():
                    if path==value.lower() or path.endswith("/"+value.lower()):ref.targets+=items
                    elif ref.kind=="model_resource" and Path(path).suffix.lower()==".4ds":
                        key=value.rsplit(".",1)[0] if "." in value else value
                        if str(PurePosixPath(path).with_suffix("")).endswith("/"+key.lower()) or str(PurePosixPath(path).with_suffix(""))==key.lower():ref.targets+=items
                ref.targets=sorted(set(ref.targets))
            ref.status="candidate" if len(ref.targets)==1 else "ambiguous_candidates" if ref.targets else "unresolved_in_supplied_folders"


class MissionSession:
    """Local roots are private UI context; deterministic exports use tagged relative paths."""
    def __init__(self, root, reference_roots=(), inspector=None):
        self.roots=[Path(root).resolve(),*[Path(p).resolve() for p in reference_roots]]
        if not all(p.is_dir() for p in self.roots):raise ValueError("Select existing local directories")
        self.inspector=inspector
        self.budget=RetentionBudget()
        self.paths={}
        self.report=MissionReport(self.roots[0].name,[])
        self.scan()

    def scan(self):
        for number,root in enumerate(self.roots):
            tag="mission" if number==0 else f"reference{number}"
            def error(exc):self.report.diagnostics.append(str(exc))
            for directory,dirs,files in os.walk(root,followlinks=False,onerror=error):
                # No links: references cannot make inspection escape a selected folder.
                dirs[:]=sorted(d for d in dirs if not (Path(directory)/d).is_symlink())
                for filename in sorted(files):
                    path=Path(directory)/filename
                    if path.is_symlink() or not path.is_file():continue
                    if len(self.paths)>=MAX_FILES:
                        self.report.diagnostics.append("File limit reached; inventory is incomplete")
                        self.report.files.sort(key=lambda f:f.path)
                        link_references(self.report)
                        enrich(self.report)
                        return
                    identity=f"{tag}/{path.relative_to(root).as_posix()}"
                    self.paths[identity]=path
                    try:
                        if number==0:self.report.files.append(self.budget.admit(parse_file(path,identity,self.inspector)))
                        else:self.report.files.append(FileReport(identity,path.stat().st_size,identify(identity)))
                    except OSError as exc:self.report.diagnostics.append(f"{identity}: {exc}")
        self.report.files.sort(key=lambda f:f.path)
        link_references(self.report)
        enrich(self.report)

    def inspect(self, identity):
        path=self.paths[identity]
        # Recheck containment and links before every explicit read.
        root=self.roots[0] if identity.startswith("mission/") else self.roots[int(identity.split("/",1)[0][9:])]
        relative=path.relative_to(root)
        if (not path.resolve().is_relative_to(root)
                or any(root.joinpath(*relative.parts[:n]).is_symlink() for n in range(1,len(relative.parts)+1))):
            raise ValueError("Reference left selected root or became a symbolic link")
        fresh=self.budget.admit(parse_file(path,identity,self.inspector))
        self.report.files=[fresh if f.path==identity else f for f in self.report.files]
        link_references(self.report)
        enrich(self.report)
        return fresh

    def export(self, destination):
        destination=Path(destination)
        resolved=destination.resolve()
        if any(resolved.is_relative_to(root) for root in self.roots):
            raise ValueError("Save reports outside all inspected folders")
        if destination.exists() and any(destination.samefile(p) for p in self.paths.values() if p.exists()):
            raise ValueError("Cannot overwrite an inspected file or alias")
        # Exclusive create also protects unrelated existing files and hardlinks.
        with destination.open("x",encoding="utf-8") as stream:
            stream.writelines(self.report.iter_json())


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mission",type=Path)
    parser.add_argument("--reference-root",type=Path,action="append",default=[])
    parser.add_argument("--inspector",type=Path)
    parser.add_argument("--output",type=Path)
    args=parser.parse_args()
    try:
        session=MissionSession(args.mission,args.reference_root,args.inspector)
        if args.output:session.export(args.output)
        else:
            import sys
            sys.stdout.writelines(session.report.iter_json())
    except (ValueError,OSError) as exc:parser.exit(2,f"{exc}\n")


if __name__=="__main__":main()
