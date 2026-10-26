"""Check the research map against its binary fingerprint and Ghidra exports."""

import argparse
import hashlib
import json
from pathlib import Path


def validate(document: dict, root: Path) -> None:
    binary = root / document["binary"]["path"]
    digest = hashlib.sha256(binary.read_bytes()).hexdigest()
    if digest != document["binary"]["sha256"]:
        raise ValueError("Research map does not match the executable hash")
    functions = document["functions"]
    addresses = [item["address"] for item in functions]
    if len(addresses) != len(set(addresses)):
        raise ValueError("Duplicate function addresses")
    records = {}
    for item in functions:
        path = root / item["export"]
        record = json.loads(path.read_text())
        if record["entry"] != item["address"] or record["sha256"] != digest:
            raise ValueError(f"Export identity mismatch: {path}")
        if not record["decompiled"] or not path.with_suffix(".c").read_text().strip():
            raise ValueError(f"Missing successful decompilation: {path}")
        records[item["address"]] = record
    for edge in document["direct_calls"]:
        source = records[edge["from"]]
        if edge["to"] not in records:
            raise ValueError(f"Unknown call destination: {edge}")
        if not any(
            ref["from"] == edge["site"]
            and ref["to"] == edge["to"]
            and "CALL" in ref["type"]
            for ref in source["references"]
        ):
            raise ValueError(f"Unsupported call edge: {edge}")
    for anchor in document["resource_anchors"]:
        if not any(
            ref["from"] == anchor["site"] and ref.get("string") == anchor["string"]
            for ref in records[anchor["function"]]["references"]
        ):
            raise ValueError(f"Unsupported resource anchor: {anchor}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "map",
        type=Path,
        nargs="?",
        default=Path(__file__).resolve().parents[1] / "analysis/initialization-map.json",
    )
    args = parser.parse_args()
    document = json.loads(args.map.read_text())
    validate(document, args.map.resolve().parent)
    print(
        f"Verified binary fingerprint, {len(document['functions'])} function exports, "
        f"{len(document['direct_calls'])} direct-call edges, and "
        f"{len(document['resource_anchors'])} resource anchors."
    )
