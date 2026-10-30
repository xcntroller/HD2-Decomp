"""Verify original client chunk-file behavior and delegated rw_data exports."""

import argparse
import hashlib
import json
from pathlib import Path
import struct

from validate_engine_host import validate as validate_host
from validate_engine_model import PEImage
from validate_frontend import validate_image
from validate_sound_codec import RW_DATA_SHA256


def validate(document: dict, root: Path) -> dict:
    counts = validate_host(document, root)
    record = document["rw_data"]
    data = (root / record["binary"]["path"]).read_bytes()
    if record["binary"]["sha256"] != RW_DATA_SHA256 or hashlib.sha256(data).hexdigest() != RW_DATA_SHA256:
        raise ValueError("Unsupported chunk-file rw_data fingerprint")
    counts["rw_data"] = validate_image(PEImage(data), record)
    return counts


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--map", type=Path, default=root / "analysis/chunk-file-map.json")
    parser.add_argument("--root", type=Path, default=root)
    args = parser.parse_args()
    try:
        counts = validate(json.loads(args.map.read_text()), args.root)
    except (ValueError, KeyError, OSError, struct.error) as error:
        parser.exit(1, f"Chunk-file evidence validation failed: {error}\n")
    print(f"Verified three fingerprints; {sum(c['instructions'] for c in counts.values())} "
          f"instruction/data anchors and {sum(c['calls'] for c in counts.values())} calls. "
          "Source-only; no original file API or game executed.")


if __name__ == "__main__":
    main()
