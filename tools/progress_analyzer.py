#!/usr/bin/env python3
"""Read repository progress metadata and emit deterministic JSON; never write files.

See tools/progress-analyzer.md for the evidence model and its limits.
"""

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

# Imported tools must not create __pycache__ during ordinary CLI use.
sys.dont_write_bytecode = True
from function_progress import ADDRESS, COUNTS, HASHES, intervals, require
from progress_graph import graph_export

ROOT = Path(__file__).resolve().parents[1]
CATALOG = "tools/progress-dependencies.json"
STAGES = ("source_recovery", "native_verification", "i386_compilation",
          "integration", "desktop_acceptance")
STATUSES = {"unknown", "unresolved", "supported_subset", "recorded_pass"}
VERIFICATION_STAGES = {
    "native": "native_verification", "native_sanitizers": "native_verification",
    "compiler_native": "native_verification", "context_native": "native_verification",
    "activation": "native_verification", "players": "native_verification",
    "new_weapon_transfer": "native_verification", "prior_equipment": "native_verification",
    "menu_text": "native_verification", "python": "native_verification",
    "i386": "i386_compilation", "actual_host_i386": "i386_compilation",
    "application": "i386_compilation", "application_compile_link": "i386_compilation",
    "application_and_startup": "i386_compilation", "new_headers": "i386_compilation",
    "desktop": "desktop_acceptance",
}


def reject_constant(value):
    raise ValueError(f"Non-finite JSON constant: {value}")


class Sources:
    """Cache exactly the bytes used, with portable paths and reproducible digests."""

    def __init__(self, root):
        self.root = root.resolve()
        self.content = {}
        self.diagnostics = []

    def path(self, name):
        require(isinstance(name, str) and name, "Expected nonempty repository path")
        path = Path(name)
        require(not path.is_absolute()
                and ".." not in path.parts and path.as_posix() == name,
                f"Unsafe repository path: {name}")
        require(path.suffix in (".json", ".md")
                and path.parts[0] in ("analysis", "tools", "plan.md")
                and not ({"work", "ghidra"} & set(path.parts)),
                f"Not an allowed progress input: {name}")
        resolved = (self.root / path).resolve()
        require(resolved.is_relative_to(self.root), f"Escaping symlink: {name}")
        require(resolved == self.root / path, f"Symlinked progress input: {name}")
        return resolved

    def read(self, name):
        if name not in self.content:
            self.content[name] = self.path(name).read_bytes()
        return self.content[name].decode("utf-8")

    def json(self, name):
        # Use the census tool's duplicate-key rejection, over cached bytes.
        from function_progress import unique_object
        return json.loads(self.read(name), object_pairs_hook=unique_object,
                          parse_constant=reject_constant)

    def issue(self, code, **details):
        self.diagnostics.append({"code": code, **details})

    def anchor(self, anchor):
        require(set(anchor) == {"path", "quote"} and isinstance(anchor["quote"], str)
                and anchor["quote"].strip(), "Evidence needs path and nonempty quote")
        name, quote = anchor["path"], anchor["quote"]
        try:
            body = self.read(name)
        except FileNotFoundError:
            self.issue("missing_evidence", path=name, quote=quote)
            return {**anchor, "state": "unknown"}
        if body.count(quote) != 1:
            self.issue("changed_or_ambiguous_evidence", path=name, quote=quote)
            return {**anchor, "state": "unknown"}
        start = body.index(quote)
        return {**anchor, "state": "verified", "line": body[:start].count("\n") + 1}

    def manifest(self):
        return [{"path": name, "sha256": hashlib.sha256(data).hexdigest()}
                for name, data in sorted(self.content.items())]


def reference(path, pointer):
    return {"path": path, "json_pointer": pointer}


