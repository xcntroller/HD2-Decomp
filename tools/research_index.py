#!/usr/bin/env python3
"""Local, incremental address/evidence lookup. Never runs Ghidra or game code.

See research-index.md. All generated content stays in an ignored SQLite cache;
the selected --root is only read, including its uncommitted and ignored evidence.
"""

import argparse
from collections import Counter, defaultdict
import hashlib
import json
import os
from pathlib import Path
import re
import sqlite3
import sys
import time

TOOL_ROOT = Path(__file__).resolve().parents[1]
SCHEMA = "4"
HASHES = {
    "client": "8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307",
    "engine": "12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee",
}
PROGRAMS = {value: key for key, value in HASHES.items()}
TEXT_SUFFIXES = {".md", ".h", ".cpp", ".c", ".py", ".java", ".json", ".asm"}
WORK_SUFFIXES = {".md", ".c", ".json", ".asm", ".java"}
SKIP_DIRS = {".git", "__pycache__", "node_modules", "build", ".venv"}
MAX_BYTES = 8 * 1024 * 1024
ADDRESS = re.compile(r"(?<![0-9a-f])(?:0x)?([0-9a-f]{8})(?![0-9a-f])", re.I)
COMPACT = re.compile(r"[g-z_][a-z_]*(00[0-9a-f]{6})(?![0-9a-f])", re.I)
ASM_ADDRESS = re.compile(r"^\s*([0-9a-f]{6,8}):", re.I)
LONG_HEX = re.compile(r"[0-9a-f]{32,}", re.I)
POLICY = ("Lookup candidates, not equivalence or delivery proof. Text mentions are "
          "program-unqualified. Export dates do not establish correctness; old exports "
          "lack a complete Ghidra type/options revision, so reuse needs review. "
          "Caller/callee links are export-reported, not an exhaustive call graph.")


def address(value):
    value = re.sub(r"^(?:0x|FUN_|sub_)", "", value, flags=re.I)
    if not re.fullmatch(r"[0-9a-fA-F]{6,8}", value):
        raise ValueError(f"Expected a 6..8 digit hex address: {value}")
    return value.lower().zfill(8)


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def safe_file(root, name):
    """Reject traversal and every symlink component, including in-root links."""
    relative = Path(name)
    if relative.is_absolute() or ".." in relative.parts:
        raise ValueError(f"Expected a root-relative path: {name}")
    path = root
    for part in relative.parts:
        path = path / part
        if path.is_symlink():
            raise ValueError(f"Symlink is outside index scope: {name}")
    if not path.is_file():
        raise ValueError(f"Missing regular file: {name}")
    return path


def corpus(root):
    """Allowlist research/source roots; skip downloaded tooling, assets and captures."""
    for name in ("AGENTS.md", "HANDOFF.md", "plan.md", "README.md", "RESEARCH.md"):
        p = root / name
        if p.is_file() and not p.is_symlink():
            yield name, p
    for name in ("recovered", "analysis", "tools", "docs"):
        base = root / name
        if not base.is_dir() or base.is_symlink():
            continue
        for directory, dirs, files in os.walk(base, followlinks=False):
            dirs[:] = sorted(d for d in dirs if d not in SKIP_DIRS
                             and not (Path(directory) / d).is_symlink()
                             and (Path(directory) / d).name != "research-index")
            for filename in sorted(files):
                p = Path(directory) / filename
                relative = p.relative_to(root)
                work = relative.parts[:2] in (("analysis", "work"), ("analysis", "ghidra"))
                if p.suffix.lower() in (WORK_SUFFIXES if work else TEXT_SUFFIXES) and not p.is_symlink():
                    yield relative.as_posix(), p


def kind(name):
    if name.startswith("analysis/checkpoints/"):
        return "receipt"
    if name.startswith(("analysis/work/", "analysis/ghidra/")):
        if Path(name).suffix == ".c":
            return "decompilation" if re.fullmatch(r"[0-9a-fA-F]{8}", Path(name).stem) else "helper"
        return {".asm": "assembly", ".json": "metadata"}.get(Path(name).suffix, "helper")
    if name.startswith("recovered/"):
        return "source"
    if name.startswith("tools/"):
        return "helper"
    return "note" if name.endswith(".md") else "metadata"


