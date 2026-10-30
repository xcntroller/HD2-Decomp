"""Check menu-first static evidence against the exact base client, without execution."""

import argparse
import hashlib
import json
from pathlib import Path
import struct

from validate_engine_model import PEImage
from validate_model_load_policy import CLIENT_SHA256


def validate_image(image: PEImage, document: dict) -> dict:
    if image.image_base != int(document["binary"]["image_base"], 16):
        raise ValueError("Front-end image-base mismatch")
    for storage in document.get("zero_initialized_storage", []):
        rva = int(storage["address"], 16) - image.image_base
        size = storage["size"]
        if size <= 0 or not any(start + raw_size <= rva and rva + size <= start + extent
                                for start, extent, _, raw_size in image.sections):
            raise ValueError("Front-end storage is not entirely PE zero-initialized memory")
    for anchor in document["instruction_anchors"]:
        image.expect(int(anchor["address"], 16), bytes.fromhex(anchor["hex"]),
                     "Front-end instruction")
    slots = 0
    seen_tables = set()
    for table in document["vtables"]:
        address = int(table["address"], 16)
        if address in seen_tables or not table["targets"]:
            raise ValueError("Duplicate or empty front-end vtable")
        seen_tables.add(address)
        for index, target in enumerate(table["targets"]):
            image.expect(address + index * 4, struct.pack("<I", int(target, 16)),
                         "Front-end vtable")
            slots += 1
    for call in document["direct_calls"]:
        site = int(call["site"], 16)
        expected = b"\xe8" + struct.pack("<i", int(call["target"], 16) - site - 5)
        image.expect(site, expected, "Front-end direct call")
    for anchor in document["resource_anchors"]:
        address = int(anchor["address"], 16)
        image.expect(int(anchor["site"], 16), b"\x68" + struct.pack("<I", address),
                     "Front-end resource reference")
        image.expect(address, anchor["text"].encode("ascii") + b"\0",
                     "Front-end resource string")
    return {
        "instructions": len(document["instruction_anchors"]),
        "vtable_slots": slots,
        "calls": len(document["direct_calls"]),
        "resources": len(document["resource_anchors"]),
    }


def validate(document: dict, root: Path) -> dict:
    binary = document["binary"]
    if binary["sha256"] != CLIENT_SHA256:
        raise ValueError("Unsupported client fingerprint in front-end map")
    data = (root / binary["path"]).read_bytes()
    if hashlib.sha256(data).hexdigest() != CLIENT_SHA256:
        raise ValueError("Front-end client fingerprint mismatch")
    return validate_image(PEImage(data), document)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--map", type=Path, default=root / "analysis/frontend-map.json")
    parser.add_argument("--root", type=Path, default=root)
    args = parser.parse_args()
    try:
        counts = validate(json.loads(args.map.read_text()), args.root)
    except (ValueError, KeyError, OSError, struct.error) as error:
        parser.exit(1, f"Front-end evidence validation failed: {error}\n")
    print(f"Verified client fingerprint; {counts['instructions']} instruction anchors, "
          f"{counts['vtable_slots']} vtable slots, {counts['calls']} direct calls "
          f"and {counts['resources']} resource/control-name anchors.")


if __name__ == "__main__":
    main()
