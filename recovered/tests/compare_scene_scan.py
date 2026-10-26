"""Cross-check the C++ reader against the independent Python scene inspector."""

import argparse
import json
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from inspect_scene2 import summarize


def chunk(kind: int, payload: bytes = b"") -> bytes:
    return struct.pack("<HI", kind, len(payload) + 6) + payload


def expected_counts(data: bytes) -> dict:
    summary = summarize(data)
    return {
        "size": summary["size"],
        "top_level_chunks": len(summary["top_level_chunks"]),
        **{key: summary[key] for key in (
            "object_containers", "object_records", "record_field_counts",
            "uninterpreted_record_kinds",
        )},
    }


def compare(scanner: Path, scene: Path) -> None:
    expected = expected_counts(scene.read_bytes())
    result = subprocess.run([str(scanner), str(scene)], capture_output=True, text=True)
    if result.returncode != 0:
        raise ValueError(f"C++ scanner failed for {scene}: {result.stderr}")
    actual = json.loads(result.stdout)
    if actual != expected:
        raise ValueError(f"C++/Python structural mismatch for {scene}: {actual} != {expected}")


def fixtures() -> list[bytes]:
    result = [
        chunk(0x4C53),
        chunk(0x4C53, chunk(0xFFFF, b"opaque, not nested chunks")),
        chunk(0x4C53, chunk(0x4000, chunk(0x4010))),
        chunk(0x4C53, chunk(0x4000, chunk(0x9999, b"opaque record"))),
        chunk(0x4C53,
              chunk(0x4000, chunk(0x4010, chunk(0x40A0, chunk(0x0010, b"opaque\0"))))
              + chunk(0x4000, chunk(0x4010, chunk(0x0010, b"named\0")))),
    ]
    generator = random.Random(0x4C53)
    for _ in range(32):
        sections = []
        for _ in range(generator.randrange(1, 5)):
            records = []
            for _ in range(generator.randrange(8)):
                fields = [chunk(0x0010, f"synthetic_{generator.randrange(1000)}".encode() + b"\0")]
                for _ in range(generator.randrange(6)):
                    kind = generator.choice([0x20, 0x22, 0x2D, 0x40A0, 0xFFFF])
                    fields.append(chunk(kind, generator.randbytes(generator.randrange(32))))
                records.append(chunk(0x4010, b"".join(fields)))
            sections.append(chunk(0x4000, b"".join(records)))
            sections.append(chunk(0xABCD, generator.randbytes(generator.randrange(32))))
        result.append(chunk(0x4C53, b"".join(sections)))
    return result


def malformed_fixtures() -> list[bytes]:
    return [
        b"",
        b"short",
        chunk(1),
        chunk(0x4C53) + chunk(1),
        chunk(0x4C53) + b"x",
        chunk(0x4C53, b"short"),
        chunk(0x4C53, struct.pack("<HI", 1, 5)),
        chunk(0x4C53, struct.pack("<HI", 1, 0xFFFFFFFF)),
        chunk(0x4C53, chunk(0x4000, chunk(0x4010, b"short"))),
    ]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scanner", type=Path, required=True)
    parser.add_argument("--scene", type=Path, help="Optionally check a local real mission")
    args = parser.parse_args()
    scanner = args.scanner.resolve(strict=True)
    valid = fixtures()
    invalid = malformed_fixtures()
    with tempfile.TemporaryDirectory(prefix="hd2-reader-compare-") as temporary:
        scene = Path(temporary) / "synthetic.bin"
        for data in valid:
            scene.write_bytes(data)
            compare(scanner, scene)
        for data in invalid:
            scene.write_bytes(data)
            try:
                expected_counts(data)
            except ValueError:
                pass
            else:
                raise ValueError("Malformed synthetic fixture was accepted by Python")
            result = subprocess.run([str(scanner), str(scene)], capture_output=True, text=True)
            if result.returncode == 0 or result.stdout or "Scene scan failed:" not in result.stderr:
                raise ValueError(f"Malformed fixture did not produce an explicit C++ error: {result}")
    print(f"Matched {len(valid)} synthetic scans; both implementations rejected {len(invalid)} malformed scenes.")
    if args.scene is not None:
        compare(scanner, args.scene)
        print("Real mission size, container/record counts and complete field census match Python.")


if __name__ == "__main__":
    main()
