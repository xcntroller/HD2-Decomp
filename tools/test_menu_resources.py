"""Focused synthetic menu staging tests; generated files stay in ignored work."""

import hashlib
import json
from pathlib import Path
import shutil
import struct
import unittest
from unittest.mock import patch
import uuid

from extract_dta_member import xor_record
from stage_menu_resources import (
    MANIFEST, MAPS_KEY, MODEL_KEY, MODEL_MEMBER, stage_menu_resources,
)
from test_model_table import model, record


ROOT = Path(__file__).resolve().parents[1]


def archive(entries, key):
    """Multi-member variant of the raw encrypted ISD0 fixture."""
    body = bytearray()
    table = bytearray()
    for name, payload in entries:
        name = name.encode("ascii")
        header_offset = 20 + len(body)
        data_offset = header_offset + 32 + len(name)
        block = xor_record(b"\0" + payload, key)
        framing = struct.pack("<HH", len(block), 0) + block
        header = struct.pack("<6I", len(framing), 32, 0, 0, len(payload), 1)
        header += bytes([len(name), 0x80]) + bytes(6)
        body += xor_record(header, key) + xor_record(name, key) + framing
        table += struct.pack("<III16s", 0, header_offset, data_offset, bytes(16))
    return (b"ISD0" + xor_record(struct.pack(
        "<4I", len(entries), 20 + len(body), len(table), 0), key)
        + body + xor_record(table, key))