def census(sources):
    """Validate public metadata, without pretending it is the frozen inventory."""
    baseline_path, ledger_path = "analysis/function-baseline.json", "analysis/function-status.json"
    baseline, ledger = sources.json(baseline_path), sources.json(ledger_path)
    require(isinstance(baseline, dict) and type(baseline.get("schema_version")) is int
            and baseline["schema_version"] == 1
            and baseline.get("scope") == "base_hd2_and_original_ls3df_only"
            and baseline.get("estimated") is True
            and baseline.get("denominator") ==
            "identified_internal_non_thunk_including_unclassified_and_libraries"
            and set(baseline.get("programs", {})) == set(HASHES), "Unsupported baseline")
    require(isinstance(ledger, dict) and set(ledger) == {"schema_version", "programs", "functions"}
            and type(ledger["schema_version"]) is int and ledger["schema_version"] == 1
            and ledger["programs"] == HASHES and isinstance(ledger["functions"], list),
            "Unsupported ledger schema or program fingerprints")
    rows, seen = [], set()
    by_program = {}
    for program, fingerprint in HASHES.items():
        record = baseline["programs"][program]
        counts = record["counts"]
        require(record["sha256"] == fingerprint and set(counts) == set(COUNTS)
                and all(type(n) is int and n >= 0 for n in counts.values())
                and counts["internal_non_thunk"] > 0
                and counts["identified_total"] == sum(counts[k] for k in
                    ("internal_non_thunk", "internal_thunk", "external"))
                and counts["unclassified_internal_non_thunk"] <= counts["internal_non_thunk"],
                f"Invalid frozen counts: {program}")
        by_program[program] = {"frozen_internal_non_thunk": counts["internal_non_thunk"],
                               "complete": 0, "partial": 0}
    for index, row in enumerate(ledger["functions"]):
        required = {"program", "entry", "status", "coverage", "limitations", "evidence"}
        require(isinstance(row, dict) and required <= set(row)
                and set(row) <= required | {"intervals", "end_to_end", "verification"},
                "Invalid ledger row keys")
        program, entry = row["program"], row["entry"]
        require(isinstance(program, str) and program in HASHES
                and isinstance(entry, str) and ADDRESS.fullmatch(entry), "Invalid ledger identity")
        key = program, entry
        require(key not in seen, f"Duplicate ledger identity: {key}")
        seen.add(key)
        require(row["status"] in ("partial", "complete")
                and isinstance(row["coverage"], str) and row["coverage"].strip(),
                f"Invalid reconstruction claim: {key}")
        if row["status"] == "partial":
            require(isinstance(row["limitations"], str) and row["limitations"].strip()
                    and row.get("end_to_end", False) is False, f"Invalid partial claim: {key}")
        else:
            require(row["limitations"] == "" and row.get("end_to_end") is True
                    and isinstance(row.get("verification"), str) and row["verification"].strip(),
                    f"Invalid complete claim: {key}")
        links = row["evidence"]
        require(isinstance(links, list) and links and all(isinstance(p, str) and p for p in links)
                and len(links) == len(set(links)), f"Invalid evidence paths: {key}")
        if "intervals" in row:
            require(intervals(row["intervals"], entry), f"Empty intervals: {key}")
        rows.append({**row, "record": reference(ledger_path, f"/functions/{index}"),
                     "inventory_membership": "unknown"})
        by_program[program][row["status"]] += 1
    for program, counts in by_program.items():
        claimed = counts["complete"] + counts["partial"]
        require(claimed <= counts["frozen_internal_non_thunk"], f"Claims exceed baseline: {program}")
        counts["without_ledger_claim"] = counts["frozen_internal_non_thunk"] - claimed
    return {"baseline": baseline, "by_program": by_program,
            "totals": {key: sum(row[key] for row in by_program.values())
                       for key in next(iter(by_program.values()))},
            "entries": sorted(rows, key=lambda row: (row["program"], row["entry"])),
            "validation_scope": "Metadata consistency only; frozen inventory membership, body bounds, "
                                "binary fingerprints and behavioral equivalence are not reverified."}


