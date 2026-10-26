"""Synthetic checks for the supported model-file prefix."""

import struct
import unittest

from inspect_model_header import decode_model_header


class ModelHeaderTests(unittest.TestCase):
    def header(self, count=13, version=0x29) -> bytes:
        return b"4DS\0" + struct.pack("<H", version) + bytes(range(8)) + struct.pack("<H", count)

    def test_supported_boundaries_and_opaque_metadata(self):
        for count in [0, 1, 5999, 6000]:
            with self.subTest(count=count):
                self.assertEqual(decode_model_header(self.header(count) + b"payload"), {
                    "version": 41, "opaque_metadata_hex": "0001020304050607",
                    "first_table_entry_count": count,
                })

    def test_all_truncated_prefixes(self):
        for size in range(16):
            with self.subTest(size=size), self.assertRaisesRegex(ValueError, "Incomplete"):
                decode_model_header(self.header()[:size])

    def test_every_signature_byte(self):
        for index in range(4):
            data = bytearray(self.header())
            data[index] ^= 1
            with self.subTest(index=index), self.assertRaisesRegex(ValueError, "signature"):
                decode_model_header(bytes(data))

    def test_unsupported_versions(self):
        for version in [0, 40, 42, 0xFFFF]:
            with self.subTest(version=version), self.assertRaisesRegex(ValueError, "version"):
                decode_model_header(self.header(version=version))

    def test_exact_count_limit(self):
        for count in [6001, 32768, 65535]:
            with self.subTest(count=count), self.assertRaisesRegex(ValueError, "exceeds 6000"):
                decode_model_header(self.header(count))


if __name__ == "__main__":
    unittest.main()