def connect(db, root, read_only=False):
    if not read_only:
        db.parent.mkdir(parents=True, exist_ok=True)
    con = sqlite3.connect(db.resolve().as_uri() + "?mode=ro", uri=True, timeout=30) if read_only else sqlite3.connect(db, timeout=30)
    con.row_factory = sqlite3.Row
    con.execute("PRAGMA foreign_keys=ON")
    if not read_only:
        con.execute("CREATE TABLE IF NOT EXISTS meta(key TEXT PRIMARY KEY,value TEXT)")
    metadata = dict(con.execute("SELECT key,value FROM meta"))
    if metadata and (metadata.get("schema") != SCHEMA or metadata.get("root") != str(root)):
        con.close()
        raise ValueError("Cache belongs to another root/schema; choose a new --db path")
    if read_only:
        if not metadata:
            con.close()
            raise ValueError("Cache has no snapshot; run index first")
        return con
    con.executescript("""
        CREATE TABLE IF NOT EXISTS files(
            id INTEGER PRIMARY KEY,path TEXT UNIQUE,kind TEXT,mtime_ns INTEGER,ctime_ns INTEGER,size INTEGER,
            digest TEXT,error TEXT);
        CREATE TABLE IF NOT EXISTS mentions(
            file_id INTEGER REFERENCES files(id) ON DELETE CASCADE,entry TEXT,line INTEGER,
            section TEXT,excerpt TEXT,PRIMARY KEY(entry,file_id,line)) WITHOUT ROWID;
        CREATE INDEX IF NOT EXISTS mentions_file ON mentions(file_id);
        CREATE VIRTUAL TABLE IF NOT EXISTS passages USING fts5(
            path UNINDEXED,kind UNINDEXED,line UNINDEXED,end_line UNINDEXED,
            section UNINDEXED,body,tokenize='unicode61');
        CREATE TABLE IF NOT EXISTS passage_files(
            id INTEGER PRIMARY KEY,path TEXT REFERENCES files(path) ON DELETE CASCADE);
        CREATE INDEX IF NOT EXISTS passage_files_path ON passage_files(path);
        CREATE TABLE IF NOT EXISTS records(
            path TEXT REFERENCES files(path) ON DELETE CASCADE,category TEXT,program TEXT,
            entry TEXT,data TEXT,PRIMARY KEY(path,category,program,entry));
        CREATE INDEX IF NOT EXISTS records_entry ON records(category,program,entry);
        CREATE TABLE IF NOT EXISTS ranges(
            path TEXT REFERENCES files(path) ON DELETE CASCADE,program TEXT,entry TEXT,
            first INTEGER,last INTEGER);
        CREATE INDEX IF NOT EXISTS ranges_address ON ranges(program,first,last);
        CREATE INDEX IF NOT EXISTS ranges_path ON ranges(path);
        CREATE TABLE IF NOT EXISTS links(
            path TEXT REFERENCES files(path) ON DELETE CASCADE,program TEXT,entry TEXT,
            direction TEXT,target TEXT,name TEXT);
        CREATE INDEX IF NOT EXISTS links_entry ON links(program,entry,direction);
        CREATE INDEX IF NOT EXISTS links_path ON links(path);
    """)
    con.executemany("INSERT OR REPLACE INTO meta VALUES (?,?)",
                    [("root", str(root)), ("schema", SCHEMA)])
    con.commit()
    return con


def put_record(con, name, category, program, entry, data):
    con.execute("INSERT OR REPLACE INTO records VALUES (?,?,?,?,?)",
                (name, category, program, entry, json.dumps(data, sort_keys=True)))