def checklist(sources):
    lines = sources.read("plan.md").splitlines()
    headings, items = {}, []
    index, in_fence = 0, False
    while index < len(lines):
        line = lines[index]
        if line.lstrip().startswith(("```", "~~~")):
            in_fence = not in_fence
        heading = re.match(r"^(#{1,6}) (.+)$", line) if not in_fence else None
        if heading:
            level = len(heading[1])
            headings = {k: v for k, v in headings.items() if k < level}
            headings[level] = heading[2]
        match = re.match(r"^- \[([ xX])\] (.+)$", line) if not in_fence else None
        if match:
            start, text = index + 1, [match[2]]
            while index + 1 < len(lines) and lines[index + 1].startswith("  "):
                index += 1
                text.append(lines[index].strip())
            body = "\n".join(text)
            items.append({"id": f"plan.md:L{start}", "path": "plan.md", "line": start,
                          "headings": list(headings.values()), "checked": match[1].lower() == "x",
                          "text": body, "explicit_source_only": bool(re.search(
                              r"source[ -]only", body, re.IGNORECASE))})
        index += 1
    require(items, "No plan checklist items found")
    return {"items": items, "counts": {"checked": sum(item["checked"] for item in items),
                                       "unchecked": sum(not item["checked"] for item in items)},
            "semantics": "Literal checklist states, including historical and postponed items; "
                         "checked is not an independently verified delivery claim."}


def milestones(sources, functions):
    path = "analysis/phase2-delivery-plan.json"
    document = sources.json(path)
    require(isinstance(document, dict) and type(document.get("schema_version")) is int
            and document["schema_version"] == 1
            and isinstance(document.get("milestones"), list), "Unsupported Phase 2 index")
    rows = document["milestones"]
    ids = [row["id"] for row in rows]
    require(all(isinstance(key, str) for key in ids) and len(ids) == len(set(ids)),
            "Duplicate/invalid milestone IDs")
    lookup = {(row["program"], row["entry"]): row["status"] for row in functions["entries"]}
    references = {}
    for index, row in enumerate(document.get("original_reference_index", [])):
        entry, program = row["entry"], row["program"]
        require(program in HASHES and isinstance(entry, str) and ADDRESS.fullmatch(entry)
                and entry not in references, "Ambiguous/invalid phase reference identity")
        references[entry] = {"program": program, "entry": entry,
                             "reported_inventory_membership": row["inventory_membership"],
                             "record": reference(path, f"/original_reference_index/{index}")}
    output, edges = [], []
    for index, row in enumerate(rows):
        require(isinstance(row["prerequisites"], list)
                and len(row["prerequisites"]) == len(set(row["prerequisites"]))
                and all(key in ids and key != row["id"] for key in row["prerequisites"]),
                f"Invalid prerequisites: {row['id']}")
        seeds = []
        for entry in row["original_function_seeds"]:
            require(isinstance(entry, str) and entry in references, "Unqualified function seed")
            original = references[entry]
            seeds.append({"program": original["program"], "entry": entry,
                          "reference_record": original["record"],
                          "ledger_status": lookup.get((original["program"], entry), "no_claim")})
        require(all(entry in references for entry in row["reference_addresses"]),
                "Unqualified phase reference address")
        output.append({"id": row["id"], "title": row["title"], "reported_status": row["status"],
                       "acceptance": row["acceptance"], "function_seeds": seeds,
                       "reference_addresses": row["reference_addresses"],
                       "references": [references[entry] for entry in row["reference_addresses"]],
                       "record": reference(path, f"/milestones/{index}")})
        edges.extend({"dependent": row["id"], "prerequisite": key, "basis": "explicit",
                      "evidence": reference(path, f"/milestones/{index}/prerequisites/{i}")}
                     for i, key in enumerate(row["prerequisites"]))
    topological_order(ids, edges)
    declared_hash = document.get("ledger_sha256")
    actual_hash = hashlib.sha256(sources.content["analysis/function-status.json"]).hexdigest()
    if declared_hash and declared_hash != actual_hash:
        sources.issue("phase_index_ledger_hash_stale", declared=declared_hash, actual=actual_hash)
    for index, row in enumerate(document.get("original_reference_index", [])):
        current = lookup.get((row["program"], row["entry"]), "uncredited")
        if row.get("ledger_status") != current:
            sources.issue("phase_reference_status_stale", entry=row["entry"],
                          declared=row.get("ledger_status"), current=current,
                          evidence=reference(path, f"/original_reference_index/{index}"))
    return {"active_milestone": document["active_milestone"], "items": output, "dependencies": edges}


