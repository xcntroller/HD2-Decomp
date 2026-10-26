"""Validate scene2.bin framing and summarize known container levels, not gameplay semantics."""

import argparse
from collections import Counter
from dataclasses import dataclass
import hashlib
import json
import math
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


def scene_chunks(data: bytes) -> list[Chunk]:
    roots = chunks(data, 0, len(data))
    if len(roots) != 1 or roots[0].kind != 0x4C53:
        raise ValueError("Expected one 0x4c53 root covering the complete file")
    root = roots[0]
    return chunks(data, root.offset + 6, root.end)


def name_value(data: bytes, field: Chunk) -> bytes:
    value = data[field.offset + 6:field.end]
    if not value or value[-1] != 0 or 0 in value[:-1]:
        raise ValueError(f"Invalid name-like field at {field.offset:#x}")
    return value[:-1]


def summarize(data: bytes) -> dict:
    top = scene_chunks(data)
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
                    name_value(data, field)
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


def decode_field(data: bytes, field: Chunk) -> dict:
    result = {
        "id": f"0x{field.kind:04x}",
        "offset": field.offset,
        "payload_size": field.size - 6,
    }
    if field.kind == 0x10:
        result.update(meaning="object_name", value=name_value(data, field).decode("ascii"))
    elif field.kind == 0x4011:
        if field.size != 10:
            raise ValueError(f"Expected a 4-byte creation kind at {field.offset:#x}")
        result.update(meaning="creation_kind", value=struct.unpack_from("<I", data, field.offset + 6)[0])
    elif field.kind in (0x20, 0x22, 0x2C, 0x2D):
        meaning, count = {
            0x20: ("position", 3),
            0x22: ("rotation_wxyz", 4),
            0x2C: ("fallback_position", 3),
            0x2D: ("scale", 3),
        }[field.kind]
        if field.size != 6 + count * 4:
            raise ValueError(f"Invalid {meaning} size at {field.offset:#x}")
        values = list(struct.unpack_from(f"<{count}f", data, field.offset + 6))
        if not all(math.isfinite(value) for value in values):
            raise ValueError(f"Nonfinite {meaning} value at {field.offset:#x}")
        result.update(meaning=meaning, value=values)
        if field.kind == 0x22:
            result["engine_xyzw_before_normalization"] = values[1:] + values[:1]
    elif field.kind == 0x4020:
        result.update(meaning="parent_reference", children=[
            {
                "id": f"0x{child.kind:04x}",
                "offset": child.offset,
                "payload_size": child.size - 6,
                **({"name": name_value(data, child).decode("ascii")} if child.kind == 0x10 else {}),
            }
            for child in chunks(data, field.offset + 6, field.end)
        ])
    return result


def decode_objects(data: bytes) -> dict:
    records = []
    creation_kinds: Counter[int] = Counter()
    actions: Counter[str] = Counter()
    decoded_counts: Counter[str] = Counter()
    for container in scene_chunks(data):
        if container.kind != 0x4000:
            continue
        for record in chunks(data, container.offset + 6, container.end):
            if record.kind != 0x4010:
                continue
            fields = []
            active_kind = 0
            for field in chunks(data, record.offset + 6, record.end):
                decoded = decode_field(data, field)
                if "meaning" in decoded:
                    decoded_counts[decoded["meaning"]] += 1
                if field.kind == 0x4011:
                    active_kind = decoded["value"]
                    creation_kinds[active_kind] += 1
                elif field.kind == 0x10:
                    action = "lookup_existing" if active_kind == 0 else "create_if_missing"
                    decoded["name_action"] = action
                    decoded["active_creation_kind"] = active_kind
                    actions[action] += 1
                fields.append(decoded)
            records.append({"offset": record.offset, "size": record.size, "fields": fields})
    return {
        "scope": "Serialized fields and loader requests, not instantiated objects or resolved world transforms. Names are ASCII-only.",
        "creation_kind_field_counts": {
            str(kind): count for kind, count in sorted(creation_kinds.items())
        },
        "name_action_counts": dict(sorted(actions.items())),
        "decoded_field_counts": dict(sorted(decoded_counts.items())),
        "records": records,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("scene", type=Path)
    parser.add_argument("--output", type=Path, help="Write a new JSON file instead of stdout")
    parser.add_argument("--objects", action="store_true",
                        help="Decode supported object fields, including local asset names")
    args = parser.parse_args()
    data = args.scene.read_bytes()
    summary = summarize(data)
    if args.objects:
        summary["object_decoding"] = decode_objects(data)
    report = json.dumps(summary, indent=2, allow_nan=False) + "\n"
    if args.output is None:
        print(report, end="")
    else:
        with args.output.open("x") as output:
            output.write(report)
        print(f"Wrote structural report to {args.output}")


if __name__ == "__main__":
    main()
