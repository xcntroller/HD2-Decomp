"""Menu text slice tests; all generated fixtures stay under ignored analysis/work."""

import hashlib
import json
from pathlib import Path
import shutil
import struct
import unittest
import uuid

from stage_menu_text import (
    LABEL_IDS, MANIFEST, SOURCES, TEXT_SOURCE, encode_menu_labels,
    parse_menu_labels, stage_menu_text,
)


ROOT = Path(__file__).resolve().parents[1]


def text_fixture():
    return b"; legacy comment \xe9\r\n" + b"".join(
        str(label_id).encode("ascii") + b'\t"fixture label"\r\n'
        for label_id in LABEL_IDS)


def atlas_fixture():
    return struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, 256, 256, 32, 8) \
        + bytes(256 * 256 * 4)


class MenuTextTests(unittest.TestCase):
    def setUp(self):
        self.base = ROOT / "analysis/work" / ("menu-text-test-" + uuid.uuid4().hex)
        self.game = self.base / "game"
        self.run = self.base / "run"
        self.game.mkdir(parents=True)
        for source in set(SOURCES.values()):
            path = self.game / source
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(text_fixture() if source == TEXT_SOURCE else
                             bytes(5120) if source.endswith(".tbl") else atlas_fixture())

    def tearDown(self):
        shutil.rmtree(self.base)

    def test_localization_framing_and_binary_slice(self):
        data = text_fixture() + b'4481 "unrelated\r\nmultiline text"\r\n8514\r\n'
        labels = parse_menu_labels(data)
        self.assertEqual(tuple(labels), LABEL_IDS)
        raw = encode_menu_labels(labels)
        self.assertEqual(raw[:8], b"H2MT" + struct.pack("<I", len(LABEL_IDS)))
        offset = 8
        for expected in LABEL_IDS:
            label_id, length = struct.unpack_from("<II", raw, offset)
            offset += 8
            self.assertEqual(label_id, expected)
            self.assertEqual(raw[offset:offset + length], labels[expected].encode("ascii"))
            offset += length
        self.assertEqual(offset, len(raw))

    def test_invalid_selected_localization(self):
        first = b'2101\t"fixture label"'
        for replacement in [b'2101 "nonascii \xe9"', b'2101 ""', b'2101 "a\0b"',
                            b'2101 "a\tb"', b'2101 "a" garbage',
                            b'2101 "unterminated', b'2101 unquoted',
                            b'2101 "' + b"a" * 4097 + b'"']:
            with self.subTest(replacement=replacement[:35]):
                with self.assertRaises(ValueError):
                    parse_menu_labels(text_fixture().replace(first, replacement))
        for invalid in [b"", text_fixture().replace(first, b""),
                        text_fixture() + first + b"\r\n", b"x" * (4 * 1024 * 1024 + 1),
                        text_fixture() + b'999999999999999999999 "x"']:
            with self.assertRaises(ValueError):
                parse_menu_labels(invalid)

    def test_multiline_record_cannot_supply_or_duplicate_selected_label(self):
        unrelated = (b'4481 "legacy \xe9 and embedded "quoted words"\r\n'
                     b'2101 "not a menu record"\r\n'
                     b'2102 "also continuation"\r\n'
                     b'last line" ; comment with "ignored quotes\r\n')
        expected = parse_menu_labels(text_fixture())
        for data in [unrelated + text_fixture(), text_fixture() + unrelated]:
            self.assertEqual(parse_menu_labels(data), expected)
        missing = text_fixture().replace(b'2101\t"fixture label"\r\n', b"")
        with self.assertRaisesRegex(ValueError, "Missing static menu labels"):
            parse_menu_labels(unrelated + missing)

    def test_unterminated_or_malformed_unrelated_record_rejected(self):
        for unrelated in [
                b'4481 "never closed\r\n2101 "fake continuation"\r\n',
                b'4481 no opening quote\r\n',
                b'4481 "closed" garbage\r\n']:
            for data in [unrelated + text_fixture(), text_fixture() + unrelated]:
                with self.subTest(unrelated=unrelated, prefix=data.startswith(unrelated)):
                    with self.assertRaises(ValueError):
                        parse_menu_labels(data)

    def test_unrelated_embedded_quotes_comments_and_bare_markers(self):
        unrelated = (b'4481 "prose with "paired quotes" and \xe9"\r\n'
                     b'4482 "" ; ignored "unbalanced quote\r\n'
                     b'; 2101 "comment"\r\n// 2102 "comment"\r\n8514\r\n')
        self.assertEqual(parse_menu_labels(unrelated + text_fixture()),
                         parse_menu_labels(text_fixture()))

    def snapshot(self, path):
        return {str(p.relative_to(path)): p.read_bytes()
                for p in path.rglob("*") if p.is_file()}

    def test_stage_provenance_repeat_and_preserve_unrelated(self):
        before = self.snapshot(self.game)
        self.run.mkdir()
        (self.run / "keep.txt").write_bytes(b"unrelated")
        result = stage_menu_text(self.game, self.run)
        self.assertEqual(len(result["assets"]), 5)
        self.assertIn("800x600", result["limitation"])
        self.assertEqual(set(a["path"] for a in result["assets"]), set(SOURCES))
        for asset in result["assets"]:
            data = (self.run / asset["path"]).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(), asset["sha256"])
            self.assertEqual(len(data), asset["size"])
            if asset["path"] != "MenuText/labels.bin":
                self.assertEqual(data, before[asset["source"]])
        for source in result["sources"]:
            self.assertEqual(source["sha256"], hashlib.sha256(before[source["path"]]).hexdigest())
        self.assertEqual(result, json.loads((self.run / MANIFEST).read_text()))
        staged = self.snapshot(self.run)
        self.assertEqual(stage_menu_text(self.game, self.run), result)
        self.assertEqual(staged, self.snapshot(self.run))
        self.assertEqual(before, self.snapshot(self.game))
        self.assertEqual((self.run / "keep.txt").read_bytes(), b"unrelated")

    def test_bad_sources_preflight_without_output(self):
        for relative, bad in [(TEXT_SOURCE, b"missing labels"),
                              ("Text/Fonts/ANSI/Set2/BIG.tbl", bytes(5119)),
                              ("Text/Fonts/ANSI/Set2/BIG.tbl",
                               struct.pack("<f", float("nan")) + bytes(5116)),
                              ("Text/Fonts/ANSI/Set2/BIG.tga", b"bad atlas")]:
            with self.subTest(relative=relative, size=len(bad)):
                path = self.game / relative
                original = path.read_bytes()
                path.write_bytes(bad)
                with self.assertRaises(ValueError):
                    stage_menu_text(self.game, self.run)
                self.assertFalse(self.run.exists())
                path.write_bytes(original)

    def test_reject_overlap(self):
        for target in [self.game, self.game / "nested", self.base]:
            with self.assertRaises(ValueError):
                stage_menu_text(self.game, target)
        self.assertFalse(self.run.exists())

    def test_reject_source_links(self):
        path = self.game / TEXT_SOURCE
        original = path.read_bytes()
        saved = self.base / "original.txt"
        saved.write_bytes(original)
        path.unlink()
        for kind in ["symlink", "hardlink"]:
            with self.subTest(kind=kind):
                if kind == "symlink":
                    path.symlink_to(saved)
                else:
                    path.hardlink_to(saved)
                with self.assertRaises(ValueError):
                    stage_menu_text(self.game, self.run)
                self.assertFalse(self.run.exists())
                path.unlink()
        path.write_bytes(original)
        alias = self.base / "alias"
        alias.symlink_to(self.game, target_is_directory=True)
        with self.assertRaises(ValueError):
            stage_menu_text(alias, self.run)

    def test_reject_target_links_collisions_and_manifest(self):
        for relative in [*SOURCES, MANIFEST]:
            with self.subTest(relative=relative):
                target = self.run / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(b"unowned")
                before = self.snapshot(self.run)
                with self.assertRaises(ValueError):
                    stage_menu_text(self.game, self.run)
                self.assertEqual(before, self.snapshot(self.run))
                shutil.rmtree(self.run)
        for kind in ["file-symlink", "file-hardlink", "directory-symlink", "root-symlink"]:
            with self.subTest(kind=kind):
                saved = self.base / "saved"
                saved.write_bytes(b"untouched")
                if kind == "root-symlink":
                    self.run.symlink_to(self.game, target_is_directory=True)
                else:
                    self.run.mkdir()
                    if kind == "directory-symlink":
                        (self.run / "Maps").symlink_to(self.game, target_is_directory=True)
                    else:
                        (self.run / "Maps").mkdir()
                        target = self.run / "Maps/HD2_MENU_BIG.TGA"
                        if kind == "file-symlink":
                            target.symlink_to(saved)
                        else:
                            target.hardlink_to(saved)
                with self.assertRaises(ValueError):
                    stage_menu_text(self.game, self.run)
                self.assertEqual(saved.read_bytes(), b"untouched")
                if self.run.is_symlink():
                    self.run.unlink()
                else:
                    shutil.rmtree(self.run)

    def test_modified_and_forged_owned_files_rejected(self):
        stage_menu_text(self.game, self.run)
        target = self.run / "MenuText/BIG.tbl"
        target.write_bytes(b"modified")
        before = self.snapshot(self.run)
        with self.assertRaises(ValueError):
            stage_menu_text(self.game, self.run)
        self.assertEqual(before, self.snapshot(self.run))
        manifest_path = self.run / MANIFEST
        manifest = json.loads(manifest_path.read_text())
        manifest["assets"][0]["path"] = "../escape"
        manifest_path.write_text(json.dumps(manifest))
        with self.assertRaises(ValueError):
            stage_menu_text(self.game, self.run)
        self.assertFalse((self.base / "escape").exists())

    def test_original_loose_assets_read_only(self):
        game = ROOT / "Hidden and Dangerous 2"
        if not (game / TEXT_SOURCE).is_file():
            self.skipTest("Original game installation is unavailable")
        originals = {relative: (game / relative).read_bytes()
                     for relative in set(SOURCES.values())}
        labels = parse_menu_labels(originals[TEXT_SOURCE])
        self.assertEqual(len(labels), len(LABEL_IDS))
        self.assertEqual(labels[2101], "MAIN MENU")
        result = stage_menu_text(game, self.run)
        self.assertEqual(len(result["assets"]), 5)
        staged = self.snapshot(self.run)
        self.assertEqual(stage_menu_text(game, self.run), result)
        self.assertEqual(self.snapshot(self.run), staged)
        for relative, original in originals.items():
            self.assertEqual((game / relative).read_bytes(), original)
        self.assertEqual((self.run / "MenuText/labels.bin").read_bytes(),
                         encode_menu_labels(labels))


if __name__ == "__main__":
    unittest.main()
