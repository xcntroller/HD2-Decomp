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
    if not isinstance(result, dict) or result.get("schema_version") != 1:
        raise RuntimeError("Unsupported inspector JSON schema; expected version 1.")
    if args and args[0] == "--catalog":
        if process.returncode or not all(isinstance(result.get(k), list) for k in ("commands", "predicates")):
            raise RuntimeError("Invalid command catalog.")
    elif (type(result.get("syntax_accepted")) is not bool
          or result["syntax_accepted"] != (process.returncode == 0)
          or not all(isinstance(result.get(k), list) for k in (
              "variables", "instructions", "references", "lexical_mentions", "limitations", "event_slots"))):
        raise RuntimeError("Invalid inspection result.")
    return result


def inspect_bytes(inspector, source, identity):
    with tempfile.TemporaryDirectory(prefix="hd2-inspection-") as directory:
        snapshot = Path(directory) / "source.scr"
        snapshot.write_bytes(source[:65535])
        report = invoke(inspector, "--json", snapshot)
    report["file"] = str(identity)
    return report


def inspect_file(inspector, path):
    with Path(path).open("rb") as stream:
        source = stream.read(65535)
    return source, inspect_bytes(inspector, source, path)
