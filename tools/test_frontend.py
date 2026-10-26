"""Synthetic front-end evidence tests; no original binaries or assets required."""

import copy
import json
from pathlib import Path
import struct
import unittest
from unittest.mock import MagicMock

from test_engine_model import synthetic_pe
from validate_engine_model import PEImage
from validate_frontend import validate, validate_image


ROOT = Path(__file__).resolve().parents[1]


class FrontendEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.document = json.loads((ROOT / "analysis/frontend-map.json").read_text())
        self.data = synthetic_pe(raw_size=0x450000, rva=0x1000)
        struct.pack_into("<I", self.data, 0x98 + 28, 0x400000)
        for anchor in self.document["instruction_anchors"]:
            self.write(int(anchor["address"], 16), bytes.fromhex(anchor["hex"]))
        for table in self.document["vtables"]:
            for index, target in enumerate(table["targets"]):
                self.write(int(table["address"], 16) + index * 4,
                           struct.pack("<I", int(target, 16)))
        for call in self.document["direct_calls"]:
            site = int(call["site"], 16)
            self.write(site, b"\xe8" + struct.pack("<i", int(call["target"], 16) - site - 5))
        for anchor in self.document["resource_anchors"]:
            address = int(anchor["address"], 16)
            self.write(int(anchor["site"], 16), b"\x68" + struct.pack("<I", address))
            self.write(address, anchor["text"].encode("ascii") + b"\0")

    def write(self, address, data):
        offset = 0x400 + address - 0x401000
        if offset < 0x400 or offset + len(data) > len(self.data):
            raise ValueError("Front-end fixture exceeds synthetic section")
        self.data[offset:offset + len(data)] = data

    def test_complete_evidence(self):
        self.assertEqual(validate_image(PEImage(bytes(self.data)), self.document), {
            "instructions": 12, "vtable_slots": 25, "calls": 10, "resources": 14,
        })

    def test_each_instruction_anchor_is_checked(self):
        for anchor in self.document["instruction_anchors"]:
            with self.subTest(address=anchor["address"]):
                changed = bytearray(self.data)
                changed[0x400 + int(anchor["address"], 16) - 0x401000] ^= 1
                with self.assertRaisesRegex(ValueError, "instruction"):
                    validate_image(PEImage(bytes(changed)), self.document)

    def test_every_vtable_slot_is_checked(self):
        for table in self.document["vtables"]:
            for index in range(len(table["targets"])):
                with self.subTest(table=table["address"], slot=index):
                    changed = bytearray(self.data)
                    changed[0x400 + int(table["address"], 16) + index * 4 - 0x401000] ^= 1
                    with self.assertRaisesRegex(ValueError, "vtable"):
                        validate_image(PEImage(bytes(changed)), self.document)

    def test_each_call_opcode_and_target_are_checked(self):
        for call in self.document["direct_calls"]:
            for delta in (0, 1):
                with self.subTest(site=call["site"], delta=delta):
                    changed = bytearray(self.data)
                    changed[0x400 + int(call["site"], 16) + delta - 0x401000] ^= 1
                    with self.assertRaises(ValueError):
                        validate_image(PEImage(bytes(changed)), self.document)

    def test_every_resource_reference_text_and_terminator_are_checked(self):
        for anchor in self.document["resource_anchors"]:
            for field, delta in (("site", 0), ("site", 1), ("address", 0),
                                 ("address", len(anchor["text"]))):
                with self.subTest(site=anchor["site"], field=field, delta=delta):
                    changed = bytearray(self.data)
                    changed[0x400 + int(anchor[field], 16) + delta - 0x401000] ^= 1
                    with self.assertRaises(ValueError):
                        validate_image(PEImage(bytes(changed)), self.document)

    def test_duplicate_or_empty_vtable_rejected(self):
        for tables in ([self.document["vtables"][0]] * 2,
                       [{"address": "00815e30", "targets": []}]):
            document = copy.deepcopy(self.document)
            document["vtables"] = tables
            with self.subTest(tables=tables), self.assertRaisesRegex(ValueError, "vtable"):
                validate_image(PEImage(bytes(self.data)), document)

    def test_wrong_image_base_rejected(self):
        self.document["binary"]["image_base"] = "00500000"
        with self.assertRaisesRegex(ValueError, "image-base"):
            validate_image(PEImage(bytes(self.data)), self.document)

    def test_unsupported_fingerprint_rejected_before_file_access(self):
        self.document["binary"]["sha256"] = "0" * 64
        root = MagicMock()
        with self.assertRaisesRegex(ValueError, "Unsupported"):
            validate(self.document, root)
        root.__truediv__.assert_not_called()

    def test_wrong_binary_rejected(self):
        root = MagicMock()
        root.__truediv__.return_value.read_bytes.return_value = bytes(self.data)
        with self.assertRaisesRegex(ValueError, "fingerprint mismatch"):
            validate(self.document, root)


if __name__ == "__main__":
    unittest.main()
