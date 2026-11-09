"""Mission sessions backed by the original resource-lookup profile, without Tk."""
from __future__ import annotations
from dataclasses import asdict
from pathlib import Path, PurePosixPath

from mission import (MissionReport, FileReport, parse_data, identify, safe_name,
                     link_references, enrich, RetentionBudget)
from resource_store import ResourceStore, ResolutionError, virtual_name
from isd1 import ArchiveError, UnsupportedCompression, ArchiveLimitError

MAX_DEPENDENCIES=2048
MAX_SCRIPTS=512


class InstallationSession:
    def __init__(self,store,mission,inspector=None):
        if mission not in {m['name'] for m in store.missions()}:raise ValueError("Unknown discovered mission")
        self.store=store;self.inspector=inspector;self.budget=RetentionBudget()
        self.roots=[store.root];self.paths={};self._files={}
        self.report=MissionReport(mission,[])
        self.report.resolution_profile=store.profile()
        self.report.limitations=[
            "Only documented serialized subsets are parsed; no runtime compatibility claim.",
            "Explicit resource profile, not a capture of live game flags or global state.",
            "ISD1 raw/LZSS payloads are checksum-verified; unsupported sound tags and invalid resources never fall back.",
            "Engine frame/cache relationships remain candidates; includes and dynamic data flow are unresolved.",
            "Dependency expansion is bounded to 2048 files / 512 script analyses; opaque resources are not decoded."]
        prefix=f"missions/{mission}/"
        for name in sorted(store.entries):
            if name.startswith(prefix):self._ensure(name,identify(name)!="opaque")
        for name in sorted(store.entries):
            if name.startswith(f"scripts/{mission}/") and name.endswith(".scr"):self._ensure(name)
        self._dependencies()
        self._finish()

    @staticmethod
    def identity(name):return "resource/"+virtual_name(name)

    def _ensure(self,name,parse=False):
        name=virtual_name(name);identity=self.identity(name)
        if identity in self._files:
            entry=self._files[identity]
            return self._parse(entry) if parse and entry.status=="indexed" else entry
        if len(self._files)>=MAX_DEPENDENCIES:
            if "Dependency file budget reached" not in self.report.diagnostics:self.report.diagnostics.append("Dependency file budget reached")
            return None
        resolution=self.store.resolution(name)
        if resolution.status=="missing":return None
        source=resolution.selected
        entry=FileReport(identity,source.size if source else 0,identify(name),resolution=asdict(resolution))
        self.paths[identity]=name
        self._files[identity]=entry
        if resolution.status=="invalid":
            entry.status="invalid_resource";entry.diagnostics=[resolution.diagnostic]
            entry.archive_decoding={"status":"invalid","error_kind":resolution.diagnostic_code}
        elif resolution.status=="blocked":
            entry.status="unavailable_resource";entry.diagnostics=[resolution.diagnostic]
            entry.archive_decoding={"status":"unavailable","error_kind":resolution.diagnostic_code}
        elif parse:return self._parse(entry)
        elif resolution.status=="selected_payload_unsupported":
            entry.status="unavailable_resource";entry.diagnostics=[resolution.diagnostic]
            entry.archive_decoding={"status":"unavailable","error_kind":"unsupported_compression","stored_payload_checksums":"not_read"}
        elif entry.format=="opaque":entry.status="unsupported"
        return entry

    def _parse(self,entry):
        try:
            result=self.store.read_result(self.paths[entry.path])
            fresh=parse_data(result.data,entry.path,self.inspector)
            import hashlib
            fresh.sha256=hashlib.sha256(result.data).hexdigest()
            fresh.archive_decoding=result.decoding
            fresh.resolution=entry.resolution
            self.budget.admit(fresh)
            self._files[entry.path]=fresh
            return fresh
        except (ValueError,OSError) as exc:
            kind=getattr(exc,"kind","unavailable")
            invalid=kind in ("invalid","integrity_error","bounds_error")
            entry.status="invalid_resource" if invalid else "limited" if kind=="inspection_limit" else "unavailable_resource"
            entry.diagnostics=[str(exc)]
            entry.archive_decoding={"status":"invalid" if invalid else "unavailable","error_kind":kind}
            if isinstance(exc,UnsupportedCompression):
                entry.archive_decoding.update(stored_payload_checksums="verified",codecs=list(exc.tags))
            entry.records=[];entry.references=[];entry.script=None;entry.sha256=None
            return entry

    def _resource_candidates(self,kind,value):
        key=safe_name(value)
        if not key:return []
        key=key.lower()
        if not hasattr(self,"_resource_index"):
            self._resource_index={}
            for name in self.store.entries:
                if name.startswith("models/") and name.endswith(".4ds"):
                    stem=str(PurePosixPath(name).with_suffix(""))
                    for alias in {stem,stem[7:],PurePosixPath(stem).name}:
                        self._resource_index.setdefault(alias,[]).append(name)
        if kind=="model_resource":
            key=key.rsplit(".",1)[0] if "." in key else key
            return sorted(set(self._resource_index.get(key,[])))
        # Sound identifiers are not guessed to have a .wav extension.
        candidates=[n for n in self.store.entries if n==key or n.endswith("/"+key) and n.startswith("sounds/")]
        return sorted(candidates)

    def _dependencies(self):
        visited=set();scripts=0;analyzed=set()
        while True:
            pending=[f for name,f in sorted(self._files.items()) if name not in visited]
            if not pending:break
            for entry in pending:
                entry=self._files[entry.path]
                visited.add(entry.path)
                for ref in entry.references:
                    if ref.value is None:continue
                    value=safe_name(ref.value)
                    if not value:continue
                    if ref.kind=="script":
                        script=value if ".scr" in value or ".SCR" in value else value+".scr"
                        name=f"scripts/{self.report.mission_name}/{script}"
                        identity=self.identity(name)
                        existing=self._files.get(identity)
                        if identity not in analyzed and (existing is None or existing.status=="indexed") and scripts<MAX_SCRIPTS:
                            result=self._ensure(name,True)
                            analyzed.add(identity)
                            if result:
                                scripts+=1
                                visited.discard(identity)
                        elif scripts>=MAX_SCRIPTS and "Script analysis budget reached" not in self.report.diagnostics:
                            self.report.diagnostics.append("Script analysis budget reached")
                    elif ref.kind in ("model_resource","sound_resource"):
                        names=self._resource_candidates(ref.kind,value)
                        if len(names)>64:
                            self.report.diagnostics.append(f"Candidate limit exceeded for {ref.kind}; match remains incomplete")
                        for name in names[:64]:self._ensure(name)

    def _finish(self):
        self.report.files=sorted(self._files.values(),key=lambda f:f.path)
        link_references(self.report)
        for entry in self.report.files:
            for ref in entry.references:
                if ref.kind!="script" or ref.value is None:continue
                value=safe_name(ref.value)
                if not value:continue
                script=value if ".scr" in value or ".SCR" in value else value+".scr"
                name=f"scripts/{self.report.mission_name}/{script}"
                resolution=self.store.resolution(name)
                identity=self.identity(name)
                if resolution.selected and identity in self._files:
                    ref.targets=[identity]
                    ref.status=("selected_payload_unsupported" if resolution.status!="selected" else
                                "selected_resource_invalid" if self._files[identity].status=="invalid_resource" else
                                "selected_resource_unavailable" if self._files[identity].status in ("resolution_blocked","unavailable_resource","limited","inspection_error") else "resolved_resource")
        enrich(self.report)

    def inspect(self,identity):
        if identity not in self._files:raise KeyError(identity)
        entry=self._files[identity]
        # Metadata selection remains visible even for unsupported selected payloads.
        selected=entry.resolution.get("selected") if entry.resolution else None
        if entry.status=="indexed" or (entry.archive_decoding is None or entry.status=="unavailable_resource") and selected and selected["format"]=="ISD1":
            entry=self._parse(entry)
        self._dependencies();self._finish()
        return self._files[identity]

    def script_bytes(self,identity):return self.store.read(self.paths[identity])[:65535]

    def export(self,destination):
        path=Path(destination)
        if path.resolve().is_relative_to(self.store.root):raise ValueError("Save exports outside the installation")
        # No overwrites, including aliases of original files.
        with path.open("x",encoding="utf-8") as stream:stream.writelines(self.report.iter_json())


def main():
    import argparse,sys,json
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("installation",type=Path)
    parser.add_argument("--mission")
    parser.add_argument("--archives-first",action="store_true")
    parser.add_argument("--language",default="English")
    parser.add_argument("--inspector",type=Path)
    parser.add_argument("--output",type=Path)
    args=parser.parse_args()
    try:
        store=ResourceStore(args.installation,args.archives_first,args.language)
        if args.mission:
            session=InstallationSession(store,args.mission.lower(),args.inspector)
            if args.output:session.export(args.output)
            else:sys.stdout.writelines(session.report.iter_json())
        else:print(json.dumps({"schema_version":2,"missions":store.missions(),"resolution_profile":store.profile()},indent=2,sort_keys=True))
    except (ValueError,OSError) as exc:parser.exit(2,f"{exc}\n")


if __name__=="__main__":main()