def structured(con, name, document):
    """Structured identities are fingerprint qualified; prose never grants credit."""
    if not isinstance(document, dict):
        return False
    if name == "analysis/function-status.json":
        if document.get("programs") != HASHES:
            raise ValueError("Ledger fingerprint scope mismatch; no claims indexed")
        for row in document["functions"]:
            if document.get("programs", {}).get(row["program"]) == HASHES.get(row["program"]):
                put_record(con, name, "ledger", row["program"], row["entry"], row)
        return True
    program = PROGRAMS.get(document.get("sha256"), "unknown")
    if name.startswith("analysis/work/function-progress/") and "functions" in document:
        if program == "unknown":
            raise ValueError("Inventory fingerprint mismatch; no membership indexed")
        for row in document["functions"]:
            if not re.fullmatch(r"[0-9a-f]{8}", row.get("entry", "")):
                continue
            put_record(con, name, "inventory", program, row["entry"], row)
            for first, last in row.get("body_ranges", []):
                con.execute("INSERT INTO ranges VALUES (?,?,?,?,?)",
                            (name, program, row["entry"], int(first, 16), int(last, 16)))
        return True
    entry = document.get("entry", "")
    if (isinstance(entry, str) and re.fullmatch(r"[0-9a-fA-F]{8}", entry)
            and "decompiled" in document and "sha256" in document):
        entry = entry.lower()
        fields = ("program", "sha256", "name", "signature", "calling_convention",
                  "stack_purge_bytes", "body_bytes", "decompiled", "decompiler_message")
        data = {k: document[k] for k in fields if k in document}
        put_record(con, name, "export", program, entry, data)
        for direction in ("callers", "callees"):
            for row in document.get(direction, []):
                target = row.get("entry", "")
                if re.fullmatch(r"[0-9a-fA-F]{8}", target):
                    con.execute("INSERT INTO links VALUES (?,?,?,?,?,?)",
                                (name, program, entry, direction, target.lower(), row.get("name", "")))
    return False


def index_text(con, name, text, category):
    file_id = con.execute("SELECT id FROM files WHERE path=?", (name,)).fetchone()[0]
    lines = text.splitlines()
    heading = ""
    section = ""
    start = 1
    block = []
    mentions = []
    passages = []
    for number, line in enumerate(lines, 1):
        if line.startswith("#") and name.endswith(".md"):
            heading = line.lstrip("# ").strip()
            if block:
                passages.append((name, category, start, number - 1, section, "\n".join(block)))
                block = []
            start = number
        if not block:
            section = heading
        block.append(line)
        # Strip fingerprints before lexical matching; assembly rows also use 7-digit VAs.
        clean = LONG_HEX.sub("", line)
        entries = {m[1].lower() for m in ADDRESS.finditer(clean)}
        entries.update(m[1].lower() for m in COMPACT.finditer(clean))
        instruction = ASM_ADDRESS.match(line) if category == "assembly" else None
        if instruction:
            entries.add(instruction[1].lower().zfill(8))
        for entry in entries:
            mentions.append((file_id, entry, number, heading, line.strip()[:360]))
        if len(block) >= 70:
            passages.append((name, category, start, number, section, "\n".join(block)))
            block = []
            start = number + 1
    if block:
        passages.append((name, category, start, len(lines), section, "\n".join(block)))
    con.executemany("INSERT INTO mentions VALUES (?,?,?,?,?)", mentions)
    for passage in passages:
        rowid = con.execute("INSERT INTO passage_files(path) VALUES (?)", (name,)).lastrowid
        con.execute("INSERT INTO passages(rowid,path,kind,line,end_line,section,body) VALUES (?,?,?,?,?,?,?)", (rowid, *passage))


def discard(con, name):
    con.execute("DELETE FROM passages WHERE rowid IN (SELECT id FROM passage_files WHERE path=?)", (name,))
    con.execute("DELETE FROM files WHERE path=?", (name,))


