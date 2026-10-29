"""Validate codec-32 evidence against the original rw_data.dll fingerprint."""

import argparse
import hashlib
import json
from pathlib import Path
import struct

from validate_engine_model import PEImage
from validate_frontend import validate_image

RW_DATA_SHA256 = "95a65ee003e2d7ab6047ed47d9adad1db330bd5482e33cfb18127d15af5879be"


def validate(document: dict, root: Path) -> dict:
    binary = document["binary"]
    if binary["sha256"] != RW_DATA_SHA256:
        raise ValueError("Unsupported rw_data fingerprint in sound-codec map")
    data = (root / binary["path"]).read_bytes()
    if hashlib.sha256(data).hexdigest() != RW_DATA_SHA256:
        raise ValueError("rw_data fingerprint mismatch")
    image = PEImage(data)
    counts = validate_image(image, document)
    table = document["codec32_table"]
    actual = image.read_va(int(table["address"], 16), 256)
    if table["entries"] != 128 or hashlib.sha256(actual).hexdigest() != table["sha256"]:
        raise ValueError("Codec-32 magnitude table fingerprint mismatch")
    expected = struct.pack("<128h", *(i * i * 32767 // (127 * 127) for i in range(128)))
    if actual != expected:
        raise ValueError("Codec-32 quadratic magnitude formula mismatch")
    return counts


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument("--map", type=Path, default=root / "analysis/sound-codec-map.json")
    parser.add_argument("--root", type=Path, default=root)
    args = parser.parse_args()
    try:
        counts = validate(json.loads(args.map.read_text()), args.root)
    except (ValueError, KeyError, OSError, struct.error) as error:
        parser.exit(1, f"Sound-codec evidence validation failed: {error}\n")
    print(f"Verified rw_data fingerprint, {counts['instructions']} instruction anchors "
          "and all 128 codec-32 magnitudes. Not runtime proof.")


if __name__ == "__main__":
    main()
