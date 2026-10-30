"""Verify original startup archive setup and delegated rw_data ABI evidence."""

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
        raise ValueError("Unsupported startup rw_data fingerprint")
    counts["rw_data"] = validate_image(PEImage(data), record)
    return counts


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--map", type=Path, default=root / "analysis/startup-archives-map.json")
    parser.add_argument("--root", type=Path, default=root)
    args = parser.parse_args()
    try:
        counts = validate(json.loads(args.map.read_text()), args.root)
    except (ValueError, KeyError, OSError, struct.error) as error:
        parser.exit(1, f"Startup archive evidence validation failed: {error}\n")
    print(f"Verified three fingerprints; {sum(c['instructions'] for c in counts.values())} "
          f"instruction anchors, {sum(c['vtable_slots'] for c in counts.values())} vtable slots, "
          f"{sum(c['calls'] for c in counts.values())} direct calls and "
          f"{sum(c['resources'] for c in counts.values())} resource references. "
          "Source-only; protection prelude excluded, rw_data remains delegated.")


if __name__ == "__main__":
    main()
