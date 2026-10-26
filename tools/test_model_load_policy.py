"""Synthetic checks for the bounded model-result evidence validator."""

from copy import deepcopy
from pathlib import Path
import struct
import unittest
from unittest.mock import patch

from validate_model_load_policy import CLIENT_SHA256, validate, validate_image


class SparseImage:
    def __init__(self, blocks: dict[int, bytes]):
        self.blocks = blocks

    def read_va(self, address: int, size: int) -> bytes:
        data = self.blocks[address]
        if len(data) < size:
            raise ValueError("Synthetic read exceeds block")
        return data[:size]


class ModelLoadEvidenceTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = {
            "instruction_anchors": [{"address": "1010", "hex": "85c07d18"}],
            "dispatch": {
                "selector_address": "1020",
                "target_address": "1040",
                "selectors": [0] * 20,
                "targets": ["1010", "1020", "1030", "1040", "1050"],
            },
            "diagnostics": [{
                "site": "1080", "string_address": "1090", "text": "synthetic diagnostic",
            }],
        }
        self.blocks = {
            0x1010: bytes.fromhex("85c07d18"),
            0x1020: bytes(20),
            0x1040: struct.pack("<5I", 0x1010, 0x1020, 0x1030, 0x1040, 0x1050),
            0x1080: b"\x68" + struct.pack("<I", 0x1090),
            0x1090: b"synthetic diagnostic\0",
        }

    def test_matching_evidence(self) -> None:
        validate_image(SparseImage(self.blocks), self.document)

    def test_mutated_instruction(self) -> None:
        self.blocks[0x1010] = bytes(4)
        with self.assertRaisesRegex(ValueError, "instruction mismatch"):
            validate_image(SparseImage(self.blocks), self.document)

    def test_mutated_selector(self) -> None:
        self.blocks[0x1020] = b"\x01" + bytes(19)
        with self.assertRaisesRegex(ValueError, "selector table mismatch"):
            validate_image(SparseImage(self.blocks), self.document)

    def test_mutated_target(self) -> None:
        self.blocks[0x1040] = bytes(20)
        with self.assertRaisesRegex(ValueError, "target table mismatch"):
            validate_image(SparseImage(self.blocks), self.document)

    def test_out_of_range_selector(self) -> None:
        document = deepcopy(self.document)
        document["dispatch"]["selectors"][0] = 5
        self.blocks[0x1020] = b"\x05" + bytes(19)
        with self.assertRaisesRegex(ValueError, "selector exceeds"):
            validate_image(SparseImage(self.blocks), document)

    def test_mutated_diagnostic_reference(self) -> None:
        self.blocks[0x1080] = bytes(5)
        with self.assertRaisesRegex(ValueError, "reference mismatch"):
            validate_image(SparseImage(self.blocks), self.document)

    def test_mutated_diagnostic_text(self) -> None:
        self.blocks[0x1090] = b"x" + self.blocks[0x1090][1:]
        with self.assertRaisesRegex(ValueError, "text mismatch"):
            validate_image(SparseImage(self.blocks), self.document)

    def test_wrong_fingerprint_rejected_before_pe_parse(self) -> None:
        document = {"binary": {"path": "synthetic.exe",
                               "sha256": CLIENT_SHA256}}
        with patch.object(Path, "read_bytes", return_value=b"different"):
            with self.assertRaisesRegex(ValueError, "fingerprint mismatch"):
                validate(document, Path("/synthetic"))

    def test_unsupported_map_fingerprint(self) -> None:
        document = {"binary": {"path": "synthetic.exe", "sha256": "0" * 64}}
        with self.assertRaisesRegex(ValueError, "Unsupported client fingerprint"):
            validate(document, Path("/synthetic"))


if __name__ == "__main__":
    unittest.main()
