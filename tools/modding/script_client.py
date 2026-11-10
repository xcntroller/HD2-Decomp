"""Tk-independent client for the inspector's prepared-source JSON protocol."""
import json
from pathlib import Path
import subprocess
import tempfile


def invoke(inspector, *args):
    try:
        process = subprocess.run([str(inspector), *map(str, args)], capture_output=True,
                                 encoding="utf-8", errors="replace", timeout=20, check=False)
    except FileNotFoundError as exc:
        raise RuntimeError("Inspector not found. Build it with make -C tools/modding, "
                           "or select it using --inspector PATH.") from exc
    if process.returncode not in (0, 1):
        raise RuntimeError(process.stderr.strip() or f"Inspector exited with {process.returncode}")
    try:
        result = json.loads(process.stdout)
    except ValueError as exc:
        raise RuntimeError("Inspector returned invalid JSON.") from exc
    if not isinstance(result, dict) or result.get("schema_version") != 2:
        raise RuntimeError("Unsupported inspector JSON schema; expected version 2.")
    if args and args[0] == "--lex-one":
        if process.returncode or not isinstance(result.get("span"),dict):raise RuntimeError("Invalid lexer result")
    elif args and args[0] == "--catalog":
        if process.returncode or not all(isinstance(result.get(k), list) for k in ("commands", "predicates")):
            raise RuntimeError("Invalid command catalog.")
    elif (type(result.get("syntax_accepted")) is not bool
          or result["syntax_accepted"] != (process.returncode == 0)
          or not all(isinstance(result.get(k), list) for k in (
              "variables", "instructions", "references", "lexical_mentions", "limitations", "event_slots",
              "tokens", "statements", "arguments", "diagnostics", "whenevers"))):
        raise RuntimeError("Invalid inspection result.")
    return result


def lex_one(inspector,source,cursor):
    if cursor is None:return {"token":"","span":{"start":len(source),"end":len(source)},"cursor":None}
    with tempfile.TemporaryDirectory(prefix="hd2-lexer-") as directory:
        snapshot=Path(directory)/"source.scr";snapshot.write_bytes(source)
        return invoke(inspector,"--lex-one",cursor,snapshot)


def inspect_bytes(inspector, source, identity, context=None):
    from script_source import expand
    from script_analysis import enrich_script
    mapping,includes,diagnostics=expand(inspector,source,str(identity),context,lex_one)
    with tempfile.TemporaryDirectory(prefix="hd2-inspection-") as directory:
        snapshot = Path(directory) / "source.scr"
        # Native input is bounded; retain the original length diagnostic below.
        snapshot.write_bytes(mapping.data[:65536])
        report = invoke(inspector, "--json", "--prepared", snapshot)
    report["file"] = str(identity)
    if any(d['severity']=='error' for d in diagnostics):
        report.update(syntax_accepted=False,diagnostic=next(d['message'] for d in diagnostics if d['severity']=='error'))
        for key in ('variables','instructions','references','event_slots','whenevers','statements','arguments','diagnostics'):report[key]=[]
        report['event_tables']={k:[] for k in report['event_tables']}
        report.update(start_pc=-1,whenever_count=0)
    report=enrich_script(report,mapping)
    # Preprocessing errors already retain their pre-insertion provenance.
    report['diagnostics']=diagnostics+report['diagnostics']
    report['includes']=includes
    report['include_context']=context.profile() if context else None
    report['preprocessing_status']='rejected' if any(d['severity']=='error' for d in diagnostics) else 'expanded' if includes else 'no_includes'
    return report


def inspect_file(inspector, path, context=None):
    with Path(path).open("rb") as stream:
        source = stream.read(65536)
    return source, inspect_bytes(inspector, source, path, context)
