"""Test engine-host launch isolation and rejection without running commercial code."""

import argparse
from pathlib import Path
import tempfile
import unittest
from unittest.mock import MagicMock, patch
import json
import io
import sys

from run_engine_window import Launch, frame_count, prepare_launch, run_launch
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
        for name in ("run", "compat/pfx", "run/host-output.log", "run/host-runtime.log"):
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

    def test_ui_mode_stages_original_fonts_and_artwork_and_is_explicit(self):
        with patch("run_engine_window.hashlib.sha256") as digest, \
                patch("run_engine_window.stage_menu_resources") as artwork, \
                patch("run_engine_window.stage_menu_text") as text:
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            launch = self.prepare(menu_ui=True)
        artwork.assert_called_once_with(self.game, self.work / "run")
        text.assert_called_once_with(self.game, self.work / "run")
        self.assertEqual(launch.command[-1], "--menu-ui")
        self.assertNotIn("--menu-model", launch.command)

    def test_old_preview_and_bootstrap_do_not_require_fonts(self):
        with patch("run_engine_window.hashlib.sha256") as digest, \
                patch("run_engine_window.stage_menu_resources"), \
                patch("run_engine_window.stage_menu_text") as text:
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            self.prepare(menu_model=True)
            self.prepare()
        text.assert_not_called()

    def test_invalid_ui_assets_prevent_command_dispatch(self):
        with patch("run_engine_window.hashlib.sha256") as digest, \
                patch("run_engine_window.stage_menu_resources"), \
                patch("run_engine_window.stage_menu_text", side_effect=ValueError("Bad font")):
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            with self.assertRaisesRegex(ValueError, "Bad font"):
                self.prepare(menu_ui=True)

    def test_missing_menu_assets_prevent_launch(self):
        with patch("run_engine_window.hashlib.sha256") as digest, \
                patch("run_engine_window.stage_menu_resources",
                      side_effect=ValueError("Missing menu texture")):
            digest.return_value.hexdigest.return_value = ENGINE_SHA256
            with self.assertRaisesRegex(ValueError, "Missing menu texture"):
                self.prepare(menu_model=True)


class HostOutputTests(unittest.TestCase):
    def test_direct_windows_log_is_relayed_even_without_standard_handles(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "host-runtime.log").write_text("stale run")
            launch = Launch([sys.executable, "-c",
                             "from pathlib import Path; "
                             "Path('host-runtime.log').write_text('original cursor loaded\\n')"],
                            root, {})
            output = io.StringIO()
            self.assertEqual(run_launch(launch, output), 0)
            self.assertEqual(output.getvalue(), "original cursor loaded\n")
            self.assertEqual((root / "host-output.log").read_text(), output.getvalue())

    def test_stdout_and_stderr_are_saved_and_relayed_with_exit_status(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            launch = Launch([sys.executable, "-c",
                             "import sys; print('model diagnostic', flush=True); "
                             "print('render failure', file=sys.stderr); sys.exit(7)"], root, {})
            output = io.StringIO()
            self.assertEqual(run_launch(launch, output), 7)
            self.assertEqual((root / "host-output.log").read_text(), output.getvalue())
            self.assertIn("model diagnostic", output.getvalue())
            self.assertIn("render failure", output.getvalue())

    def test_symlink_log_is_rejected_without_modifying_target(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            target = root / "unrelated"
            target.write_text("preserve")
            (root / "host-output.log").symlink_to(target)
            with self.assertRaisesRegex(ValueError, "symlink"):
                run_launch(Launch(["not-launched"], root, {}), io.StringIO())
            self.assertEqual(target.read_text(), "preserve")

    def test_hardlinked_runtime_log_cannot_truncate_an_unrelated_file(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            target = root / "unrelated"
            target.write_text("preserve")
            (root / "host-runtime.log").hardlink_to(target)
            with self.assertRaisesRegex(ValueError, "hardlinked"):
                run_launch(Launch(["not-launched"], root, {}), io.StringIO())
            self.assertEqual(target.read_text(), "preserve")


class EngineHostEvidenceTests(unittest.TestCase):
    def test_both_map_fingerprints_checked_before_file_access(self):
        root = Path(__file__).resolve().parents[1]
        for name in ("engine-host-map.json", "menu-scene-map.json", "menu-cursor-map.json",
                     "menu-ui-map.json"):
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
