"""Synthetic tests: no commercial files required."""

import io
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from extract_dta_member import (
    Member,
    decompress_lzss,
    extract_member,
    read_index,
    xor_record,
)
from inspect_scene2 import chunks, summarize


KEY = struct.pack("<II", 0x12345678, 0x87654321)


def chunk(kind: int, payload: bytes = b"") -> bytes:
    return struct.pack("<HI", kind, len(payload) + 6) + payload


def archive(payload: bytes, encrypted: bool = True) -> bytes:
    name = b"missions\\test\\scene2.bin"
    data_offset = 20 + 32 + len(name)
    block = b"\0" + payload
    stored = xor_record(block, KEY) if encrypted else block
    framing = struct.pack("<HH", len(stored), 0) + stored
    table_offset = data_offset + len(framing)
    header = struct.pack("<4I", 1, table_offset, 28, 0)
    info = struct.pack("<6I", len(framing), 32, 0, 0, len(payload), 1)
    info += bytes([len(name), 0x80 if encrypted else 0]) + bytes(6)
    table = struct.pack("<III16s", 0, 20, data_offset, b"scene2.bin")
    return (
        b"ISD0" + xor_record(header, KEY) + xor_record(info, KEY)
        + xor_record(name, KEY) + framing + xor_record(table, KEY)
    )


class CompressionTests(unittest.TestCase):
    def test_literals_and_overlap(self):
        self.assertEqual(decompress_lzss(b"\x10\x00ABC\x00\x33", 9), b"ABCABCABC")

    def test_literal_group(self):
        self.assertEqual(decompress_lzss(b"\0\0" + b"abcdefghijklmnop", 16),
                         b"abcdefghijklmnop")

    def test_run_length(self):
        self.assertEqual(decompress_lzss(b"\x80\0\0\0\x04X", 20), b"X" * 20)

    def test_invalid_tokens(self):
        for data in [b"\0", b"\x80\0\x01", b"\x80\0\0\0", b"\x80\0\x00\x10"]:
            with self.subTest(data=data), self.assertRaises(ValueError):
                decompress_lzss(data, 32)

    def test_output_limit(self):
        for data in [b"\0\0AB", b"\x80\0\0\0\x04X"]:
            with self.subTest(data=data), self.assertRaises(ValueError):
                decompress_lzss(data, 1)

    def test_xor_key_size(self):
        for key in [b"", bytes(7), bytes(9)]:
            with self.subTest(length=len(key)), self.assertRaises(ValueError):
                xor_record(b"data", key)


