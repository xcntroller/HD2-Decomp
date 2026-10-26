"""Synthetic PE and evidence rejection tests; no commercial binary required."""

import copy
import json
from pathlib import Path
import struct
import unittest
from unittest.mock import Mock

from validate_engine_model import ENGINE_SHA256, PEImage, address, validate, validate_evidence


ROOT = Path(__file__).resolve().parents[1]


def synthetic_pe(raw_size=64, virtual_size=None, rva=0x9000) -> bytearray:
    data = bytearray(0x400 + raw_size)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HH", data, 0x84, 0x14C, 1)
    struct.pack_into("<H", data, 0x94, 0xE0)
    struct.pack_into("<H", data, 0x98, 0x10B)
    struct.pack_into("<I", data, 0x98 + 28, 0x10000000)
    section = 0x178
    data[section:section + 8] = b".sample\0"
    struct.pack_into("<4I", data, section + 8,
                     raw_size if virtual_size is None else virtual_size, rva, raw_size, 0x400)
    return data


class PEImageTests(unittest.TestCase):
    def test_section_translation_not_va_minus_base(self):
        data = synthetic_pe()
        data[0x400:0x404] = b"test"
        image = PEImage(bytes(data))
        self.assertEqual(image.read_va(0x10009000, 4), b"test")
        self.assertEqual(image.read_va(0x1000903F, 1), b"\0")
        for va, size in [(0x10000000, 1), (0x10008FFF, 1), (0x1000903F, 2),
                         (0x10009040, 1), (0x0FFFFFFF, 1), (0x10009000, 0),
                         (0x10009000, -1), (0xFFFFFFFF, 2)]:
            with self.subTest(va=va, size=size), self.assertRaises(ValueError):
                image.read_va(va, size)

    def test_virtual_tail_and_raw_padding_are_not_readable(self):
        for virtual_size, va in [(128, 0x10009040), (32, 0x10009020)]:
            image = PEImage(bytes(synthetic_pe(virtual_size=virtual_size)))
            with self.subTest(virtual_size=virtual_size), self.assertRaises(ValueError):
                image.read_va(va, 1)

    def test_truncated_headers_and_sections(self):
        data = synthetic_pe()
        for size in [0, 1, 0x3F, 0x83, 0x97, 0x100, 0x19F, len(data) - 1]:
            with self.subTest(size=size), self.assertRaises(ValueError):
                PEImage(bytes(data[:size]))

    def test_invalid_header_identity_and_section_range(self):
        for offset, value in [(0, b"NO"), (0x80, b"BAD!"), (0x84, b"\x64\x86"),
                              (0x86, b"\0\0"), (0x94, b"\x01\0"),
                              (0x98, b"\x0b\x02"), (0x3C, b"\xff" * 4),
                              (0x184, b"\xf0\xff\xff\xff")]:
            data = synthetic_pe()
            data[offset:offset + len(value)] = value
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                PEImage(bytes(data))

    def test_overlapping_sections_rejected(self):
        data = synthetic_pe()
        struct.pack_into("<H", data, 0x86, 2)
        data[0x1A0:0x1C8] = data[0x178:0x1A0]
        with self.assertRaisesRegex(ValueError, "Overlapping"):
            PEImage(bytes(data))


class EngineEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.document = json.loads((ROOT / "analysis/engine-model-map.json").read_text())
        self.data = synthetic_pe(raw_size=0xA7000, rva=0x1000)
        for check in self.document["byte_checks"]:
            self.write(address(check["va"]), bytes.fromhex(check["hex"]))
        for pointer in self.document["pointers"]:
            self.write(address(pointer["va"]), struct.pack("<I", address(pointer["target"])))
        for call in self.document["direct_calls"]:
            site = address(call["site"])
            self.write(site, b"\xe8" + struct.pack("<i", address(call["target"]) - site - 5))
        table = address(self.document["type_query"]["jump_table"])
        for item in self.document["frame_kinds"]:
            case, string = address(item["case_va"]), address(item["string_va"])
            self.write(table + 4 * (item["kind"] - 1), struct.pack("<I", case))
            self.write(case, b"\xb8" + struct.pack("<I", string) + b"\xc2\x08\x00")
            self.write(string, item["name"].encode("ascii") + b"\0")

    def write(self, va, value):
        offset = 0x400 + va - 0x10001000
        if offset < 0x400 or offset + len(value) > len(self.data):
            raise ValueError(f"Evidence address exceeds synthetic section: {va:#x}")
        self.data[offset:offset + len(value)] = value

    def test_complete_synthetic_evidence(self):
        counts = validate_evidence(self.document, PEImage(bytes(self.data)))
        self.assertEqual(counts, {"byte_checks": 14, "pointers": 5,
                                  "direct_calls": 3, "frame_kinds": 16})

    def test_instruction_pointer_call_table_case_and_string_mismatches(self):
        for va, message in [
            (0x10068FB0, "Instruction bytes"), (0x1009B694, "Pointer"),
            (0x10019E19, "Direct-call opcode"), (0x10019E1A, "Direct-call target"),
            (0x10019FB4, "jump table"), (0x10019F17, "Type-query case"),
            (0x100A660C, "Frame-kind string"), (0x100A6618, "Frame-kind string"),
        ]:
            data = bytearray(self.data)
            data[0x400 + va - 0x10001000] ^= 1
            with self.subTest(va=va), self.assertRaisesRegex(ValueError, message):
                validate_evidence(self.document, PEImage(bytes(data)))

    def test_wrong_base_and_incomplete_kind_mapping(self):
        document = copy.deepcopy(self.document)
        document["binary"]["image_base"] = "20000000"
        with self.assertRaisesRegex(ValueError, "image-base"):
            validate_evidence(document, PEImage(bytes(self.data)))
        for replacement in [[], self.document["frame_kinds"][:-1],
                            self.document["frame_kinds"][::-1]]:
            document = copy.deepcopy(self.document)
            document["frame_kinds"] = replacement
            with self.assertRaisesRegex(ValueError, "exactly frame kinds"):
                validate_evidence(document, PEImage(bytes(self.data)))

    def test_fingerprint_rejection_precedes_pe_parsing(self):
        root = Mock()
        root.__truediv__ = Mock(return_value=Mock())
        (root / "unused").read_bytes.return_value = b"not even a PE"
        self.assertEqual(self.document["binary"]["sha256"], ENGINE_SHA256)
        with self.assertRaisesRegex(ValueError, "binary fingerprint mismatch"):
            validate(self.document, root)
        document = copy.deepcopy(self.document)
        document["binary"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "unsupported engine fingerprint"):
            validate(document, root)


if __name__ == "__main__":
    unittest.main()
