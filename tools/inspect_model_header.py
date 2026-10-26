"""Decode only the verified 16-byte prefix of a version-0x29 model file."""

import argparse
import json
from pathlib import Path
import struct


def decode_model_header(data: bytes) -> dict:
    if len(data) < 4:
        raise ValueError("Incomplete model signature")
    if data[:4] != b"4DS\0":
        raise ValueError("Invalid model signature")
    if len(data) < 6:
        raise ValueError("Incomplete model version")
    version = struct.unpack_from("<H", data, 4)[0]
    if version != 0x29:
        raise ValueError("Unsupported model version")
    if len(data) < 16:
        raise ValueError("Incomplete model metadata/count")
    count = struct.unpack_from("<H", data, 14)[0]
    if count > 6000:
        raise ValueError("Model first-table count exceeds 6000")
    return {
        "version": version,
        "opaque_metadata_hex": data[6:14].hex(),
        "first_table_entry_count": count,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("model", type=Path)
    args = parser.parse_args()
    with args.model.open("rb") as stream:
        print(json.dumps(decode_model_header(stream.read(16)), indent=2))


if __name__ == "__main__":
    main()