def refresh(con, root, force=False):
    started = time.monotonic()
    con.execute("BEGIN IMMEDIATE")
    old = {row["path"]: row for row in con.execute("SELECT * FROM files")}
    seen = set()
    counts = Counter()
    for name, path in corpus(root):
        seen.add(name)
        try:
            stat = path.stat()
            stamp = (stat.st_mtime_ns, stat.st_ctime_ns, stat.st_size)
            previous = old.get(name)
            if not force and previous and stamp == tuple(previous[k] for k in ("mtime_ns", "ctime_ns", "size")):
                counts["unchanged"] += 1
                continue
            raw = path.read_bytes() if stat.st_size <= MAX_BYTES else b""
            after = path.stat()
            if stamp != (after.st_mtime_ns, after.st_ctime_ns, after.st_size):
                raise OSError("File changed during indexing; refresh again")
            digest = sha(raw) if stat.st_size <= MAX_BYTES else ""
            if previous and digest and previous["digest"] == digest and not force:
                con.execute("UPDATE files SET mtime_ns=?,ctime_ns=?,size=? WHERE path=?", (*stamp, name))
                counts["unchanged_content"] += 1
                continue
            if previous:
                discard(con, name)
            category = kind(name)
            error = "over 8 MiB limit; not indexed" if stat.st_size > MAX_BYTES else ""
            con.execute("INSERT INTO files(path,kind,mtime_ns,ctime_ns,size,digest,error) VALUES (?,?,?,?,?,?,?)",
                        (name, category, *stamp, digest, error))
            if error:
                counts["skipped"] += 1
                continue
            text = raw.decode("utf-8")
            if "\0" in text:
                raise ValueError("NUL in text; not indexed")
            special = False
            if name.endswith(".json"):
                special = structured(con, name, json.loads(text))
            if category == "decompilation":
                lines = text.splitlines()
                signature_line = next(((n, line.strip()) for n, line in enumerate(lines, 1)
                                       if "(" in line and not line.lstrip().startswith(("//", "/*", "*"))), (1, ""))
                put_record(con, name, "c_export", "unknown", Path(name).stem.lower(),
                           {"signature_excerpt": signature_line[1][:360], "line": signature_line[0]})
            if not special:
                index_text(con, name, text, category)
            counts["updated"] += 1
        except (OSError, UnicodeError, ValueError, KeyError, TypeError) as error:
            # Remove earlier results too: a transient/invalid file must never retain stale hits.
            discard(con, name)
            con.execute("INSERT INTO files(path,kind,mtime_ns,ctime_ns,size,digest,error) VALUES (?,?,?,?,?,?,?)",
                        (name, kind(name), 0, 0, 0, "", str(error)[:300]))
            counts["errors"] += 1
    for name in old.keys() - seen:
        discard(con, name)
        counts["removed"] += 1
    con.execute("INSERT OR REPLACE INTO meta VALUES ('refreshed',?)", (str(time.time()),))
    con.commit()
    return {**counts, "seconds": round(time.monotonic() - started, 3)}


def record_rows(con, category, program, entry):
    return [dict(row) | {"data": json.loads(row["data"])} for row in con.execute(
        "SELECT * FROM records WHERE category=? AND program=? AND entry=? ORDER BY path",
        (category, program, entry))]


def location(con, row):
    row = dict(row)
    file = con.execute("SELECT digest,mtime_ns FROM files WHERE path=?", (row["path"],)).fetchone()
    return row | {"file_sha256": file["digest"], "mtime_ns": file["mtime_ns"]}


def exports(con, program, entry, limit):
    rows = record_rows(con, "export", program, entry)
    groups = defaultdict(list)
    for row in rows:
        candidate = location(con, row)
        c_path = str(Path(row["path"]).with_suffix(".c"))
        c_file = con.execute("SELECT digest,error FROM files WHERE path=?", (c_path,)).fetchone()
        candidate["c_path"] = c_path if c_file and not c_file["error"] and row["data"].get("decompiled") is True else None
        candidate["c_sha256"] = c_file["digest"] if candidate["c_path"] else None
        # Preserve every differing metadata record, including changed references.
        identity = (candidate["file_sha256"], candidate["c_sha256"])
        groups[identity].append(candidate)
    result = []
    for copies in groups.values():
        copies.sort(key=lambda r: (-r["mtime_ns"], r["path"]))
        best = copies[0]
        result.append(best | {"copy_count": len(copies), "copies": [r["path"] for r in copies]})
    result.sort(key=lambda r: (-bool(r["c_path"] and r["data"].get("decompiled")), -r["mtime_ns"], r["path"]))
    return {"record_count": len(rows), "variant_count": len(result), "items": result[:limit]}


