"""Check the client's model-result switch tables and instruction anchors read-only."""

import argparse
import hashlib
import json
from pathlib import Path
import struct

from validate_engine_model import PEImage


CLIENT_SHA256 = "8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"


def validate_image(image: PEImage, document: dict) -> None:
    for anchor in document["instruction_anchors"]:
        expected = bytes.fromhex(anchor["hex"])
        address = int(anchor["address"], 16)
        if image.read_va(address, len(expected)) != expected:
            raise ValueError(f"Model-load instruction mismatch at {address:#x}")
    dispatch = document["dispatch"]
    selectors = image.read_va(int(dispatch["selector_address"], 16), 20)
    if list(selectors) != dispatch["selectors"]:
        raise ValueError("Model-load result selector table mismatch")
    targets = struct.unpack("<5I", image.read_va(int(dispatch["target_address"], 16), 20))
    if targets != tuple(int(value, 16) for value in dispatch["targets"]):
        raise ValueError("Model-load result target table mismatch")
    if any(selector >= len(targets) for selector in selectors):
        raise ValueError("Model-load selector exceeds target table")
    for anchor in document["diagnostics"]:
        site = int(anchor["site"], 16)
        address = int(anchor["string_address"], 16)
        if image.read_va(site, 5) != b"\x68" + struct.pack("<I", address):
            raise ValueError(f"Model-load diagnostic reference mismatch at {site:#x}")
        expected = anchor["text"].encode("ascii") + b"\0"
        if image.read_va(address, len(expected)) != expected:
            raise ValueError(f"Model-load diagnostic text mismatch at {address:#x}")


def validate(document: dict, root: Path) -> None:
    binary = document["binary"]
    if binary["sha256"] != CLIENT_SHA256:
        raise ValueError("Unsupported client fingerprint in model-load map")
    data = (root / binary["path"]).read_bytes()
    if hashlib.sha256(data).hexdigest() != CLIENT_SHA256:
        raise ValueError("Model-load client fingerprint mismatch")
    validate_image(PEImage(data), document)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--map", type=Path, default=root / "analysis/model-load-policy.json")
    parser.add_argument("--root", type=Path, default=root)
    args = parser.parse_args()
    validate(json.loads(args.map.read_text()), args.root)
    print("Verified client fingerprint, model-open arguments, result-switch tables, "
          "signed success/retry branches and version/time diagnostic anchors.")


if __name__ == "__main__":
    main()
