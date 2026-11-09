"""Read-only virtual resource index with explicit original dtaOpen lookup profiles.

ISD0 payload codecs reuse the existing extractor. ISD1 names are indexed from
previously exported metadata contracts; payload loading deliberately rejects.
"""
from __future__ import annotations
from dataclasses import asdict, dataclass
from pathlib import Path, PurePosixPath
import os
import struct
import sys

# Reuse existing verified codecs rather than create another decompressor.
sys.path.append(str(Path(__file__).resolve().parent.parent))
from extract_dta_member import read_index, extract_member, read_exact, xor_record, Member

EVIDENCE = "analysis/startup-archives.md:10004B8C/1000C110/006DB6E0"
INDEX_EVIDENCE = "rw_data 10003763/1000C110: ISD1 index/name metadata only"
ARCHIVES = [
    ("Patch.dta",0x10acb252,0x5d805270), ("Models.dta",0x10acb252,0x5d805259),
    ("Missions.dta",0x22bcda98,0x7654a3f0), ("Maps.dta",0xf26527fa,0xb438d0a5),
    ("Sounds.dta",0x8d2965ca,0x4fe85106), ("Scripts.dta",0xcf761298,0x0ab4e72d),
    ("Others.dta",0x65f7ab23,0xea85902a), ("Maps_C.dta",0xe26520fa,0xb038d0a0),
    ("Maps_U.dta",0xf26520fa,0xb038d1a1),
]
MAX_ENTRIES = 200_000
MAX_PAYLOAD = 64*1024*1024


def virtual_name(value):
    text=value.replace("\\","/")
    path=PurePosixPath(text)
    if not text or path.is_absolute() or ".." in path.parts or ":" in text or "\0" in text:
        raise ValueError("Unsafe resource path")
    return str(path).lower()


@dataclass(frozen=True)
class Source:
    virtual_path: str
    origin: str
    format: str
    size: int
    header_offset: int | None = None
    data_offset: int | None = None


@dataclass
class Resolution:
    virtual_path: str
    selected: Source | None
    shadowed: list[Source]
    status: str
    evidence: str = EVIDENCE
    diagnostic: str = ""


class ResolutionError(ValueError):
    pass


def isd1_index(stream, key):
    """10003763 header/table +1000C110 names; no payload/CRC equivalence claim."""
    stream.seek(0,2);length=stream.tell();stream.seek(0)
    if read_exact(stream,4)!=b"ISD1":raise ValueError("Not an ISD1 archive")
    count,offset,size,_,_=struct.unpack("<5I",xor_record(read_exact(stream,20),key))
    if count>100_000 or size!=count*28 or offset<24 or offset+size!=length:
        raise ValueError("Invalid ISD1 header/index bounds")
    stream.seek(offset);table=xor_record(read_exact(stream,size),key)
    names=[];seen=set()
    for index in range(count):
        name_hash,header_offset,data_offset,_=struct.unpack_from("<III16s",table,index*28)
        if not 24<=header_offset<=offset-32:raise ValueError("Invalid ISD1 member header")
        stream.seek(header_offset);header=xor_record(read_exact(stream,32),key)
        flags=struct.unpack_from("<H",header,28)[0];name_length=flags&0x7fff
        if not name_length or name_length>4096 or header_offset+32+name_length>offset:
            raise ValueError("Invalid ISD1 member name bounds")
        raw=xor_record(read_exact(stream,name_length),key)
        name=raw.decode("ascii");normalized=virtual_name(name)
        # Original lookup hashes uppercased path length and byte sum.
        lowered=name.upper().encode("ascii")
        if name_hash != ((len(lowered)<<16)|(sum(lowered)&0xffff)):
            raise ValueError("ISD1 name/index hash mismatch")
        if normalized in seen:raise ValueError("Duplicate ISD1 resource name")
        seen.add(normalized)
        unpacked,blocks=struct.unpack_from("<II",header,16)
        if data_offset>offset or blocks>1_000_000:raise ValueError("Invalid ISD1 member bounds")
        names.append((normalized,unpacked,header_offset,data_offset))
    return names


