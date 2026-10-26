#!/usr/bin/env python3
"""Fingerprint-guarded, dependency-free function census and honest completion bar.

See analysis/function-progress.md for schemas, status/coverage rules and refresh
commands. Inventories are local metadata, never pseudocode. --freeze-baseline is
explicit: routine rendering/checking must never silently change the denominator.
"""

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
HASHES = {
    "client": "8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307",
    "engine": "12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee",
}
NAMES = {"client": "Base game client", "engine": "Original LS3DF.dll"}
START = "<!-- FUNCTION-PROGRESS:START -->"
END = "<!-- FUNCTION-PROGRESS:END -->"
ADDRESS = re.compile(r"[0-9a-f]{8}\Z")
COUNTS = ("internal_non_thunk", "internal_thunk", "external", "identified_total",
          "unclassified_internal_non_thunk")


def require(condition, message):
    if not condition:
        raise ValueError(message)


def integer(value):
    return type(value) is int and value >= 0


def text(value):
    return isinstance(value, str) and bool(value.strip())


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, f"Duplicate JSON key: {key}")
        result[key] = value
    return result


def load(path):
    return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=unique_object)


def digest(value):
    return hashlib.sha256(json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=True
    ).encode("ascii")).hexdigest()


def intervals(value, label):
    require(isinstance(value, list), f"{label}: expected interval list")
    result = []
    previous = -1
    for pair in value:
        require(isinstance(pair, list) and len(pair) == 2
                and all(isinstance(a, str) and ADDRESS.fullmatch(a) for a in pair),
                f"{label}: intervals need two lowercase eight-digit hex addresses")
        start, end = map(lambda a: int(a, 16), pair)
        require(start <= end and start > previous, f"{label}: reversed/overlapping/unsorted intervals")
        result.append((start, end))
        previous = end
    return result


def validate_inventory(document, program):
    require(isinstance(document, dict) and type(document.get("schema_version")) is int
            and document["schema_version"] == 1,
            f"{program}: unsupported inventory schema")
    require(document.get("program") == program and document.get("sha256") == HASHES[program],
            f"{program}: wrong program/fingerprint (expansion is excluded)")
    for field in ("program_name", "ghidra_version", "image_base"):
        require(text(document.get(field)), f"{program}: missing {field}")
    require(ADDRESS.fullmatch(document["image_base"]), f"{program}: invalid image base")
    require(isinstance(document.get("functions"), list) and document["functions"],
            f"{program}: empty/missing function inventory")
    functions = {}
    counts = Counter({key: 0 for key in COUNTS})
    structural = []
    for row in document["functions"]:
        require(isinstance(row, dict), f"{program}: invalid function row")
        entry = row.get("entry")
        require(text(entry) and entry not in functions, f"{program}: duplicate/invalid entry {entry}")
        require(type(row.get("external")) is bool and type(row.get("thunk")) is bool,
                f"{entry}: external/thunk must be Boolean")
        require(integer(row.get("body_bytes")), f"{entry}: invalid body size")
        for field in ("name", "namespace", "name_source"):
            require(text(row.get(field)), f"{entry}: missing category clue {field}")
        require(isinstance(row.get("tags"), list)
                and all(text(tag) for tag in row["tags"])
                and len(row["tags"]) == len(set(row["tags"])), f"{entry}: invalid tags")
        if row["external"]:
            category = "external"
            require(re.fullmatch(r"EXTERNAL:[0-9a-f]{8}", entry) and row.get("body_ranges") == [],
                    f"{entry}: malformed external address/body")
        else:
            require(ADDRESS.fullmatch(entry), f"{entry}: invalid internal entry")
            ranges = intervals(row.get("body_ranges"), entry)
            require(ranges and any(a <= int(entry, 16) <= b for a, b in ranges),
                    f"{entry}: body does not include entry")
            require(sum(b - a + 1 for a, b in ranges) == row["body_bytes"],
                    f"{entry}: body size disagrees with ranges")
            category = "internal_thunk" if row["thunk"] else "internal_non_thunk"
        classification = category if category != "internal_non_thunk" else "unclassified"
        require(row.get("classification") == classification, f"{entry}: inconsistent classification")
        counts[category] += 1
        if category == "internal_non_thunk":
            counts["unclassified_internal_non_thunk"] += 1
        functions[entry] = row
        structural.append({key: row[key] for key in
                           ("entry", "external", "thunk", "body_bytes", "body_ranges", "classification")})
    counts["identified_total"] = len(functions)
    require(counts["internal_non_thunk"] > 0, f"{program}: no denominator")
    return functions, {
        "sha256": HASHES[program],
        "ghidra_version": document["ghidra_version"],
        "image_base": document["image_base"],
        "counts": dict(counts),
        "structure_sha256": digest(sorted(structural, key=lambda row: row["entry"])),
    }


