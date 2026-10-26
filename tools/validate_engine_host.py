"""Check engine-host ABI maps against both fingerprinted original binaries."""

import argparse
import hashlib
import json
from pathlib import Path
import struct

from validate_engine_model import ENGINE_SHA256, PEImage
from validate_frontend import validate_image
from validate_model_load_policy import CLIENT_SHA256


def validate(document: dict, root: Path) -> dict:
    identities = (("engine", ENGINE_SHA256), ("client", CLIENT_SHA256))
    for key, expected in identities:
        if document[key]["binary"]["sha256"] != expected:
            raise ValueError(f"Unsupported {key} fingerprint in engine-host map")
    counts = {}
    for key, expected in identities:
        record = document[key]
        data = (root / record["binary"]["path"]).read_bytes()
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError(f"Engine-host {key} fingerprint mismatch")
        counts[key] = validate_image(PEImage(data), record)
    return counts


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--map", type=Path, default=root / "analysis/engine-host-map.json")
    parser.add_argument("--root", type=Path, default=root)
    args = parser.parse_args()
    try:
        counts = validate(json.loads(args.map.read_text()), args.root)
    except (ValueError, KeyError, OSError, struct.error) as error:
        parser.exit(1, f"Engine-host evidence validation failed: {error}\n")
    print(f"Verified both fingerprints; {sum(c['instructions'] for c in counts.values())} "
          f"instruction anchors, {counts['engine']['vtable_slots']} engine vtable slots "
          f"and {counts['engine']['resources']} prerequisite references. Not runtime proof.")


if __name__ == "__main__":
    main()