class MenuResourcesTests(unittest.TestCase):
    def setUp(self):
        self.base = ROOT / "analysis/work" / ("menu-synthetic-" + uuid.uuid4().hex)
        self.game = self.base / "game"
        self.run = self.base / "run"
        self.game.mkdir(parents=True)
        self.model = model([
            record(0x40080000, b"one.bmp", b"EXTRA.BMP", b"SECOND.bmp"),
            record(0, b"ONE.BMP"), record(0, b""),
        ])
        self.maps = [
            (r"MAPS\ONE.BMP", b"one"), (r"MAPS\extra.bmp", b"extra"),
            (r"MAPS\SECOND.BMP", b"second"), (r"OTHER\unused.bmp", b"unused"),
            (r"MAPS\Snow.tga", b"default snow"),
        ]
        self.write_archives()

    def tearDown(self):
        shutil.rmtree(self.base)

    def write_archives(self, models=None, maps=None):
        (self.game / "models.dta").write_bytes(archive(
            [(MODEL_MEMBER, self.model)] if models is None else models, MODEL_KEY))
        (self.game / "Maps.dta").write_bytes(archive(
            self.maps if maps is None else maps, MAPS_KEY))

    def assert_no_output(self):
        self.assertFalse(self.run.exists())

    def test_unique_all_name_fields_keys_hashes_and_repeat(self):
        before = {p.name: p.read_bytes() for p in self.game.iterdir()}
        self.run.mkdir()
        unrelated = self.run / "runtime.dll"
        unrelated.symlink_to(self.game / "models.dta")
        (self.run / "keep.txt").write_bytes(b"keep")
        result = stage_menu_resources(self.game, self.run)
        self.assertNotEqual(MODEL_KEY, MAPS_KEY)
        self.assertEqual([asset["path"] for asset in result["assets"]], [
            "Models/main_menu.4ds", "Maps/EXTRA.BMP", "Maps/ONE.BMP", "Maps/SECOND.BMP",
            "Maps/SNOW.TGA",
        ])
        self.assertEqual(result, json.loads((self.run / MANIFEST).read_text()))
        for asset in result["assets"]:
            data = (self.run / asset["path"]).read_bytes()
            self.assertEqual(asset["sha256"], hashlib.sha256(data).hexdigest())
            self.assertEqual(asset["size"], len(data))
        for source in result["archives"]:
            self.assertEqual(source["sha256"],
                             hashlib.sha256(before[source["path"]]).hexdigest())
        snapshot = {p.relative_to(self.run): p.read_bytes()
                    for p in self.run.rglob("*") if p.is_file()}
        self.assertEqual(stage_menu_resources(self.game, self.run), result)
        self.assertEqual(snapshot, {p.relative_to(self.run): p.read_bytes()
                                   for p in self.run.rglob("*") if p.is_file()})
        self.assertTrue(unrelated.is_symlink())
        self.assertEqual((self.run / "keep.txt").read_bytes(), b"keep")
        self.assertEqual(before, {p.name: p.read_bytes() for p in self.game.iterdir()})
        self.assertIn("no patch/expansion overlays", result["limitation"])

    def test_scene_default_prefers_dds_else_tga_and_deduplicates(self):
        for entries, expected in [
            (self.maps, "SNOW.TGA"),
            (self.maps + [(r"MAPS\snow.DDS", b"dds snow")], "SNOW.DDS"),
            ([item for item in self.maps if item[0] != r"MAPS\Snow.tga"]
             + [(r"MAPS\Snow.dds", b"dds snow")], "SNOW.DDS"),
        ]:
            with self.subTest(expected=expected, entries=len(entries)):
                self.model = model([record(0, expected.lower().encode("ascii"))])
                self.write_archives(maps=entries)
                result = stage_menu_resources(self.game, self.run)
                self.assertEqual(len(result["assets"]), 2)
                snow = result["assets"][1]
                self.assertEqual(snow["path"], "Maps/" + expected)
                self.assertEqual(snow["reason"], "scene_default")
                self.assertEqual((self.run / snow["path"]).read_bytes(),
                                 b"default snow" if expected == "SNOW.TGA" else b"dds snow")
                self.assertEqual(stage_menu_resources(self.game, self.run), result)
                shutil.rmtree(self.run)

    def test_missing_scene_default_rejected_without_outputs(self):
        maps = [item for item in self.maps if item[0] != r"MAPS\Snow.tga"]
        self.write_archives(maps=maps + [
            (r"OTHER\Snow.dds", b"wrong"), ("MAPS/Snow.tga", b"wrong"),
        ])
        with self.assertRaisesRegex(ValueError, "Missing scene-default texture"):
            stage_menu_resources(self.game, self.run)
        self.assert_no_output()

    def test_missing_model_texture_or_archive_has_no_outputs(self):
        for kind in ["model", "texture", "archive"]:
            with self.subTest(kind=kind):
                self.write_archives(
                    models=[] if kind == "model" else None,
                    maps=self.maps[1:] if kind == "texture" else None)
                if kind == "archive":
                    (self.game / "Maps.dta").unlink()
                with self.assertRaises((ValueError, FileNotFoundError)):
                    stage_menu_resources(self.game, self.run)
                self.assert_no_output()

    def test_exact_member_paths_no_basename_fallback(self):
        for models, maps in [
            ([(r"MODELS\MAIN_MENU.4DS", self.model)], None),
            (None, [(r"OTHER\ONE.BMP", b"wrong")] + self.maps[1:]),
            (None, [("MAPS/ONE.BMP", b"wrong")] + self.maps[1:]),
        ]:
            with self.subTest(models=models is None, maps=maps is None):
                self.write_archives(models, maps)
                with self.assertRaisesRegex(ValueError, "Missing exact"):
                    stage_menu_resources(self.game, self.run)
                self.assert_no_output()

    def test_unsafe_names_in_every_field_have_no_outputs(self):
        for name in [b"../one.bmp", b"a/b.bmp", b"a\\b.bmp", b"C:one.bmp",
                     b"\xff.bmp", b"a\0.bmp", b".", b"..", b"a..bmp",
                     b"one.bmp.", b"NUL.bmp", b"a b.bmp", b"a*.bmp"]:
            for field in ["primary", "extra", "secondary"]:
                with self.subTest(name=name, field=field):
                    names = dict(primary=b"one.bmp", extra=b"", secondary=b"")
                    names[field] = name
                    self.model = model([record(0x40080000, **names)])
                    self.write_archives()
                    with self.assertRaises(ValueError):
                        stage_menu_resources(self.game, self.run)
                    self.assert_no_output()

    def test_duplicate_case_insensitive_archive_entries(self):
        for which in ["model", "maps"]:
            with self.subTest(which=which):
                self.write_archives(
                    models=[(MODEL_MEMBER, self.model),
                            (MODEL_MEMBER.upper(), self.model)] if which == "model" else None,
                    maps=self.maps + [(r"maps\one.bmp", b"duplicate")]
                    if which == "maps" else None)
                with self.assertRaisesRegex(ValueError, "duplicate archive member"):
                    stage_menu_resources(self.game, self.run)
                self.assert_no_output()

    def test_malformed_model_archive_and_late_extraction(self):
        for kind in ["model", "index", "late"]:
            with self.subTest(kind=kind):
                self.write_archives(models=[(MODEL_MEMBER, b"broken")]
                                    if kind == "model" else None)
                if kind == "index":
                    (self.game / "Maps.dta").write_bytes(b"broken")
                if kind == "late":
                    import stage_menu_resources as stage
                    original = stage.extract_member

                    def fail_late(stream, member, key):
                        if member.name == r"MAPS\SECOND.BMP":
                            raise ValueError("late corrupt dependency")
                        return original(stream, member, key)

                    with patch.object(stage, "extract_member", side_effect=fail_late):
                        with self.assertRaises(ValueError):
                            stage.stage_menu_resources(self.game, self.run)
                else:
                    with self.assertRaises(ValueError):
                        stage_menu_resources(self.game, self.run)
                self.assert_no_output()

    def test_isolation_both_directions_and_alias(self):
        for output in [self.game, self.game / "nested", self.base]:
            with self.subTest(output=output), self.assertRaisesRegex(ValueError, "isolated"):
                stage_menu_resources(self.game, output)
        self.run.symlink_to(self.game, target_is_directory=True)
        with self.assertRaises(ValueError):
            stage_menu_resources(self.game, self.run)
        self.assertEqual(set(p.name for p in self.game.iterdir()), {"models.dta", "Maps.dta"})

    def test_target_parent_and_leaf_symlinks_rejected_without_writes(self):
        outside = self.base / "outside"
        outside.mkdir()
        sentinel = outside / "sentinel"
        sentinel.write_bytes(b"safe")
        for relative in ["Models", "Maps", "Models/main_menu.4ds",
                         "Maps/ONE.BMP", MANIFEST]:
            with self.subTest(relative=relative):
                self.run.mkdir()
                target = self.run / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.symlink_to(outside if relative in ["Models", "Maps"] else sentinel)
                with self.assertRaisesRegex(ValueError, "symlink"):
                    stage_menu_resources(self.game, self.run)
                self.assertEqual(sentinel.read_bytes(), b"safe")
                self.assertEqual(list(outside.iterdir()), [sentinel])
                self.assertFalse((self.run / "Models/main_menu.4ds").is_file()
                                 and relative != "Models/main_menu.4ds")
                shutil.rmtree(self.run)

    def test_unowned_modified_manifest_and_hardlinks_preserved(self):
        self.run.mkdir()
        destination = self.run / "Maps/ONE.BMP"
        destination.parent.mkdir()
        destination.write_bytes(b"unrelated")
        with self.assertRaisesRegex(ValueError, "unrelated"):
            stage_menu_resources(self.game, self.run)
        self.assertEqual(destination.read_bytes(), b"unrelated")
        self.assertFalse((self.run / "Models").exists())
        destination.unlink()
        stage_menu_resources(self.game, self.run)
        destination.write_bytes(b"modified")
        with self.assertRaisesRegex(ValueError, "modified"):
            stage_menu_resources(self.game, self.run)
        self.assertEqual(destination.read_bytes(), b"modified")
        destination.unlink()
        destination.hardlink_to(self.game / "models.dta")
        with self.assertRaisesRegex(ValueError, "regular file"):
            stage_menu_resources(self.game, self.run)
        destination.unlink()
        (self.run / MANIFEST).write_text('{"format": "not ours"}')
        with self.assertRaisesRegex(ValueError, "manifest|menu-resources"):
            stage_menu_resources(self.game, self.run)


if __name__ == "__main__":
    unittest.main()
