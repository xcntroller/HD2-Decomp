"""Check curated mission observations, fingerprints and selected local Ghidra evidence."""

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path

from inspect_model_header import decode_model_header
from inspect_scene2 import decode_objects, summarize


def fingerprint(item: dict, root: Path) -> str:
    path = root / item["path"]
    if "size" in item and path.stat().st_size != item["size"]:
        raise ValueError(f"File size mismatch: {path}")
    with path.open("rb") as stream:
        digest = hashlib.file_digest(stream, "sha256").hexdigest()
    if digest != item["sha256"]:
        raise ValueError(f"File fingerprint mismatch: {path}")
    return digest


def check_equal(actual: object, expected: object, label: str) -> None:
    if actual != expected:
        raise ValueError(f"Mission evidence mismatch: {label}")


def validate(document: dict, root: Path, verify_archive: bool = False) -> None:
    digest = fingerprint(document["binary"], root)
    fingerprint(document["engine"], root)
    if verify_archive:
        fingerprint(document["archive"], root)
    samples = document["samples"]
    for sample in samples:
        fingerprint(sample, root)
        if "model_header" in sample:
            with (root / sample["path"]).open("rb") as stream:
                check_equal(decode_model_header(stream.read(16)), sample["model_header"],
                            f"model prefix {sample['path']}")
    scene = next(sample for sample in samples if Path(sample["path"]).name == "scene2.bin")
    data = (root / scene["path"]).read_bytes()
    summary = summarize(data)
    expected = document["observations"]
    check_equal(len(summary["top_level_chunks"]), expected["top_level_chunks"], "root children")
    check_equal(summary["object_containers"], 1, "object containers")
    check_equal(summary["object_records"], expected["object_records"], "object records")
    check_equal(summary["nul_terminated_name_fields"], expected["object_records"], "name fields")
    check_equal(summary["record_field_counts"], expected["record_field_counts"], "field census")
    check_equal(summary["uninterpreted_record_kinds"], {}, "unknown record kinds")
    container = next(item for item in summary["top_level_chunks"] if item["id"] == "0x4000")
    check_equal(container["offset"], expected["object_container_offset"], "container offset")
    check_equal(container["size"], expected["object_container_total_size"], "container size")
    decoded = decode_objects(data)
    observed = expected["object_decoding"]
    check_equal(len(decoded["records"]), observed["decoded_records"], "decoded records")
    for key in ("creation_kind_field_counts", "name_action_counts", "decoded_field_counts"):
        check_equal(decoded[key], observed[key], key)
    fields = [field for record in decoded["records"] for field in record["fields"]]
    names = {field["value"] for field in fields if field["id"] == "0x0010"}
    parents = [
        child["name"] for field in fields if field["id"] == "0x4020"
        for child in field["children"] if "name" in child
    ]
    check_equal(len(names), observed["unique_object_names"], "unique names")
    check_equal(len(parents), observed["parent_name_fields"], "parent names")
    check_equal(sum(name in names for name in parents),
                observed["parent_names_matching_scene2_names"], "parent-name matches")
    resources = [field for field in fields if field["id"] == "0x2012"]
    check_equal(
        {
            "fields": len(resources),
            "unique_names": len({field["value"] for field in resources}),
            "unique_lookup_keys": len({field["lookup_key"] for field in resources}),
            "extensionless_names": sum("." not in field["value"] for field in resources),
            "active_creation_kind_counts": dict(sorted(Counter(
                str(field["active_creation_kind"]) for field in resources
            ).items())),
        },
        observed["model_resources"], "model resource observations",
    )
    tolerance = observed["rotation_squared_norm_tolerance"]
    if not 0 < tolerance <= 0.000001:
        raise ValueError("Rotation squared-norm tolerance must be in (0, 1e-6]")
    for field in fields:
        if field["id"] == "0x0022":
            if abs(sum(value * value for value in field["value"]) - 1) > tolerance:
                raise ValueError(f"Sample quaternion norm mismatch at {field['offset']:#x}")
    exports = {}
    for entry in document["loader_evidence"]["exports"]:
        path = root / entry["path"]
        record = json.loads(path.read_text())
        address = entry["address"]
        if address in exports:
            raise ValueError(f"Duplicate evidence function: {address}")
        check_equal(record["entry"], address, "export address")
        check_equal(record["sha256"], digest, "export executable hash")
        if not record["decompiled"] or not path.with_suffix(".c").read_text().strip():
            raise ValueError(f"Missing successful decompilation: {path}")
        if "signature" in entry:
            check_equal(record["signature"], entry["signature"], f"signature {address}")
        exports[address] = record
    for call in document["loader_evidence"]["calls"]:
        if not any(
            ref["from"] == call["site"] and ref["to"] == call["to"]
            and "CALL" in ref["type"]
            and ("symbol" not in call or ref.get("symbol") == call["symbol"])
            for ref in exports[call["from"]]["references"]
        ):
            raise ValueError(f"Unsupported loader call: {call}")
    for anchor in document["loader_evidence"].get("resource_anchors", []):
        if not any(
            ref["from"] == anchor["site"] and ref.get("string") == anchor["string"]
            for ref in exports[anchor["function"]]["references"]
        ):
            raise ValueError(f"Unsupported model resource anchor: {anchor}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--map", type=Path, default=root / "analysis/mission-format.json")
    parser.add_argument("--root", type=Path, default=root)
    parser.add_argument("--archive", action="store_true", help="Also hash the full missions archive")
    args = parser.parse_args()
    validate(json.loads(args.map.read_text()), args.root, args.archive)
    print("Verified mission fingerprints, model prefixes, exact record/field counts, quaternion tolerance, "
          "name-link/model-resource observations, repaired signatures and selected loader evidence.")


if __name__ == "__main__":
    main()