def topological_order(ids, edges):
    remaining = {key: set() for key in ids}
    for edge in edges:
        remaining[edge["dependent"]].add(edge["prerequisite"])
    ordered = []
    while remaining:
        ready = sorted(key for key, needs in remaining.items() if not needs)
        require(ready, "Dependency cycle detected")
        ordered.extend(ready)
        for key in ready:
            del remaining[key]
        for needs in remaining.values():
            needs.difference_update(ready)
    return ordered


def checkpoints(sources):
    result = []
    directory = sources.path("analysis/checkpoints/index.json").parent
    for path in sorted(directory.glob("*.json")):
        name = path.relative_to(sources.root).as_posix()
        row = sources.json(name)
        require(isinstance(row, dict), f"Invalid checkpoint: {name}")
        require(isinstance(row.get("original_entries", []), list)
                and all(isinstance(entry, str) and ADDRESS.fullmatch(entry)
                        for entry in row.get("original_entries", [])),
                f"Invalid checkpoint addresses: {name}")
        verification = row.get("verification", {})
        require(isinstance(verification, dict)
                or (isinstance(verification, list)
                    and all(isinstance(item, str) and item.strip() for item in verification)),
                f"Invalid checkpoint verification: {name}")
        observations = []
        # Prose lists retain their report scope without guessing verification stages.
        items = verification.items() if isinstance(verification, dict) else []
        for key, value in sorted(items):
            if key in VERIFICATION_STAGES:
                observations.append({"stage": VERIFICATION_STAGES[key], "reported": value,
                                     "record": reference(name, f"/verification/{key}"),
                                     "interpretation": "Scoped repository report; not rerun or "
                                                       "promoted to a whole loading gate."})
        result.append({"path": name, "date": row.get("date"), "scope": row.get("scope"),
                       "source_only": row.get("source_only"),
                       "desktop_verified": row.get("desktop_verified"),
                       "original_entries": row.get("original_entries", []),
                       "reported_verification": verification, "observations": observations,
                       "record": reference(name, "")})
    return result