def c_exports(con, program, entry, limit):
    """Custom rollback-only ABI exporters often write C without any JSON sidecar."""
    groups = defaultdict(list)
    for row in con.execute("SELECT * FROM records WHERE category='c_export' AND program='unknown' AND entry=? ORDER BY path", (entry,)):
        item = location(con, row)
        item["data"] = json.loads(item["data"])
        sidecar = str(Path(item["path"]).with_suffix(".json"))
        metadata = con.execute("SELECT program,data FROM records WHERE category='export' AND path=? AND entry=?",
                               (sidecar, entry)).fetchone()
        scope = metadata["program"] if metadata else "unknown"
        if scope not in (program, "unknown"):
            continue
        item["program_scope"] = scope
        item["reported_sha256"] = json.loads(metadata["data"]).get("sha256") if metadata else None
        item["sidecar_decompiled"] = json.loads(metadata["data"]).get("decompiled") if metadata else None
        groups[(item["reported_sha256"], item["file_sha256"])].append(item)
    items = []
    for copies in groups.values():
        copies.sort(key=lambda r: (-r["mtime_ns"], r["path"]))
        items.append(copies[0] | {"copy_count": len(copies), "copies": [r["path"] for r in copies]})
    items.sort(key=lambda r: (-r["mtime_ns"], r["path"]))
    return {"variant_count": len(items), "items": items[:limit]}


def find(con, entry, program, limit):
    inventories = record_rows(con, "inventory", program, entry)
    ledger = record_rows(con, "ledger", program, entry)
    containing = [dict(row) for row in con.execute(
        "SELECT DISTINCT path,entry FROM ranges WHERE program=? AND first<=? AND last>=? AND entry!=? ORDER BY entry",
        (program, int(entry, 16), int(entry, 16), entry))]
    groups = {}
    for category in ("source", "note", "receipt", "assembly", "decompilation", "metadata", "helper"):
        # A few locations per category; latest sections first in append-only research notes.
        rows = con.execute("""SELECT f.path,m.entry,m.line,m.section,m.excerpt
            FROM mentions m JOIN files f ON f.id=m.file_id
            WHERE m.entry=? AND f.kind=? ORDER BY
            CASE WHEN REPLACE(m.excerpt,' ','') LIKE ? THEN 0 ELSE 1 END,
            CASE WHEN f.kind='note' THEN -f.mtime_ns ELSE 0 END,path,
            CASE WHEN f.kind='note' THEN -line ELSE line END""", (entry, category, "%//" + entry + "%"))
        items = []
        paths = set()
        total = 0
        for row in rows:
            if row["path"] in paths:
                continue
            paths.add(row["path"])
            total += 1
            if len(items) < limit:
                items.append(dict(row))
        if total:
            groups[category] = {"file_count": total, "items": items}
    return {"entry": entry, "program": program, "inventory": inventories,
            "ledger": ledger, "containing_functions": containing,
            "exports": exports(con, program, entry, limit),
            "c_exports": c_exports(con, program, entry, limit), "mentions_unqualified": groups}


def neighbors(con, entry, program, limit):
    result = {}
    for direction in ("callers", "callees"):
        rows = con.execute("SELECT * FROM links WHERE program=? AND entry=? AND direction=? ORDER BY target,path",
                           (program, entry, direction))
        groups = defaultdict(list)
        for row in rows:
            groups[row["target"]].append(dict(row))
        items = []
        for target, evidence in sorted(groups.items()):
            ledger = record_rows(con, "ledger", program, target)
            items.append({"entry": target, "name": evidence[0]["name"],
                          "ledger_status": ledger[0]["data"]["status"] if ledger else "no_ledger_claim",
                          "export_records": con.execute(
                              "SELECT COUNT(*) FROM records WHERE category='export' AND program=? AND entry=?",
                              (program, target)).fetchone()[0],
                          "c_variants": c_exports(con, program, target, 1)["variant_count"],
                          "reported_by": [r["path"] for r in evidence]})
        result[direction] = {"total": len(items), "items": items[:limit]}
    return {"entry": entry, "program": program, **result}


def search(con, query, category, limit, path_filter=None, phrase=False):
    words = query.split()
    if not words:
        raise ValueError("Search needs at least one word")
    terms = [query] if phrase else words
    match = " AND ".join('"' + word.replace('"', '""') + '"' for word in terms)
    where = ""
    params = [match]
    if category:
        where += " AND kind=?"
        params.append(category)
    if path_filter:
        where += " AND instr(path,?)>0"
        params.append(path_filter)
    params.append(limit)
    rows = con.execute("""SELECT path,kind,line,end_line,section,
        snippet(passages,5,'[',']',' … ',30) AS excerpt FROM passages
        WHERE passages MATCH ?""" + where + " ORDER BY bm25(passages),path,CAST(line AS INTEGER) LIMIT ?", params)
    return {"query": query, "items": [dict(row) for row in rows]}