def make_baseline(inventories):
    require(set(inventories) == set(HASHES), "Exactly client and engine inventories required")
    functions, programs = {}, {}
    for program in HASHES:
        functions[program], programs[program] = validate_inventory(inventories[program], program)
    return functions, {
        "schema_version": 1,
        "scope": "base_hd2_and_original_ls3df_only",
        "denominator": "identified_internal_non_thunk_including_unclassified_and_libraries",
        "estimated": True,
        "programs": programs,
    }


def validate_baseline(baseline, actual):
    require(digest(baseline) == digest(actual),
            "Frozen baseline differs from inventory structure/counts. Review the changes; "
            "use --freeze-baseline only for an intentional new baseline, then --render.")


def validate_ledger(ledger, functions, root=ROOT):
    require(isinstance(ledger, dict)
            and set(ledger) == {"schema_version", "programs", "functions"}
            and type(ledger["schema_version"]) is int
            and ledger["schema_version"] == 1, "Unsupported ledger schema/keys")
    require(ledger["programs"] == HASHES, "Ledger fingerprints/program scope mismatch")
    require(isinstance(ledger["functions"], list), "Ledger functions must be a list")
    seen, counts = set(), {program: Counter() for program in HASHES}
    for row in ledger["functions"]:
        required = {"program", "entry", "status", "coverage", "limitations", "evidence"}
        require(isinstance(row, dict) and required <= set(row)
                and set(row) <= required | {"intervals", "end_to_end", "verification"},
                "Ledger function has missing/unknown keys")
        program, entry = row["program"], row["entry"]
        require(isinstance(program, str) and program in HASHES
                and isinstance(entry, str) and ADDRESS.fullmatch(entry),
                "Ledger has invalid program/entry")
        key = program, entry
        require(key not in seen, f"Duplicate ledger function: {program}:{entry}")
        seen.add(key)
        function = functions[program].get(entry)
        require(function is not None and not function["external"] and not function["thunk"],
                f"{program}:{entry}: status must match an inventoried internal non-thunk entry")
        require(row["status"] in ("partial", "complete"), f"{entry}: invalid status")
        require(text(row["coverage"]), f"{entry}: coverage description required")
        if row["status"] == "partial":
            require(text(row["limitations"]) and row.get("end_to_end", False) is False,
                    f"{entry}: partial needs limitations, cannot claim end-to-end")
        else:
            require(row["limitations"] == "" and row.get("end_to_end") is True
                    and text(row.get("verification")),
                    f"{entry}: complete requires end_to_end=true, no exclusions, verification")
        if "end_to_end" in row:
            require(type(row["end_to_end"]) is bool, f"{entry}: end_to_end must be Boolean")
        require(isinstance(row["evidence"], list) and row["evidence"]
                and all(text(path) for path in row["evidence"]), f"{entry}: evidence required")
        require(len(row["evidence"]) == len(set(row["evidence"])), f"{entry}: duplicate evidence")
        for link in row["evidence"]:
            path = Path(link)
            require(not path.is_absolute() and ".." not in path.parts
                    and (root / path).resolve().is_relative_to(root.resolve())
                    and (root / path).is_file(), f"{entry}: missing/unsafe evidence link {link}")
        if "intervals" in row:
            coverage = intervals(row["intervals"], f"{entry} coverage")
            require(coverage, f"{entry}: empty optional intervals")
            body = intervals(function["body_ranges"], f"{entry} body")
            require(all(any(c <= a <= b <= d for c, d in body) for a, b in coverage),
                    f"{entry}: coverage interval outside original function body")
            if row["status"] == "complete":
                require(coverage == body, f"{entry}: complete intervals must cover the entire body")
        counts[program][row["status"]] += 1
    return counts


