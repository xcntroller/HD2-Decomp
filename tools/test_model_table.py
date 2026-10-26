"""Synthetic first-table layouts; arbitrary bytes are not treated as validated asset names."""

import struct
import unittest

from inspect_model_table import decode_model_table, summarize_model_table


LAYOUT_BITS = [0x00080000, 0x04000000, 0x40000000, 0x80000000, 0x20000000, 0x01000000]


def record(flags: int, primary=b"primary", extra=b"", secondary=b"") -> bytes:
    def name(value):
        if len(value) > 255:
            raise ValueError("Synthetic name length exceeds 255")
        return bytes([len(value)]) + value

    result = struct.pack("<I", flags) + bytes(range(76 if flags & 0x80000 else 72))
    if flags & 0x80000:
        result += name(extra)
    result += name(primary)
    if primary and flags & 0x40000000 and not flags & 0xA1000000:
        result += name(secondary)
    if flags & 0x4000000:
        result += bytes([254, 129]) + bytes(range(16))
    return result


def model(records: list[bytes]) -> bytes:
    return b"4DS\0" + struct.pack("<H", 41) + bytes(8) + struct.pack("<H", len(records)) + b"".join(records)


class ModelTableTests(unittest.TestCase):
    def test_every_layout_flag_and_primary_presence(self):
        for mask in range(64):
            flags = 0x00800001 | sum(bit for index, bit in enumerate(LAYOUT_BITS) if mask & (1 << index))
            for primary in [b"", b"\0\xffA"]:
                data = model([record(flags, primary)]) + b"unread"
                _, records, end = decode_model_table(data)
                row = records[0]
                with self.subTest(mask=mask, primary_present=bool(primary)):
                    self.assertEqual(row.flags, flags)
                    self.assertEqual(row.payload, bytes(range(76 if flags & 0x80000 else 72)))
                    self.assertEqual(row.extra_name, b"" if flags & 0x80000 else None)
                    self.assertEqual(row.primary_name, primary)
                    self.assertEqual(row.secondary_name, b"" if (
                        primary and flags & 0x40000000 and not flags & 0xA1000000
                    ) else None)
                    self.assertEqual(row.sequence_counts, b"\xfe\x81" if flags & 0x4000000 else None)
                    self.assertEqual(row.sequence_parameters, bytes(range(16)) if flags & 0x4000000 else None)
                    self.assertEqual(data[end:], b"unread")

    def test_maximum_name_lengths(self):
        name = bytes(range(255))
        _, records, _ = decode_model_table(model([record(0x44080000, name, name, name)]))
        self.assertEqual((records[0].extra_name, records[0].primary_name,
                          records[0].secondary_name), (name, name, name))

    def test_all_truncations_and_record_count_overruns(self):
        data = model([record(0x44080000, bytes(255), bytes(255), bytes(255))])
        for size in range(len(data)):
            with self.subTest(size=size), self.assertRaises(ValueError):
                decode_model_table(data[:size])
        data = bytearray(model([record(0)]))
        struct.pack_into("<H", data, 14, 2)
        with self.assertRaisesRegex(ValueError, "Incomplete"):
            decode_model_table(bytes(data))

    def test_zero_and_limit_count(self):
        self.assertEqual(decode_model_table(model([]) + b"tail")[2], 16)
        self.assertEqual(len(decode_model_table(model([record(0, b"")] * 6000))[1]), 6000)
        data = bytearray(model([]))
        struct.pack_into("<H", data, 14, 6001)
        with self.assertRaisesRegex(ValueError, "6000"):
            decode_model_table(bytes(data))

    def test_runtime_flag_update_does_not_change_serialized_name_gate(self):
        def gate(flags):
            return bool(flags & 0x40000000 and not flags & 0xA1000000)

        for mask in range(64):
            flags = sum(bit for index, bit in enumerate(LAYOUT_BITS) if mask & (1 << index))
            for feature in [False, True]:
                for texture_bit in [0, 0x00800000]:
                    original = flags | texture_bit
                    mode_100 = feature and bool(original & 0x00800000)
                    mode_8 = bool(original & 0x20000000)
                    updated = original | (0x40000000 if mode_100 and mode_8 else 0)
                    self.assertEqual(gate(original), gate(updated))

    def test_summary_keeps_names_local_and_distinguishes_empty_fields(self):
        data = model([record(0x44080000, b"synthetic", b"", b"")]) + b"tail"
        summary = summarize_model_table(data)
        self.assertNotIn("record_data", summary)
        self.assertEqual(summary["sequence_records"], 1)
        self.assertEqual(summary["plain_records"], 0)
        self.assertEqual(summary["extra_name_fields"], 1)
        self.assertEqual(summary["secondary_name_fields"], 1)
        self.assertEqual(summary["table_end"], len(data) - 4)
        detail = summarize_model_table(data, include_records=True)["record_data"][0]
        self.assertEqual(detail["extra_name_hex"], "")
        self.assertEqual(detail["sequence"]["counts"], [254, 129])


if __name__ == "__main__":
    unittest.main()