def dependency_graph(sources):
    catalog = sources.json(CATALOG)
    require(isinstance(catalog, dict) and type(catalog.get("schema_version")) is int
            and catalog["schema_version"] == 1,
            "Unsupported dependency catalog")
    nodes, edges = [], []
    require(catalog.get("function_program") in HASHES, "Invalid catalog function program")
    for index, row in enumerate(catalog["nodes"]):
        require(row["stage"] in STAGES and row["status"] in STATUSES
                and isinstance(row["id"], str) and row["evidence"], "Invalid dependency node")
        evidence = [sources.anchor(anchor) for anchor in row["evidence"]]
        valid = all(anchor["state"] == "verified" for anchor in evidence)
        nodes.append({**row, "status": row["status"] if valid else "unknown",
                      "evidence_state": "verified" if valid else "unknown", "evidence": evidence,
                      "basis": "explicit", "catalog_claim": row["status"],
                      "catalog_record": reference(CATALOG, f"/nodes/{index}")})
    ids = [node["id"] for node in nodes]
    require(len(ids) == len(set(ids)), "Duplicate dependency node")
    for row in catalog["dependencies"]:
        require(row["dependent"] in ids and row["prerequisite"] in ids
                and row["dependent"] != row["prerequisite"] and row["evidence"]
                and row["basis"] in ("explicit", "inferred"), "Invalid dependency edge")
        if row["basis"] == "inferred":
            require(isinstance(row.get("rationale"), str) and row["rationale"].strip(),
                    "Inferred relationship requires a rationale")
        evidence = [sources.anchor(anchor) for anchor in row["evidence"]]
        edges.append({**row, "evidence": evidence, "evidence_state":
                      "verified" if all(a["state"] == "verified" for a in evidence) else "unknown"})
    pairs = [(row["dependent"], row["prerequisite"]) for row in edges]
    require(len(pairs) == len(set(pairs)), "Duplicate dependency edge")
    topological_order(ids, edges)
    explicit = [row for row in edges if row["basis"] == "explicit"
                and row["evidence_state"] == "verified"]
    closures = {}
    for target in (catalog["render_target"], catalog["delivery_target"]):
        require(target in ids, "Unknown dependency target")
        closure = {target}
        while True:
            expanded = closure | {row["prerequisite"] for row in explicit if row["dependent"] in closure}
            if expanded == closure:
                break
            closure = expanded
        closures[target] = sorted(closure)
    return {"milestone": catalog["milestone"], "function_program": catalog["function_program"],
            "render_target": catalog["render_target"], "delivery_target": catalog["delivery_target"],
            "stages": list(STAGES), "nodes": sorted(nodes, key=lambda row: row["id"]),
            "dependencies": sorted(edges, key=lambda row: (row["dependent"], row["prerequisite"])),
            "explicit_dependency_closures": closures,
            "prerequisites_first": topological_order(ids, explicit),
            "unresolved_dependencies": [
                {"dependent": row["dependent"], "prerequisite": row["prerequisite"],
                 "status": next(n["status"] for n in nodes if n["id"] == row["prerequisite"])}
                for row in explicit
                if next(n["status"] for n in nodes if n["id"] == row["prerequisite"])
                in ("unresolved", "unknown", "supported_subset")],
            "coverage": "Bounded evidence-backed prerequisite graph, not an exhaustive call graph "
                        "or a duration-based critical path. Unknown and subset nodes are not satisfied."}


def analyze(root):
    sources = Sources(root)
    functions = census(sources)
    plan = checklist(sources)
    phases = milestones(sources, functions)
    receipts = checkpoints(sources)
    graph = dependency_graph(sources)
    website_graph = graph_export(functions, phases, receipts, graph, STAGES)
    return {"schema_version": 1,
            "epistemic_policy": {
                "verified": "Exact statements or structured values found in repository inputs; "
                            "not independent proof of original behavior or reproduced validation.",
                "inferred": "Reviewed relationships explicitly labeled inferred, with rationale; "
                            "excluded from explicit dependency closures.",
                "unknown": "No applicable evidence, missing input, or changed evidence anchor.",
                "function_dimensions": "Ledger status is source reconstruction credit. Native, i386, "
                                       "integration and desktop status cannot be inferred per function.",
                "receipt_scope": "Verification receipts retain their original fixture boundaries; "
                                 "they never promote a whole loading gate or function.",
                "omitted_metrics": "No completion percentages, weights, ETAs or inferred call edges."},
            "functions": functions, "checklist": plan, "milestones": phases,
            "checkpoints": receipts, "first_frame": graph, "graph": website_graph,
            "diagnostics": sorted(sources.diagnostics, key=lambda row: json.dumps(row, sort_keys=True)),
            "inputs": sources.manifest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT, help="repository root (default: script's repository)")
    args = parser.parse_args()
    try:
        result = analyze(args.root)
        encoded = json.dumps(result, indent=2, sort_keys=True, ensure_ascii=True, allow_nan=False)
    except (ValueError, KeyError, TypeError, OSError) as error:
        print(json.dumps({"error": str(error)}, sort_keys=True), file=sys.stderr)
        return 1
    print(encoded)
    return 0


if __name__ == "__main__":
    sys.exit(main())