class ArchiveTests(unittest.TestCase):
    def test_round_trip_raw_blocks(self):
        for encrypted in [True, False]:
            stream = io.BytesIO(archive(b"synthetic data", encrypted))
            members = read_index(stream, KEY)
            self.assertEqual(len(members), 1)
            self.assertEqual(members[0].name, "missions\\test\\scene2.bin")
            self.assertEqual(extract_member(stream, members[0], KEY), b"synthetic data")

    def test_index_rejections(self):
        for data, key in [(b"ISD1" + bytes(32), KEY), (archive(b"test"), bytes(8)),
                          (archive(b"test")[:-1], KEY), (b"ISD0", KEY)]:
            with self.subTest(size=len(data)), self.assertRaises(ValueError):
                read_index(io.BytesIO(data), key)

    def test_invalid_block_sizes_and_types(self):
        for data in [struct.pack("<HH", 0, 0), struct.pack("<HH", 1, 7) + b"\0",
                     struct.pack("<HH", 1, 0) + b"\x08"]:
            with self.subTest(data=data), self.assertRaises(ValueError):
                extract_member(io.BytesIO(data), Member("test", 0, 0, 1, False), KEY)

    def test_truncated_block(self):
        with self.assertRaises(ValueError):
            extract_member(io.BytesIO(struct.pack("<HH", 3, 0) + b"\0"),
                           Member("test", 0, 2, 1, False), KEY)

    def test_member_size_mismatch(self):
        with self.assertRaises(ValueError):
            extract_member(io.BytesIO(), Member("test", 0, 2, 0, False), KEY)
        with self.assertRaises(ValueError):
            extract_member(io.BytesIO(struct.pack("<HH", 2, 0) + b"\0A"),
                           Member("test", 0, 2, 1, False), KEY)

    def test_compressed_member(self):
        block = b"\x01\x10\0ABC\0\x33"
        data = struct.pack("<HH", len(block), 0) + xor_record(block, KEY)
        self.assertEqual(
            extract_member(io.BytesIO(data), Member("test", 0, 9, 1, True), KEY),
            b"ABCABCABC",
        )

    def test_duplicate_names(self):
        data = archive(b"test")
        _, offset, _, _ = struct.unpack("<4I", xor_record(data[4:20], KEY))
        entry = xor_record(data[offset:], KEY)
        duplicate = (
            b"ISD0" + xor_record(struct.pack("<4I", 2, offset, 56, 0), KEY)
            + data[20:offset] + xor_record(entry + entry, KEY)
        )
        with self.assertRaisesRegex(ValueError, "duplicate archive member"):
            read_index(io.BytesIO(duplicate), KEY)

    def test_multiple_blocks(self):
        block = b"\0" + b"A" * 32768
        data = struct.pack("<HH", len(block), 0) + block
        data += struct.pack("<HH", 2, len(block)) + b"\0B"
        self.assertEqual(
            extract_member(io.BytesIO(data), Member("test", 0, 32769, 2, False), KEY),
            b"A" * 32768 + b"B",
        )

    def test_cli_does_not_overwrite(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "sample.dta"
            destination = Path(directory) / "output.bin"
            source.write_bytes(archive(b"test"))
            command = [
                sys.executable, "-B", str(Path(__file__).with_name("extract_dta_member.py")),
                str(source), "MISSIONS/TEST/SCENE2.BIN", str(destination),
                "--key1", "0x87654321", "--key2", "0x12345678",
            ]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(destination.read_bytes(), b"test")
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("already exists", result.stderr)
            self.assertEqual(destination.read_bytes(), b"test")


class SceneTests(unittest.TestCase):
    def test_nested_records(self):
        record = chunk(0x4010, chunk(0x10, b"test\0") + chunk(0x4011, bytes(4)))
        data = chunk(0x4C53, chunk(0x4000, record) + chunk(0xB810))
        summary = summarize(data)
        self.assertEqual(summary["object_records"], 1)
        self.assertEqual(summary["nul_terminated_name_fields"], 1)
        self.assertEqual(summary["record_field_counts"], {"0x0010": 1, "0x4011": 1})

    def test_unknown_record_is_not_guessed(self):
        summary = summarize(chunk(0x4C53, chunk(0x4000, chunk(0xFFFF, b"opaque"))))
        self.assertEqual(summary["object_records"], 0)
        self.assertEqual(summary["uninterpreted_record_kinds"], {"0xffff": 1})

    def test_invalid_chunk_bounds(self):
        for data in [b"\0", struct.pack("<HI", 1, 5), struct.pack("<HI", 1, 100)]:
            with self.subTest(data=data), self.assertRaises(ValueError):
                chunks(data, 0, len(data))
        with self.assertRaises(ValueError):
            chunks(b"", -1, 0)

    def test_invalid_root_and_trailing_bytes(self):
        for data in [b"", chunk(1), chunk(0x4C53) + chunk(1), chunk(0x4C53) + b"X"]:
            with self.subTest(data=data), self.assertRaises(ValueError):
                summarize(data)

    def test_bad_name_field(self):
        for name in [b"", b"unterminated", b"two\0names\0"]:
            data = chunk(0x4C53, chunk(0x4000, chunk(0x4010, chunk(0x10, name))))
            with self.subTest(name=name), self.assertRaises(ValueError):
                summarize(data)


if __name__ == "__main__":
    unittest.main()