def read(root, name, first, count, con):
    path = safe_file(root, name)
    cached = con.execute("SELECT * FROM files WHERE path=?", (name,)).fetchone()
    if not cached or cached["error"]:
        raise ValueError("Only successfully indexed text can be read")
    if path.stat().st_size > MAX_BYTES:
        raise ValueError("File exceeds text limit")
    raw = path.read_bytes()
    lines = raw.decode("utf-8").splitlines()
    return {"path": name, "file_sha256": sha(raw), "changed_since_index": sha(raw) != cached["digest"],
            "line": first, "total_lines": len(lines),
            "lines": [{"line": n, "text": text} for n, text in enumerate(lines, 1) if first <= n < first + count]}


def status(con, db):
    return {"database": str(db), "files": dict(con.execute("SELECT kind,COUNT(*) FROM files GROUP BY kind")),
            "issues": [dict(row) for row in con.execute("SELECT path,error FROM files WHERE error!='' ORDER BY path")],
            "records": dict(con.execute("SELECT category,COUNT(*) FROM records GROUP BY category")),
            "mentions": con.execute("SELECT COUNT(*) FROM mentions").fetchone()[0],
            "passages": con.execute("SELECT COUNT(*) FROM passages").fetchone()[0]}


def format_text(command, result):
    if command == "find":
        for hit in result["results"]:
            print(f"\n{hit['program']}:{hit['entry']}")
            for record in hit["inventory"]:
                row = record["data"]
                print(f"  Inventory: {row['name']}; {row.get('body_bytes')} bytes; thunk={row.get('thunk')}")
            if not hit["inventory"]:
                print("  Inventory: no exact indexed entry (not proof of absence)")
            for row in hit["containing_functions"]:
                print(f"  Interior of {row['entry']} in {row['path']}")
            for record in hit["ledger"]:
                row = record["data"]
                print(f"  Ledger: {row['status']} — {row['coverage']}")
                print(f"  Limitations: {row['limitations']}")
                for path in row["evidence"]:
                    print(f"    evidence: {path}")
            if not hit["ledger"]:
                print("  Ledger: no claim; uncommitted bodies may still exist")
            cs = hit["c_exports"]
            print(f"  C candidates: {cs['variant_count']} content/scope variants (newest modified first, not correctness order)")
            for item in cs["items"]:
                print(f"    {item['path']}:{item['data']['line']} ({item['copy_count']} copies; program={item['program_scope']})")
                print(f"      {item['data']['signature_excerpt']}")
                if item["sidecar_decompiled"] is False:
                    print("      Sidecar reports decompilation FAILURE; this C may be left over")
            ex = hit["exports"]
            print(f"  Exports: {ex['record_count']} records / {ex['variant_count']} variants (showing {len(ex['items'])})")
            for item in ex["items"]:
                row = item["data"]
                print(f"    {item['path']} ({item['copy_count']} copies)")
                print(f"      {row.get('signature')}; convention={row.get('calling_convention')}; purge={row.get('stack_purge_bytes')}")
                print(f"      C: {item['c_path'] or 'unavailable'}; decompiled={row.get('decompiled')}")
                if row.get("decompiler_message"):
                    print(f"      message: {row['decompiler_message']}")
            for category, group in hit["mentions_unqualified"].items():
                print(f"  {category} mentions ({group['file_count']} files, program unqualified):")
                for item in group["items"]:
                    print(f"    {item['path']}:{item['line']} [{item['section']}] {item['excerpt']}")
    elif command == "search":
        for item in result["items"]:
            print(f"{item['path']}:{item['line']}..{item['end_line']} ({item['kind']}) [{item['section']}]")
            print("  " + " ".join(item["excerpt"].split()))
    elif command == "neighbors":
        for direction in ("callers", "callees"):
            group = result[direction]
            print(f"{direction}: {group['total']} export-reported targets (showing {len(group['items'])})")
            for item in group["items"]:
                print(f"  {item['entry']} {item['name']}; {item['ledger_status']}; {item['export_records']} metadata exports; {item['c_variants']} C variants")
                print(f"    {item['reported_by'][0]}")
    elif command == "read":
        print(f"{result['path']} — {result['total_lines']} lines; changed_since_index={result['changed_since_index']}")
        for line in result["lines"]:
            print(f"{line['line']:5}: {line['text']}")
    else:
        print(json.dumps(result, indent=2, sort_keys=True))


