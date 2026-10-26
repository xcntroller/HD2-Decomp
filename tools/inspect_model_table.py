"""Inspect first-table record boundaries, keeping payloads and name bytes uninterpreted."""

import argparse
from collections import Counter
from dataclasses import dataclass
import json
from pathlib import Path
import struct

from inspect_model_header import decode_model_header


@dataclass(frozen=True)
class ModelTableRecord:
    offset: int
    end: int
    flags: int
    payload: bytes
    extra_name: bytes | None
    primary_name: bytes
    secondary_name: bytes | None
    sequence_counts: bytes | None
    sequence_parameters: bytes | None


def decode_model_table(data: bytes) -> tuple[dict, list[ModelTableRecord], int]:
    header = decode_model_header(data)
    position = 16

    def take(size: int) -> bytes:
        nonlocal position
        if size > len(data) - position:
            raise ValueError(f"Incomplete model first-table field at {position:#x}")
        result = data[position:position + size]
        position += size
        return result

    def name() -> bytes:
        return take(take(1)[0])

    records = []
    for _ in range(header["first_table_entry_count"]):
        offset = position
        flags = struct.unpack("<I", take(4))[0]
        extended = bool(flags & 0x00080000)
        payload = take(76 if extended else 72)
        extra_name = name() if extended else None
        primary_name = name()
        secondary_name = None
        if primary_name and flags & 0x40000000 and not flags & 0xA1000000:
            secondary_name = name()
        counts = parameters = None
        if flags & 0x04000000:
            counts, parameters = take(2), take(16)
        records.append(ModelTableRecord(offset, position, flags, payload, extra_name,
                                        primary_name, secondary_name, counts, parameters))
    return header, records, position


def summarize_model_table(data: bytes, *, include_records: bool = False) -> dict:
    header, records, end = decode_model_table(data)
    names = [
        name for record in records
        for name in (record.extra_name, record.primary_name, record.secondary_name)
        if name is not None
    ]
    summary = {
        "header": header,
        "records": len(records),
        "table_end": end,
        "plain_records": sum(record.sequence_counts is None for record in records),
        "sequence_records": sum(record.sequence_counts is not None for record in records),
        "extra_name_fields": sum(record.extra_name is not None for record in records),
        "secondary_name_fields": sum(record.secondary_name is not None for record in records),
        "max_name_length": max(map(len, names), default=0),
        "flags": {f"0x{flag:08x}": count for flag, count in sorted(Counter(
            record.flags for record in records
        ).items())},
    }
    if include_records:
        summary["record_data"] = [
            {
                "offset": record.offset, "end": record.end, "flags": record.flags,
                "payload_hex": record.payload.hex(),
                "extra_name_hex": None if record.extra_name is None else record.extra_name.hex(),
                "primary_name_hex": record.primary_name.hex(),
                "secondary_name_hex": (None if record.secondary_name is None
                                       else record.secondary_name.hex()),
                "sequence": (None if record.sequence_counts is None else {
                    "counts": list(record.sequence_counts),
                    "parameters_hex": record.sequence_parameters.hex(),
                }),
            } for record in records
        ]
    return summary


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("model", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    summary = summarize_model_table(args.model.read_bytes())
    text = json.dumps(summary, indent=2) + "\n"
    if args.output is None:
        print(text, end="")
    else:
        with args.output.open("x") as stream:
            stream.write(text)
        print(f"Wrote model-table boundary report to {args.output}")


if __name__ == "__main__":
    main()
