"""Synthetic tests: no commercial files required."""

import io
import copy
import hashlib
import json
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
from inspect_scene2 import chunks, decode_objects, frame_kind_name, model_resource_key, summarize
from validate_mission_sample import validate as validate_mission


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


class ObjectDecodingTests(unittest.TestCase):
    def scene(self, fields: bytes) -> bytes:
        return chunk(0x4C53, chunk(0x4000, chunk(0x4010, fields)))

    def test_frame_kind_labels_match_verified_manifest(self):
        path = Path(__file__).resolve().parents[1] / "analysis/engine-model-map.json"
        kinds = json.loads(path.read_text())["frame_kinds"]
        self.assertEqual([item["kind"] for item in kinds], list(range(1, 17)))
        for item in kinds:
            with self.subTest(kind=item["kind"]):
                self.assertEqual(frame_kind_name(item["kind"]), item["name"])
        for value in [0, 17, 0xFFFFFFFF]:
            with self.subTest(unknown=value):
                self.assertIsNone(frame_kind_name(value))

    def test_labels_preserve_numeric_fields_and_unknown_kinds(self):
        values = list(range(1, 17)) + [0, 17, 0xFFFFFFFF]
        fields = b"".join(
            chunk(0x4011, struct.pack("<I", value))
            + chunk(0x10, b"synthetic\0") + chunk(0x2012, b"synthetic.4ds\0")
            for value in values
        )
        result = decode_objects(self.scene(fields))
        decoded = result["records"][0]["fields"]
        self.assertEqual(result["creation_kind_field_counts"],
                         {str(value): 1 for value in sorted(values)})
        expected = []
        for index, value in enumerate(values):
            with self.subTest(value=value):
                creation, name, resource = decoded[index * 3:index * 3 + 3]
                self.assertEqual(creation["value"], value)
                self.assertEqual(creation["label"], frame_kind_name(value))
                for field in [name, resource]:
                    self.assertEqual(field["active_creation_kind"], value)
                    self.assertEqual(field["active_creation_kind_label"], frame_kind_name(value))
                self.assertEqual(resource["required_frame_kind"], 9)
                self.assertEqual(resource["required_frame_kind_label"], "FRAME_MODEL")
                self.assertEqual(name["name_action"],
                                 "lookup_existing" if value == 0 else "create_if_missing")
            expected.append({"kind": value, "label": frame_kind_name(value), "count": 1})
        self.assertEqual(result["serialized_creation_kind_field_counts_labeled"],
                         sorted(expected, key=lambda item: item["kind"]))
        self.assertIn('"label": null', json.dumps(result))
        self.assertNotIn("runtime_instance_counts", result)

    def test_decodes_supported_fields(self):
        fields = (
            chunk(0x4011, struct.pack("<I", 6))
            + chunk(0x10, b"synthetic\0")
            + chunk(0x20, struct.pack("<3f", 1, -2, 3))
            + chunk(0x22, struct.pack("<4f", 1, 0, 0, 0))
            + chunk(0x2C, struct.pack("<3f", 4, 5, 6))
            + chunk(0x2D, struct.pack("<3f", 2, 3, 4))
            + chunk(0x4020, chunk(0x10, b"parent\0"))
        )
        result = decode_objects(self.scene(fields))
        decoded = result["records"][0]["fields"]
        self.assertEqual(result["creation_kind_field_counts"], {"6": 1})
        self.assertEqual(result["name_action_counts"], {"create_if_missing": 1})
        self.assertEqual(decoded[1]["active_creation_kind"], 6)
        self.assertEqual(decoded[2]["value"], [1, -2, 3])
        self.assertEqual(decoded[3]["value"], [1, 0, 0, 0])
        self.assertEqual(decoded[3]["engine_xyzw_before_normalization"], [0, 0, 0, 1])
        self.assertEqual(decoded[4]["value"], [4, 5, 6])
        self.assertEqual(decoded[5]["value"], [2, 3, 4])
        self.assertEqual(decoded[6]["children"][0]["name"], "parent")

    def test_name_actions_follow_field_order(self):
        data = self.scene(
            chunk(0x10, b"before\0")
            + chunk(0x4011, struct.pack("<I", 9))
            + chunk(0x10, b"after\0")
            + chunk(0x4011, bytes(4))
            + chunk(0x10, b"zero\0")
        )
        result = decode_objects(data)
        names = [field for field in result["records"][0]["fields"] if field["id"] == "0x0010"]
        self.assertEqual([field["active_creation_kind"] for field in names], [0, 9, 0])
        self.assertEqual(result["name_action_counts"], {"lookup_existing": 2, "create_if_missing": 1})

    def test_record_resets_creation_kind(self):
        first = chunk(0x4010, chunk(0x4011, struct.pack("<I", 2)) + chunk(0x10, b"first\0"))
        second = chunk(0x4010, chunk(0x10, b"second\0"))
        result = decode_objects(chunk(0x4C53, chunk(0x4000, first + second)))
        self.assertEqual(result["name_action_counts"], {"lookup_existing": 1, "create_if_missing": 1})

    def test_opaque_fields_and_missing_values_are_preserved(self):
        result = decode_objects(self.scene(chunk(0xABCD, b"not a float")))
        field = result["records"][0]["fields"][0]
        self.assertEqual(field["id"], "0xabcd")
        self.assertEqual(field["payload_size"], 11)
        self.assertNotIn("value", field)
        self.assertEqual(result["decoded_field_counts"], {})
        self.assertEqual(result["name_action_counts"], {})

    def test_wrong_fixed_widths(self):
        for kind, size in [(0x4011, 4), (0x20, 12), (0x22, 16), (0x2C, 12), (0x2D, 12)]:
            for wrong_size in [size - 1, size + 1]:
                with self.subTest(kind=kind, size=wrong_size), self.assertRaises(ValueError):
                    decode_objects(self.scene(chunk(kind, bytes(wrong_size))))

    def test_nonfinite_vectors(self):
        for value in [float("nan"), float("inf"), float("-inf")]:
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, "Nonfinite"):
                decode_objects(self.scene(chunk(0x20, struct.pack("<3f", value, 0, 0))))

    def test_rotation_order_and_no_silent_normalization(self):
        result = decode_objects(self.scene(chunk(0x22, struct.pack("<4f", 2, 3, 4, 5))))
        field = result["records"][0]["fields"][0]
        self.assertEqual(field["value"], [2, 3, 4, 5])
        self.assertEqual(field["engine_xyzw_before_normalization"], [3, 4, 5, 2])

    def test_parent_boundaries_and_unknown_children(self):
        result = decode_objects(self.scene(chunk(0x4020, chunk(0x1234, b"opaque"))))
        child = result["records"][0]["fields"][0]["children"][0]
        self.assertEqual(child["id"], "0x1234")
        self.assertNotIn("name", child)
        with self.assertRaises(ValueError):
            decode_objects(self.scene(chunk(0x4020, b"broken")))

    def test_rejects_invalid_and_unsupported_names(self):
        for name in [b"", b"bad", b"a\0b\0", b"\xff\0"]:
            with self.subTest(name=name), self.assertRaises(ValueError):
                decode_objects(self.scene(chunk(0x10, name)))

    def test_cli_objects_is_opt_in_and_no_overwrite(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "scene2.bin"
            output = Path(directory) / "decoded.json"
            source.write_bytes(self.scene(chunk(0x10, b"synthetic\0")))
            command = [
                sys.executable, "-B", str(Path(__file__).with_name("inspect_scene2.py")),
                str(source),
            ]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertNotIn("object_decoding", result.stdout)
            result = subprocess.run(command + ["--objects", "--output", str(output)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            original = output.read_bytes()
            self.assertIn(b'"object_decoding"', original)
            result = subprocess.run(command + ["--objects", "--output", str(output)],
                                    capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(output.read_bytes(), original)

    def test_model_resource_key_last_dot_rule(self):
        cases = {
            "": "", "crate": "crate", "crate.4ds": "crate",
            "crate.variant.4ds": "crate.variant", ".hidden": "",
            "folder.name/object": "folder", "folder\\thing.4ds": "folder\\thing",
            "MiXeD.4DS": "MiXeD", "name.": "name", " a .b": " a ",
            ".": "", "..": ".",
        }
        for name, expected in cases.items():
            with self.subTest(name=name):
                self.assertEqual(model_resource_key(name), expected)
        with self.assertRaisesRegex(ValueError, "embedded NUL"):
            model_resource_key("before\0after")

    def test_model_resource_field(self):
        data = self.scene(chunk(0x4011, struct.pack("<I", 9))
                          + chunk(0x2012, b"synthetic.variant.4ds\0"))
        field = decode_objects(data)["records"][0]["fields"][1]
        self.assertEqual(field["meaning"], "model_resource")
        self.assertEqual(field["value"], "synthetic.variant.4ds")
        self.assertEqual(field["lookup_key"], "synthetic.variant")
        self.assertEqual(field["required_frame_kind"], 9)
        self.assertEqual(field["active_creation_kind"], 9)

    def test_model_resource_does_not_infer_actual_frame_kind(self):
        data = self.scene(chunk(0x2012, b"before\0")
                          + chunk(0x4011, struct.pack("<I", 3))
                          + chunk(0x2012, b"after\0"))
        fields = decode_objects(data)["records"][0]["fields"]
        self.assertEqual(fields[0]["active_creation_kind"], 0)
        self.assertEqual(fields[2]["active_creation_kind"], 3)
        self.assertEqual(fields[0]["required_frame_kind"], 9)
        self.assertNotIn("applied", fields[0])

    def test_model_resource_name_validation(self):
        for name in [b"", b"unterminated", b"two\0names\0", b"\xff\0"]:
            with self.subTest(name=name), self.assertRaises(ValueError):
                decode_objects(self.scene(chunk(0x2012, name)))
        field = decode_objects(self.scene(chunk(0x2012, b"\0")))["records"][0]["fields"][0]
        self.assertEqual(field["value"], "")


class MissionEvidenceTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        binary = self.write_fixture("binary.fixture", b"synthetic executable")
        engine = self.write_fixture("engine.fixture", b"synthetic engine")
        scene = self.write_fixture("scene2.bin", self.scene(1))
        summary = summarize(self.scene(1))
        decoded = decode_objects(self.scene(1))
        export = {
            "entry": "00000001", "sha256": binary["sha256"], "decompiled": True,
            "signature": "synthetic signature",
            "references": [{"from": "00000010", "to": "00000002", "type": "UNCONDITIONAL_CALL",
                            "symbol": "Synthetic::Normalize"}],
        }
        (self.root / "export.json").write_text(json.dumps(export))
        (self.root / "export.c").write_text("synthetic export fixture")
        self.document = {
            "binary": binary, "engine": engine, "archive": binary, "samples": [scene],
            "observations": {
                "top_level_chunks": 1, "object_records": 1,
                "record_field_counts": summary["record_field_counts"],
                "object_container_offset": 6, "object_container_total_size": len(self.scene(1)) - 6,
                "object_decoding": {
                    "decoded_records": 1,
                    **{key: decoded[key] for key in (
                        "creation_kind_field_counts", "name_action_counts", "decoded_field_counts"
                    )},
                    "unique_object_names": 1, "parent_name_fields": 1,
                    "parent_names_matching_scene2_names": 0,
                    "rotation_squared_norm_tolerance": 0.000001,
                    "model_resources": {
                        "fields": 0, "unique_names": 0, "unique_lookup_keys": 0,
                        "extensionless_names": 0, "active_creation_kind_counts": {},
                    },
                },
            },
            "loader_evidence": {
                "exports": [{"address": "00000001", "path": "export.json",
                             "signature": "synthetic signature"}],
                "calls": [{"from": "00000001", "site": "00000010", "to": "00000002",
                           "symbol": "Synthetic::Normalize"}],
            },
        }

    def scene(self, rotation_scalar):
        return chunk(0x4C53, chunk(0x4000, chunk(
            0x4010, chunk(0x10, b"synthetic\0")
            + chunk(0x22, struct.pack("<4f", rotation_scalar, 0, 0, 0))
            + chunk(0x4020, chunk(0x10, b"external_parent\0"))
        )))

    def write_fixture(self, name, data):
        (self.root / name).write_bytes(data)
        return {"path": name, "size": len(data), "sha256": hashlib.sha256(data).hexdigest()}

    def test_valid_evidence(self):
        validate_mission(self.document, self.root, verify_archive=True)

    def test_wrong_binary_fingerprint(self):
        self.document["binary"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "fingerprint mismatch"):
            validate_mission(self.document, self.root)

    def test_wrong_counts_and_signatures(self):
        for path in [
            ("observations", "object_records"),
            ("observations", "object_decoding", "decoded_records"),
            ("observations", "object_decoding", "unique_object_names"),
            ("observations", "object_decoding", "parent_name_fields"),
            ("observations", "object_decoding", "parent_names_matching_scene2_names"),
            ("observations", "object_decoding", "model_resources", "fields"),
            ("observations", "object_decoding", "model_resources", "unique_lookup_keys"),
        ]:
            document = copy.deepcopy(self.document)
            target = document
            for key in path[:-1]:
                target = target[key]
            target[path[-1]] += 1
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, "evidence mismatch"):
                validate_mission(document, self.root)
        self.document["loader_evidence"]["exports"][0]["signature"] = "invented signature"
        with self.assertRaisesRegex(ValueError, "signature"):
            validate_mission(self.document, self.root)

    def test_fabricated_call(self):
        for key in ("site", "to", "symbol"):
            document = copy.deepcopy(self.document)
            document["loader_evidence"]["calls"][0][key] = "invented"
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, "Unsupported loader call"):
                validate_mission(document, self.root)

    def test_fabricated_model_resource_anchor(self):
        self.document["loader_evidence"]["resource_anchors"] = [
            {"function": "00000001", "site": "invented", "string": "invented resource error"}
        ]
        with self.assertRaisesRegex(ValueError, "Unsupported model resource anchor"):
            validate_mission(self.document, self.root)

    def test_wrong_export_identity_and_missing_decompilation(self):
        path = self.root / "export.json"
        original = json.loads(path.read_text())
        for key, value in [("entry", "ffffffff"), ("sha256", "0" * 64), ("decompiled", False)]:
            path.write_text(json.dumps({**original, key: value}))
            with self.subTest(key=key), self.assertRaises(ValueError):
                validate_mission(self.document, self.root)

    def test_duplicate_exports(self):
        self.document["loader_evidence"]["exports"] *= 2
        with self.assertRaisesRegex(ValueError, "Duplicate evidence"):
            validate_mission(self.document, self.root)

    def test_nonunit_rotation_is_not_silently_accepted(self):
        self.document["samples"][0] = self.write_fixture("scene2.bin", self.scene(2))
        with self.assertRaisesRegex(ValueError, "quaternion norm mismatch"):
            validate_mission(self.document, self.root)

    def test_tolerance_cannot_be_disabled(self):
        for tolerance in [0, -1, 0.001, float("nan")]:
            self.document["observations"]["object_decoding"]["rotation_squared_norm_tolerance"] = tolerance
            with self.subTest(tolerance=tolerance), self.assertRaisesRegex(ValueError, "tolerance"):
                validate_mission(self.document, self.root)


if __name__ == "__main__":
    unittest.main()
