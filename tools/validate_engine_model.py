"""Verify bounded engine evidence against a fingerprint-guarded, section-mapped PE32."""

import argparse
import hashlib
import json
from pathlib import Path
import struct


ENGINE_SHA256 = "12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee"


class PEImage:
    """Translate only file-backed section bytes, never VA minus image base alone."""

    def __init__(self, data: bytes):
        self.data = data
        if self.read_file(0, 2) != b"MZ":
            raise ValueError("Missing DOS MZ signature")
        pe = self.unpack_file("<I", 0x3C)[0]
        if self.read_file(pe, 4) != b"PE\0\0":
            raise ValueError("Missing PE signature")
        machine, count = self.unpack_file("<HH", pe + 4)
        optional_size = self.unpack_file("<H", pe + 20)[0]
        if machine != 0x14C or not count or optional_size < 96:
            raise ValueError("Expected an i386 PE32 with sections and a full optional header")
        optional = pe + 24
        self.read_file(optional, optional_size)
        if self.unpack_file("<H", optional)[0] != 0x10B:
            raise ValueError("Expected PE32 optional header")
        self.image_base = self.unpack_file("<I", optional + 28)[0]
        self.sections = []
        for index in range(count):
            header = optional + optional_size + 40 * index
            self.read_file(header, 40)
            virtual_size, rva, raw_size, raw = self.unpack_file("<4I", header + 8)
            extent = virtual_size or raw_size
            if rva + extent > 0x100000000:
                raise ValueError("Section virtual range overflows PE32")
            self.read_file(raw, raw_size)
            if any(rva < previous + size and previous < rva + extent
                   for previous, size, _, _ in self.sections):
                raise ValueError("Overlapping PE virtual sections")
            self.sections.append((rva, extent, raw, raw_size))

    def read_file(self, offset: int, size: int) -> bytes:
        if offset < 0 or size < 0 or offset > len(self.data) - size:
            raise ValueError(f"Truncated PE file range at {offset:#x}, size {size:#x}")
        return self.data[offset:offset + size]

    def unpack_file(self, format_: str, offset: int) -> tuple:
        return struct.unpack(format_, self.read_file(offset, struct.calcsize(format_)))

    def read_va(self, va: int, size: int) -> bytes:
        if not 0 <= va <= 0xFFFFFFFF or size <= 0 or va + size > 0x100000000:
            raise ValueError("Invalid PE32 VA range")
        rva = va - self.image_base
        for start, extent, raw, raw_size in self.sections:
            delta = rva - start
            if 0 <= delta < extent and delta + size <= min(extent, raw_size):
                return self.read_file(raw + delta, size)
        raise ValueError(f"VA {va:#x}, size {size:#x} is not in a file-backed section")

    def expect(self, va: int, expected: bytes, label: str) -> None:
        if self.read_va(va, len(expected)) != expected:
            raise ValueError(f"{label} mismatch at VA {va:#x}")


def address(value: str) -> int:
    return int(value, 16)


def validate_evidence(document: dict, image: PEImage) -> dict:
    if image.image_base != address(document["binary"]["image_base"]):
        raise ValueError("PE image-base mismatch")
    for check in document["byte_checks"]:
        image.expect(address(check["va"]), bytes.fromhex(check["hex"]), "Instruction bytes")
    for pointer in document["pointers"]:
        image.expect(address(pointer["va"]), struct.pack("<I", address(pointer["target"])),
                     "Pointer")
    for call in document["direct_calls"]:
        site = address(call["site"])
        instruction = image.read_va(site, 5)
        if instruction[0] != 0xE8:
            raise ValueError(f"Direct-call opcode mismatch at VA {site:#x}")
        target = site + 5 + struct.unpack("<i", instruction[1:])[0]
        if target != address(call["target"]):
            raise ValueError(f"Direct-call target mismatch at VA {site:#x}")
    kinds = document["frame_kinds"]
    if [item["kind"] for item in kinds] != list(range(1, 17)):
        raise ValueError("Expected exactly frame kinds 1 through 16 in order")
    table = address(document["type_query"]["jump_table"])
    for item in kinds:
        case = address(item["case_va"])
        string = address(item["string_va"])
        image.expect(table + (item["kind"] - 1) * 4, struct.pack("<I", case),
                     "Type-query jump table")
        image.expect(case, b"\xb8" + struct.pack("<I", string) + b"\xc2\x08\x00",
                     "Type-query case")
        image.expect(string, item["name"].encode("ascii") + b"\0", "Frame-kind string")
    return {
        "byte_checks": len(document["byte_checks"]),
        "pointers": len(document["pointers"]),
        "direct_calls": len(document["direct_calls"]),
        "frame_kinds": len(kinds),
    }


def validate(document: dict, root: Path) -> dict:
    if document["binary"]["sha256"] != ENGINE_SHA256:
        raise ValueError("Evidence map has an unsupported engine fingerprint")
    data = (root / document["binary"]["path"]).read_bytes()
    if hashlib.sha256(data).hexdigest() != ENGINE_SHA256:
        raise ValueError("Engine binary fingerprint mismatch")
    return validate_evidence(document, PEImage(data))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("map", type=Path, nargs="?",
                        default=Path(__file__).resolve().parents[1] / "analysis/engine-model-map.json")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    try:
        counts = validate(json.loads(args.map.read_text()), args.root)
    except (ValueError, KeyError, OSError, struct.error) as error:
        parser.exit(1, f"Engine evidence validation failed: {error}\n")
    print(f"Verified engine fingerprint; {counts['byte_checks']} instruction anchors, "
          f"{counts['pointers']} pointers, {counts['direct_calls']} direct calls, "
          f"and {counts['frame_kinds']} frame-kind table/case/string mappings.")


if __name__ == "__main__":
    main()
