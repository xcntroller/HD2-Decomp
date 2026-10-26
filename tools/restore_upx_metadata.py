"""Restore UPX identifiers on a separate copy; never modify the input file."""

import argparse
from pathlib import Path
import struct


def restore(source: Path, destination: Path) -> None:
    data = bytearray(source.read_bytes())
    if data[:2] != b"MZ" or len(data) < 64:
        raise ValueError("Input is not a DOS/PE executable")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("Missing PE signature")
    machine, count = struct.unpack_from("<HH", data, pe + 4)
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    optional = pe + 24
    if machine != 0x14C or count != 3 or optional_size != 224:
        raise ValueError("Expected a three-section x86 PE32 executable")
    if struct.unpack_from("<H", data, optional)[0] != 0x10B:
        raise ValueError("Expected a PE32 optional header")
    sections = optional + optional_size
    if sections + count * 40 > len(data):
        raise ValueError("Truncated section table")
    if data[sections:sections + 8] != bytes(8):
        raise ValueError("First section name is not blank")
    if data[sections + 40:sections + 48] != bytes(8):
        raise ValueError("Second section name is not blank")
    raw_size = struct.unpack_from("<I", data, sections + 16)[0]
    packed_offset = struct.unpack_from("<I", data, sections + 60)[0]
    if raw_size != 0 or not sections + count * 40 + 32 <= packed_offset < len(data):
        raise ValueError("Unexpected packed section layout")
    header = packed_offset - 32
    if data[header:header + 8] != b"\0\0\0\0\x0d\x09\x08\x07":
        raise ValueError("Expected erased marker and UPX version/format/method/level")
    data[sections:sections + 8] = b"UPX0\0\0\0\0"
    data[sections + 40:sections + 48] = b"UPX1\0\0\0\0"
    data[header:header + 4] = b"UPX!"
    with destination.open("xb") as output:
        output.write(data)
    print(f"Restored two section names and UPX marker at {header:#x}: {destination}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    restore(args.source, args.destination)