def render(baseline, statuses):
    programs = baseline["programs"]
    total = sum(row["counts"]["internal_non_thunk"] for row in programs.values())
    complete = sum(row["complete"] for row in statuses.values())
    partial = sum(row["partial"] for row in statuses.values())
    percent = complete * 100 / total
    filled = complete * 30 // total
    lines = [
        START, "### Original-function reconstruction (base game only)", "",
        f"`[{'#' * filled}{'-' * (30 - filled)}] {percent:.2f}% complete`", "",
        f"**{complete:,} complete / {total:,} identified functions** "
        f"(frozen, estimated baseline); **{partial:,} unique functions partially reconstructed**.",
        "",
        "| Program | Counted functions | Complete | Partial |",
        "| --- | ---: | ---: | ---: |",
    ]
    for program in HASHES:
        row = programs[program]["counts"]
        lines.append(f"| {NAMES[program]} | {row['internal_non_thunk']:,} | "
                     f"{statuses[program]['complete']:,} | {statuses[program]['partial']:,} |")
    lines += [
        "",
        "This covers the **base game and original LS3D engine only**, not Sabre",
        "Squadron. The total is an estimate from Ghidra, including library code we",
        "haven't separated yet. Imported routines and small forwarding stubs are",
        "counted separately, outside this bar.",
        "",
        "The bar fills only when an entire original function is reconstructed.",
        "Our existing source recovers useful **parts** of functions, so it appears",
        "under Partial rather than Complete. Discoveries and research milestones",
        "are listed below; **0% complete does not mean no progress**.",
        "",
        "Ghidra can miss functions or get their boundaries wrong, so the baseline",
        "may be deliberately revised. This percentage does not measure understanding,",
        "difficulty, playability, or time remaining.",
        "",
        "[Status ledger](analysis/function-status.json) |",
        "[Frozen counts](analysis/function-baseline.json) |",
        "[Counting rules and refresh commands](analysis/function-progress.md)",
        END,
    ]
    return "\n".join(lines)


def replace_block(readme, block):
    require(readme.count(START) == 1 and readme.count(END) == 1,
            "README needs exactly one function-progress marker pair")
    start, end = readme.index(START), readme.index(END)
    require(start < end, "README progress markers reversed")
    return readme[:start] + block + readme[end + len(END):]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--check", action="store_true", help="validate inventories, ledger, baseline and README")
    action.add_argument("--render", action="store_true", help="update only the marked README block")
    action.add_argument("--freeze-baseline", action="store_true", help="explicitly replace frozen counts after review")
    parser.add_argument("--inventory-dir", type=Path, default=ROOT / "analysis/work/function-progress")
    args = parser.parse_args()
    try:
        inventories = {p: load(args.inventory_dir / f"{p}.json") for p in HASHES}
        functions, actual = make_baseline(inventories)
        statuses = validate_ledger(load(ROOT / "analysis/function-status.json"), functions)
        baseline_path = ROOT / "analysis/function-baseline.json"
        if args.freeze_baseline:
            baseline_path.write_text(json.dumps(actual, indent=2) + "\n", encoding="utf-8")
            print("Frozen baseline saved; review its diff, then run --render and --check.")
            return
        baseline = load(baseline_path)
        validate_baseline(baseline, actual)
        readme_path = ROOT / "README.md"
        current = readme_path.read_text(encoding="utf-8")
        expected = replace_block(current, render(baseline, statuses))
        if args.check:
            require(current == expected, "README progress block is stale: run --render")
            print("PASS: exact fingerprints, full inventories, frozen counts, unique statuses, evidence and README.")
        else:
            readme_path.write_text(expected, encoding="utf-8")
            print("README function-progress block updated.")
        for program in HASHES:
            print(f"{program}: {baseline['programs'][program]['counts']}; "
                  f"complete={statuses[program]['complete']}, partial={statuses[program]['partial']}")
    except (ValueError, KeyError, TypeError, OSError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
