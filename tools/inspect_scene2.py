"""Validate scene2.bin framing and summarize known container levels, not gameplay semantics."""

import argparse
from collections import Counter
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import struct


@dataclass(frozen=True)
class Chunk:
    offset: int
    kind: int
    size: int

    @property
    def end(self) -> int:
        return self.offset + self.size


def chunks(data: bytes, start: int, end: int) -> list[Chunk]:
    if not 0 <= start <= end <= len(data):
        raise ValueError("Invalid chunk-container bounds")
    result = []
    while start < end:
        if end - start < 6:
            raise ValueError(f"Truncated chunk header at {start:#x}")
        kind, size = struct.unpack_from("<HI", data, start)
        if size < 6 or start + size > end:
            raise ValueError(f"Invalid chunk size {size} at {start:#x}")
        result.append(Chunk(start, kind, size))
        start += size
    return result


def summarize(data: bytes) -> dict:
    roots = chunks(data, 0, len(data))
    if len(roots) != 1 or roots[0].kind != 0x4C53:
        raise ValueError("Expected one 0x4c53 root covering the complete file")
    root = roots[0]
    top = chunks(data, root.offset + 6, root.end)
    containers = [item for item in top if item.kind == 0x4000]
    field_counts: Counter[int] = Counter()
    record_count = 0
    name_count = 0
    unknown_record_kinds: Counter[int] = Counter()
    for container in containers:
        for record in chunks(data, container.offset + 6, container.end):
            if record.kind != 0x4010:
                unknown_record_kinds[record.kind] += 1
                continue
            record_count += 1
            for field in chunks(data, record.offset + 6, record.end):
                field_counts[field.kind] += 1
                if field.kind == 0x10:
                    value = data[field.offset + 6:field.end]
                    if not value or value[-1] != 0 or 0 in value[:-1]:
                        raise ValueError(f"Invalid name-like field at {field.offset:#x}")
                    name_count += 1
    return {
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "root_id": "0x4c53",
        "top_level_chunks": [
            {"offset": item.offset, "id": f"0x{item.kind:04x}", "size": item.size}
            for item in top
        ],
        "object_containers": len(containers),
        "object_records": record_count,
        "nul_terminated_name_fields": name_count,
        "record_field_counts": {
            f"0x{kind:04x}": count for kind, count in sorted(field_counts.items())
        },
        "uninterpreted_record_kinds": {
            f"0x{kind:04x}": count for kind, count in sorted(unknown_record_kinds.items())
        },
        "scope": "Structural checks only; unknown payloads are opaque. Records are not soldiers or gameplay entities by definition.",
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scene", type=Path)
    parser.add_argument("--output", type=Path, help="Write a new JSON file instead of stdout")
    args = parser.parse_args()
    report = json.dumps(summarize(args.scene.read_bytes()), indent=2) + "\n"
    if args.output is None:
        print(report, end="")
    else:
        with args.output.open("x") as output:
            output.write(report)
        print(f"Wrote structural report to {args.output}")


if __name__ == "__main__":
    main()
