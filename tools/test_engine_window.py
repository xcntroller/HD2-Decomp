"""Test engine-host launch isolation and rejection without running commercial code."""

import argparse
from pathlib import Path
import tempfile
import unittest
from unittest.mock import MagicMock, patch
import json

from run_engine_window import frame_count, prepare_launch
from test_engine_model import synthetic_pe
from validate_engine_model import ENGINE_SHA256
from validate_engine_host import validate


class EngineWindowLaunchTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.game = self.root / "original game"
        self.game.mkdir()
        (self.game / "LS3DF.dll").write_bytes(b"synthetic engine")
        self.exe = self.root / "engine_window.exe"
        self.exe.write_bytes(synthetic_pe())
        self.proton = self.root / "proton"
        self.proton.write_text("#!/bin/sh\nexit 0\n")
        self.proton.chmod(0o700)
        self.work = self.root / "runtime"

    def prepare(self, **overrides):
        arguments = dict(game=self.game, executable=self.exe, proton=self.proton,
                         steam=self.root, work=self.work, frames=3,
                         environment={"STEAM_COMPAT_DATA_PATH": "/unrelated/game/prefix"})
        arguments.update(overrides)
        return prepare_launch(**arguments)

    def test_isolated_command_and_environment(self):
        with patch("run_engine_window.hashlib.sha256") as digest:
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            launch = self.prepare()
        self.assertEqual(launch.cwd, self.work / "run")
        self.assertTrue(launch.cwd.is_dir())
        self.assertEqual(launch.env["STEAM_COMPAT_DATA_PATH"], str(self.work / "compat"))
        self.assertEqual(launch.env["STEAM_COMPAT_INSTALL_PATH"], str(self.work / "run"))
        self.assertEqual(launch.env["SteamAppId"], "0")
        self.assertEqual(launch.env["PROTON_LOG_DIR"], str(self.work / "run"))
        self.assertEqual(launch.command, [
            str(self.proton), "run", str(self.exe),
            "Z:" + str(self.game / "LS3DF.dll").replace("/", "\\"), "--frames", "3",
        ])
        self.assertEqual(list(self.game.iterdir()), [self.game / "LS3DF.dll"])

    def test_wrong_fingerprint_rejected_before_creating_runtime(self):
        with self.assertRaisesRegex(ValueError, "fingerprint"):
            self.prepare()
        self.assertFalse(self.work.exists())

    def test_game_directory_overlap_and_invalid_counts_rejected(self):
        for override in ({"work": self.game}, {"work": self.game / "run"},
                         {"work": self.root}, {"frames": -1}, {"frames": 0x100000000}):
            with self.subTest(override=override), self.assertRaises(ValueError):
                self.prepare(**override)
        self.assertFalse(self.work.exists())

    def test_bad_executable_and_missing_proton_rejected(self):
        with patch("run_engine_window.hashlib.sha256") as digest:
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            for override in ({"executable": self.game / "LS3DF.dll"},
                             {"proton": self.root / "missing"},
                             {"steam": self.root / "missing"}):
                with self.subTest(override=override), self.assertRaises(ValueError):
                    self.prepare(**override)
        self.assertFalse(self.work.exists())

    def test_runtime_symlink_cannot_redirect_writes_to_game(self):
        for name in ("run", "compat/pfx"):
            target = self.work / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.symlink_to(self.game, target_is_directory=True)
            with patch("run_engine_window.hashlib.sha256") as digest:
                digest.return_value.hexdigest.return_value = ENGINE_SHA256
                with self.subTest(name=name), self.assertRaisesRegex(ValueError, "escapes"):
                    self.prepare()
            target.unlink()
        self.assertEqual(list(self.game.iterdir()), [self.game / "LS3DF.dll"])

    def test_strict_frame_argument(self):
        for text, expected in (("0", 0), ("180", 180), ("4294967295", 0xFFFFFFFF)):
            self.assertEqual(frame_count(text), expected)
        for text in ("", "-1", "+1", " 1", "1 ", "1x", "4294967296", "\u0661"):
            with self.subTest(text=text), self.assertRaises(argparse.ArgumentTypeError):
                frame_count(text)

    def test_menu_mode_stages_assets_and_passes_explicit_host_flag(self):
        with patch("run_engine_window.hashlib.sha256") as digest, \
                patch("run_engine_window.stage_menu_resources") as stage:
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            launch = self.prepare(menu_model=True)
        stage.assert_called_once_with(self.game, self.work / "run")
        self.assertEqual(launch.command[-1], "--menu-model")

    def test_default_does_not_stage_assets(self):
        with patch("run_engine_window.hashlib.sha256") as digest, \
                patch("run_engine_window.stage_menu_resources") as stage:
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            self.prepare()
        stage.assert_not_called()

    def test_missing_menu_assets_prevent_launch(self):
        with patch("run_engine_window.hashlib.sha256") as digest, \
                patch("run_engine_window.stage_menu_resources",
                      side_effect=ValueError("Missing menu texture")):
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            with self.assertRaisesRegex(ValueError, "Missing menu texture"):
                self.prepare(menu_model=True)


class EngineHostEvidenceTests(unittest.TestCase):
    def test_both_map_fingerprints_checked_before_file_access(self):
        root = Path(__file__).resolve().parents[1]
        for name in ("engine-host-map.json", "menu-scene-map.json"):
            for key in ("engine", "client"):
                document = json.loads((root / "analysis" / name).read_text())
                document[key]["binary"]["sha256"] = "0" * 64
                filesystem = MagicMock()
                with self.subTest(map=name, key=key), \
                        self.assertRaisesRegex(ValueError, "Unsupported"):
                    validate(document, filesystem)
                filesystem.__truediv__.assert_not_called()


if __name__ == "__main__":
    unittest.main()
