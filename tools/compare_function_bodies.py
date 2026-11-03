"""Compare bounded original PE32 bodies with compiled i386 COFF functions.

Only verified direct-call relocation operands are normalized. Differences are
reported, never treated as proof of behavioral equivalence or completion.
"""

import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

from validate_engine_model import PEImage
from validate_frontend import validate


def disassemble(tool, path):
    result = subprocess.run([str(tool), "-d", "-r", str(path)], check=True,
                            capture_output=True, text=True)
    if "file format coff-i386" not in result.stdout:
        raise ValueError("Expected a compiled i386 COFF object")
    functions = {}
    current = None
    for line in result.stdout.splitlines():
        label = re.fullmatch(r"([0-9a-fA-F]+) <([^>]+)>:", line)
        if label:
            name = label[2]
            if name in functions:
                raise ValueError(f"Ambiguous function symbol: {name}")
            current = {"start": int(label[1], 16), "instructions": [], "relocations": []}
            functions[name] = current
            continue
        if current is None:
            continue
        relocation = re.fullmatch(r"\s*([0-9a-fA-F]+):\s+(IMAGE_REL_\S+)\s+(\S+)\s*", line)
        if relocation:
            current["relocations"].append((int(relocation[1], 16), relocation[2], relocation[3]))
            continue
        instruction = re.fullmatch(r"\s*([0-9a-fA-F]+):[ \t]+((?:[0-9a-fA-F]{2}[ \t]+)+)([A-Za-z.(].*)", line)
        if instruction:
            current["instructions"].append((int(instruction[1], 16),
                                             bytes.fromhex(instruction[2]), instruction[3]))
    return functions


def compare(document, root, object_path, tool, inventory):
    # Reuse the original fingerprint/instruction/call guards; do not run it.
    validate(document, root)
    original = PEImage((root / document["binary"]["path"]).read_bytes())
    if inventory["sha256"] != document["binary"]["sha256"]:
        raise ValueError("Inventory fingerprint mismatch")
    census = {row["entry"]: row for row in inventory["functions"]}
    rows = document["comparison_functions"]
    symbols = {row["candidate_symbol"]: row["entry"] for row in rows}
    if len(symbols) != len(rows) or len({row["entry"] for row in rows}) != len(rows):
        raise ValueError("Duplicate comparison entry/symbol")
    candidates = disassemble(tool, object_path)
    results = []
    for row in rows:
        entry = row["entry"]
        start = int(entry, 16)
        function = census[entry]
        ranges = function["body_ranges"]
        if (function["external"] or function["thunk"] or len(ranges) != 1
                or int(ranges[0][0], 16) != start):
            raise ValueError("Comparison requires a single contiguous internal body")
        size = int(ranges[0][1], 16) - start + 1
        anchors = [anchor for anchor in document["instruction_anchors"]
                   if int(anchor["address"], 16) == start]
        if (len(anchors) != 1 or len(bytes.fromhex(anchors[0]["hex"])) != size
                or function["body_bytes"] != size):
            raise ValueError("Full-body anchor and inventory extent disagree")
        expected = original.read_va(start, size)
        candidate = candidates[row["candidate_symbol"]]
        instructions = candidate["instructions"]
        if not instructions:
            raise ValueError("Candidate body is empty")
        cursor = candidate["start"]
        for address, raw, _ in instructions:
            if address != cursor:
                raise ValueError("Noncontiguous candidate disassembly")
            cursor += len(raw)
        actual = b"".join(raw for _, raw, _ in instructions)
        normalized_expected = bytearray(expected)
        expected_calls = []
        for call in document["direct_calls"]:
            site = int(call["site"], 16)
            if start <= site < start + size:
                if site + 5 > start + size:
                    raise ValueError("Call exceeds original body")
                normalized_expected[site - start + 1:site - start + 5] = b"\0" * 4
                expected_calls.append(call["target"])
        normalized_actual = bytearray(actual)
        actual_calls = []
        for address, kind, symbol in candidate["relocations"]:
            offset = address - candidate["start"]
            instruction = next((raw for site, raw, _ in instructions if site + 1 == address), None)
            if (kind != "IMAGE_REL_I386_REL32" or symbol not in symbols
                    or instruction is None or len(instruction) != 5
                    or instruction[0] not in (0xe8, 0xe9)
                    or struct.unpack("<i", actual[offset:offset + 4])[0] != 0):
                raise ValueError(f"Unaccounted candidate relocation: {entry}: {symbol}")
            normalized_actual[offset:offset + 4] = b"\0" * 4
            actual_calls.append(symbols[symbol])
        # A candidate with unrelated relocations or targets must never be called
        # an exact match just because the four-byte displacements were masked.
        same_targets = actual_calls == expected_calls
        exact = normalized_actual == normalized_expected and same_targets
        results.append({
            "entry": entry, "candidate_symbol": row["candidate_symbol"],
            "original_bytes": size, "candidate_bytes": len(actual),
            "original_sha256": hashlib.sha256(expected).hexdigest(),
            "candidate_sha256": hashlib.sha256(actual).hexdigest(),
            "original_calls": expected_calls, "candidate_calls_including_tail_jumps": actual_calls,
            "same_call_target_order": same_targets,
            "relocation_normalized_exact": exact,
            "normalized_original_hex": normalized_expected.hex(),
            "normalized_candidate_hex": normalized_actual.hex(),
            "candidate_instructions": [text for _, _, text in instructions],
            "byte_diff": list(difflib.unified_diff(
                [normalized_expected[i:i+8].hex() for i in range(0, size, 8)],
                [normalized_actual[i:i+8].hex() for i in range(0, len(actual), 8)],
                fromfile="original", tofile="candidate", lineterm="")),
        })
    return {"original_sha256": document["binary"]["sha256"],
            "object_sha256": hashlib.sha256(object_path.read_bytes()).hexdigest(),
            "comparison": "Exact bytes after verified direct-call relocation operands only; no semantic equivalence inferred.",
            "exact": sum(row["relocation_normalized_exact"] for row in results),
            "different": sum(not row["relocation_normalized_exact"] for row in results),
            "functions": results}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--map", type=Path, required=True)
    parser.add_argument("--object", type=Path, required=True)
    parser.add_argument("--objdump", type=Path, required=True)
    parser.add_argument("--inventory", type=Path, default=Path("analysis/work/function-progress/client.json"))
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = compare(json.loads(args.map.read_text()), args.root, args.object, args.objdump,
                         json.loads(args.inventory.read_text()))
        args.output.write_text(json.dumps(result, indent=2) + "\n")
    except (KeyError, ValueError, OSError, struct.error, subprocess.CalledProcessError) as error:
        parser.exit(1, f"Function comparison failed: {error}\n")
    print(f"Compared {len(result['functions'])} bodies: {result['exact']} relocation-normalized exact, "
          f"{result['different']} different. Static comparison only.")


if __name__ == "__main__":
    main()
