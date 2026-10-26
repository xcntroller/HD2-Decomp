"""Cross-check supported model prefixes; optionally inspect local model files."""

import argparse
import json
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from inspect_model_header import decode_model_header


def compare(reader: Path, path: Path) -> None:
    with path.open("rb") as stream:
        expected = decode_model_header(stream.read(16))
    result = subprocess.run([str(reader), str(path)], capture_output=True, text=True)
    if result.returncode != 0:
        raise ValueError(f"C++ model-header reader failed: {result.stderr}")
    if json.loads(result.stdout) != expected:
        raise ValueError("C++/Python model-header mismatch")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reader", type=Path, required=True)
    parser.add_argument("--model", type=Path, action="append", default=[])
    args = parser.parse_args()
    reader = args.reader.resolve(strict=True)
    generator = random.Random(0x4D530029)
    valid = [
        b"4DS\0" + struct.pack("<H", 0x29) + generator.randbytes(8) + struct.pack("<H", count)
        for count in [0, 1, 13, 86, 5999, 6000]
        + [generator.randrange(6001) for _ in range(24)]
    ]
    invalid = [valid[0][:size] for size in range(16)]
    for index in range(4):
        data = bytearray(valid[0])
        data[index] ^= 1
        invalid.append(bytes(data))
    for version in [0, 0x28, 0x2A, 0xFFFF]:
        invalid.append(b"4DS\0" + struct.pack("<H", version) + valid[0][6:])
    for count in [6001, 32768, 65535]:
        invalid.append(valid[0][:14] + struct.pack("<H", count))
    with tempfile.TemporaryDirectory(prefix="hd2-model-header-") as directory:
        path = Path(directory) / "synthetic.4ds"
        for data in valid:
            path.write_bytes(data + b"unread payload")
            compare(reader, path)
        for data in invalid:
            path.write_bytes(data)
            try:
                decode_model_header(data)
            except ValueError:
                pass
            else:
                raise ValueError("Python accepted a malformed synthetic model header")
            result = subprocess.run([str(reader), str(path)], capture_output=True, text=True)
            if result.returncode != 1 or result.stdout or not result.stderr.strip():
                raise ValueError("C++ must reject malformed headers explicitly without JSON success")
    print(f"Matched {len(valid)} synthetic headers; both readers rejected {len(invalid)} malformed headers.")
    for model in args.model:
        compare(reader, model)
    if args.model:
        print(f"Matched {len(args.model)} local model-file headers.")


if __name__ == "__main__":
    main()