class ResourceStore:
    def __init__(self, root, archives_first=False, language="English"):
        self.root=Path(root).resolve()
        if not self.root.is_dir():raise ValueError("Select an installation directory")
        if not language.isalpha() or not language.isascii():raise ValueError("Language must be an ASCII name")
        self.archives_first=archives_first
        self.language=language
        self.entries={}
        self.paths={}
        self.members={}
        self.archive_paths={}
        self.archive_stamps={}
        self.errors=[]
        self.barriers=[]
        self.archive_summary=[]
        self._index_loose()
        # Original selected-language creation fallback precedes Patch.
        wanted="Lang"+language+".dta"
        selected=self._loose_path(wanted)
        if selected is None:
            wanted="LangEnglish.dta";selected=self._loose_path(wanted)
        specs=([(wanted,0xa0a0a0a0,0xa0a0a0a0)] if selected else [])+ARCHIVES
        for rank,(name,high,low) in enumerate(specs):
            path=self._loose_path(name)
            if path is None:continue
            self.archive_paths[name]=path
            stat=path.stat();self.archive_stamps[name]=(stat.st_size,stat.st_mtime_ns)
            key=struct.pack("<II",low,high)
            try:
                with path.open("rb") as stream:
                    signature=read_exact(stream,4);stream.seek(0)
                    if signature==b"ISD0":
                        members=read_index(stream,key)
                        for member in members:
                            identity=virtual_name(member.name)
                            self._add(Source(identity,name,"ISD0",member.size,None,member.data_offset),rank)
                            self.members[(name,identity)]=(member,key)
                        count=len(members)
                    elif signature==b"ISD1":
                        members=isd1_index(stream,key)
                        for identity,size,header,data in members:
                            self._add(Source(identity,name,"ISD1",size,header,data),rank)
                        count=len(members)
                    else:raise ValueError("Unsupported archive signature")
                self.archive_summary.append({"name":name,"format":signature.decode(),"members":count,
                    "payload_support":signature==b"ISD0","evidence":EVIDENCE,
                    "integrity":"bounded structural/name checks; original ISD1 CRCs not validated" if signature==b"ISD1" else "existing ISD0 reader"})
            except (ValueError,OSError,UnicodeError,struct.error) as exc:
                self.barriers.append((rank,name))
                self.errors.append(f"{name}: {exc}; later archive resolution cannot be trusted")

    def _loose_path(self, name):
        candidates=self.paths.get(virtual_name(name),[])
        if len(candidates)>1:raise ResolutionError(f"Case-colliding loose file: {name}")
        return candidates[0] if candidates else None

    def _add(self, source, rank):
        count=getattr(self,"_entry_count",0)
        if count>=MAX_ENTRIES:raise ValueError("Resource inventory limit exceeded")
        self._entry_count=count+1
        self.entries.setdefault(source.virtual_path,[]).append((rank,source))

    def _index_loose(self):
        count=0
        def error(exc):raise ResolutionError(str(exc))
        for directory,dirs,files in os.walk(self.root,followlinks=False,onerror=error):
            dirs[:]=sorted(d for d in dirs if not (Path(directory)/d).is_symlink())
            for filename in sorted(files):
                path=Path(directory)/filename
                if path.is_symlink() or not path.is_file():continue
                count+=1
                if count>MAX_ENTRIES:raise ValueError("Loose inventory limit exceeded")
                key=virtual_name(path.relative_to(self.root).as_posix())
                self.paths.setdefault(key,[]).append(path)
                self._add(Source(key,"loose","loose",path.stat().st_size),-1)

    def resolution(self, name):
        key=virtual_name(name)
        candidates=self.entries.get(key,[])
        loose=[s for rank,s in candidates if rank==-1]
        archives=sorted(((rank,s) for rank,s in candidates if rank>=0),key=lambda item:item[0])
        if len(loose)>1 and not (self.archives_first and archives):
            return Resolution(key,None,[s for _,s in candidates],"blocked",diagnostic="Case-colliding loose paths")
        if not self.archives_first and loose:
            return Resolution(key,loose[0],[s for _,s in archives],"selected")
        rank=archives[0][0] if archives else float("inf")
        if any(b_rank<=rank for b_rank,_ in self.barriers):
            return Resolution(key,None,[s for _,s in candidates],"blocked",diagnostic="An earlier archive could not be indexed")
        if archives:
            chosen=archives[0][1]
            return Resolution(key,chosen,[s for _,s in archives[1:]]+loose,
                              "selected" if chosen.format=="ISD0" else "selected_payload_unsupported",
                              diagnostic="ISD1 payload decoder unavailable; lower-priority resources were not substituted" if chosen.format=="ISD1" else "")
        if len(loose)==1:return Resolution(key,loose[0],[],"selected")
        return Resolution(key,None,[],"missing")

    def read(self,name):
        resolution=self.resolution(name)
        source=resolution.selected
        if resolution.status!="selected" or source is None:
            raise ResolutionError(resolution.diagnostic or resolution.status)
        if source.size>MAX_PAYLOAD:raise ResolutionError("Selected payload exceeds 64 MiB limit")
        if source.origin=="loose":
            path=self._loose_path(source.virtual_path)
            self._check(path)
            with path.open("rb") as stream:data=stream.read(MAX_PAYLOAD+1)
        else:
            path=self.archive_paths[source.origin];self._check(path)
            stat=path.stat()
            if (stat.st_size,stat.st_mtime_ns)!=self.archive_stamps[source.origin]:
                raise ResolutionError("Archive changed since indexing; refresh installation")
            member,key=self.members[(source.origin,source.virtual_path)]
            with path.open("rb") as stream:data=extract_member(stream,member,key)
        if len(data)>MAX_PAYLOAD:raise ResolutionError("Selected payload exceeds 64 MiB limit")
        return data

    def _check(self,path):
        relative=path.relative_to(self.root)
        if not path.resolve().is_relative_to(self.root) or any(self.root.joinpath(*relative.parts[:n]).is_symlink() for n in range(1,len(relative.parts)+1)):
            raise ResolutionError("Resource left installation or became a symbolic link")

    def missions(self):
        names={}
        for name in self.entries:
            parts=name.split("/")
            if len(parts)==3 and parts[0]=="missions" and parts[2] in ("scene2.bin","actors.bin","scripts.dta","check2.bin"):
                names.setdefault(parts[1],set()).add(parts[2])
        return [{"name":name,"recognized_files":sorted(files),
                 "scene_resolution":asdict(self.resolution(f"missions/{name}/scene2.bin"))}
                for name,files in sorted(names.items())]

    def profile(self):
        return {"name":"base startup registration", "archives_first":self.archives_first,
                "low_byte_flags":1 if self.archives_first else 0,"override_byte":0,
                "language":self.language,"evidence":EVIDENCE,"archives":self.archive_summary,
                "diagnostics":self.errors,
                "limitations":["Explicit lookup profile, not a capture of live global state.",
                    "Expansion/PatchX01 registration and ISD1 payload codecs unsupported.",
                    "ISD1 names are structurally indexed; original CRC verification not implemented."]}