def positive(value):
    value = int(value)
    if not 1 <= value <= 200:
        raise argparse.ArgumentTypeError("Expected 1..200")
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=TOOL_ROOT, help="Checkout to read, including ignored exports")
    parser.add_argument("--db", type=Path, help="Cache path; default is under this TOOL checkout, never --root")
    parser.add_argument("--json", action="store_true", help="Machine-readable bounded results")
    parser.add_argument("--no-refresh", action="store_true", help="Query previous snapshot; may be stale")
    sub = parser.add_subparsers(dest="command", required=True)
    build = sub.add_parser("index", help="Incrementally refresh the index")
    build.add_argument("--force", action="store_true", help="Rehash/reparse every candidate")
    sub.add_parser("status", help="Cache coverage and skipped/invalid inputs")
    lookup = sub.add_parser("find", help="Batch addresses: ledger, inventory, exports, source, contracts, receipts")
    lookup.add_argument("addresses", nargs="+", type=address)
    lookup.add_argument("--program", choices=(*HASHES, "unknown"), default="client")
    lookup.add_argument("--limit", type=positive, default=4, help="Locations per category, per address")
    links = sub.add_parser("neighbors", help="Existing export-reported callers/callees; no inferred call edges")
    links.add_argument("address", type=address)
    links.add_argument("--program", choices=tuple(HASHES), default="client")
    links.add_argument("--limit", type=positive, default=30)
    query = sub.add_parser("search", help="AND text terms within 70-line passages")
    query.add_argument("query")
    query.add_argument("--kind", choices=("source", "note", "receipt", "assembly", "decompilation", "metadata", "helper"))
    query.add_argument("--limit", type=positive, default=12)
    query.add_argument("--path", help="Literal path substring to restrict results")
    query.add_argument("--phrase", action="store_true", help="Match the complete query as one phrase")
    show = sub.add_parser("read", help="Read bounded live text at an indexed location")
    show.add_argument("path")
    show.add_argument("--line", type=int, default=1)
    show.add_argument("--count", type=positive, default=50)
    args = parser.parse_args()
    con = None
    try:
        root = args.root.resolve(strict=True)
        if not (root / "analysis").is_dir():
            raise ValueError("--root must be a reconstruction checkout")
        db = args.db or TOOL_ROOT / "analysis/work/research-index" / (sha(str(root).encode())[:16] + "-v" + SCHEMA + ".sqlite3")
        con = connect(db, root, args.no_refresh and args.command != "index")
        update = None
        if not args.no_refresh or args.command == "index":
            update = refresh(con, root, getattr(args, "force", False))
        if args.command in ("index", "status"):
            result = status(con, db)
        elif args.command == "find":
            result = {"results": [find(con, entry, args.program, args.limit) for entry in args.addresses]}
        elif args.command == "neighbors":
            result = neighbors(con, args.address, args.program, args.limit)
        elif args.command == "search":
            result = search(con, args.query, args.kind, args.limit, args.path, args.phrase)
        else:
            if args.line < 1:
                raise ValueError("--line must be positive")
            result = read(root, args.path, args.line, args.count, con)
        result.update(root=str(root), policy=POLICY, refresh=update,
                      omitted_file_count=con.execute("SELECT COUNT(*) FROM files WHERE error!=''").fetchone()[0],
                      snapshot_refreshed_unix=dict(con.execute("SELECT key,value FROM meta")).get("refreshed"),
                      snapshot_may_be_stale=args.no_refresh and args.command != "index")
        if args.json:
            print(json.dumps(result, sort_keys=True, indent=2))
        else:
            print(f"Root: {root}")
            if update:
                print(f"Refresh: {update}")
            else:
                print("Snapshot only (--no-refresh): live source may have changed")
            if result["omitted_file_count"]:
                print(f"Coverage: {result['omitted_file_count']} skipped/invalid files; use status for paths")
            format_text(args.command, result)
            print("\n" + POLICY)
        return 0
    except (ValueError, OSError, sqlite3.Error) as error:
        print(f"research-index: {error}", file=sys.stderr)
        return 1
    finally:
        if con is not None:
            con.close()


if __name__ == "__main__":
    sys.exit(main())
