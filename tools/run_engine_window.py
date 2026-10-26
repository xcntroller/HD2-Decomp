"""Run the rebuilt engine host, optionally previewing the original static menu model."""

import argparse
from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import subprocess
import sys
from typing import TextIO

from validate_engine_model import ENGINE_SHA256, PEImage
from stage_menu_resources import stage_menu_resources
from stage_menu_text import stage_menu_text


@dataclass
class Launch:
    command: list[str]
    cwd: Path
    env: dict[str, str]


def prepare_launch(game: Path, executable: Path, proton: Path, steam: Path,
                   work: Path, frames: int, environment: dict[str, str],
                   menu_model: bool = False, menu_ui: bool = False) -> Launch:
    game, executable, proton, steam, work = (
        path.resolve() for path in (game, executable, proton, steam, work)
    )
    if work.is_relative_to(game) or game.is_relative_to(work):
        raise ValueError("The runtime workspace must be separate from the original game directory")
    if not 0 <= frames <= 0xFFFFFFFF:
        raise ValueError("Frame limit must fit an unsigned 32-bit integer")
    engine = game / "LS3DF.dll"
    if hashlib.sha256(engine.read_bytes()).hexdigest() != ENGINE_SHA256:
        raise ValueError("Unsupported LS3DF.dll fingerprint; no game code was launched")
    PEImage(executable.read_bytes())
    if not proton.is_file() or not os.access(proton, os.X_OK):
        raise ValueError(f"Proton launcher is missing or not executable: {proton}")
    if not steam.is_dir():
        raise ValueError(f"Steam installation directory does not exist: {steam}")
    for name in ("run", "run/host-output.log", "run/host-runtime.log",
                 "compat", "compat/pfx", "shaders"):
        if not (work / name).resolve().is_relative_to(work):
            raise ValueError(f"Runtime path escapes the isolated workspace: {work / name}")
    run = work / "run"
    compat = work / "compat"
    run.mkdir(parents=True, exist_ok=True)
    compat.mkdir(parents=True, exist_ok=True)
    if menu_model or menu_ui:
        stage_menu_resources(game, run)
    if menu_ui:
        stage_menu_text(game, run)
    env = dict(environment)
    env.update({
        "STEAM_COMPAT_CLIENT_INSTALL_PATH": str(steam),
        "STEAM_COMPAT_DATA_PATH": str(compat),
        "STEAM_COMPAT_INSTALL_PATH": str(run),
        "STEAM_COMPAT_SHADER_PATH": str(work / "shaders"),
        "SteamAppId": "0",
        "SteamGameId": "0",
        "STEAM_COMPAT_APP_ID": "0",
        "PROTON_LOG": "1",
        "PROTON_LOG_DIR": str(run),
    })
    engine_windows = "Z:" + str(engine).replace("/", "\\")
    command = [str(proton), "run", str(executable), engine_windows, "--frames", str(frames)]
    if menu_ui:
        command.append("--menu-ui")
    elif menu_model:
        command.append("--menu-model")
    return Launch(command, run, env)


def run_launch(launch: Launch, output: TextIO) -> int:
    log_path = launch.cwd / "host-output.log"
    runtime_path = launch.cwd / "host-runtime.log"
    for path in (log_path, runtime_path):
        if path.is_symlink():
            raise ValueError("Host diagnostic log must not be a symlink")
        if path.exists() and (not path.is_file() or path.stat().st_nlink != 1):
            raise ValueError("Host diagnostic log must be a regular non-hardlinked file")
    runtime_path.write_text("", encoding="utf-8")
    with log_path.open("w", encoding="utf-8") as log:
        with subprocess.Popen(launch.command, cwd=launch.cwd, env=launch.env,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              text=True, encoding="utf-8", errors="replace") as process:
            assert process.stdout is not None
            for line in process.stdout:
                log.write(line)
                log.flush()
                output.write(line)
                output.flush()
            returncode = process.wait()
        with runtime_path.open(encoding="utf-8", errors="replace") as runtime:
            for line in runtime:
                log.write(line)
                output.write(line)
        log.flush()
        output.flush()
        return returncode


def frame_count(text: str) -> int:
    if not text.isascii() or not text.isdecimal() or int(text) > 0xFFFFFFFF:
        raise argparse.ArgumentTypeError("Frames must be decimal digits in 0..4294967295")
    return int(text)


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    steam = Path.home() / ".local/share/Steam"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", type=Path, default=root / "Hidden and Dangerous 2")
    parser.add_argument("--executable", type=Path, default=root / "recovered/build/engine_window.exe")
    parser.add_argument("--steam-root", type=Path, default=steam)
    parser.add_argument("--proton", type=Path,
                        default=steam / "steamapps/common/Proton - Experimental/proton")
    parser.add_argument("--frames", type=frame_count, default=180,
                        help="Post Close after this many frames; 0 waits for Escape/Close")
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--menu-model", action="store_true",
                        help="Stage and preview the original base-archive menu artwork; "
                             "not an interactive menu")
    modes.add_argument("--menu-ui", action="store_true",
                       help="Original artwork, cursor and static English bitmap labels at "
                            "800x600; only Exit is enabled")
    args = parser.parse_args()
    try:
        launch = prepare_launch(args.game_dir, args.executable, args.proton, args.steam_root,
                                root / "analysis/work/engine-window", args.frames, dict(os.environ),
                                menu_model=args.menu_model, menu_ui=args.menu_ui)
        print("Static original-font menu slice: only Exit closes this host; "
              "other screens, profile/save data and movies are not implemented."
              if args.menu_ui else
              "Original static menu-model preview: text/input/movies are not implemented."
              if args.menu_model else
              "Graphics bootstrap only: no original menu or movies are loaded.", flush=True)
        print(f"Isolated runtime files: {launch.cwd}", flush=True)
        print(f"Saved terminal diagnostics: {launch.cwd / 'host-output.log'}", flush=True)
        print(f"Direct Windows host diagnostics: {launch.cwd / 'host-runtime.log'}", flush=True)
        returncode = run_launch(launch, sys.stdout)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Engine-window launch failed: {error}\n")
    if returncode != 0:
        print(f"Proton exited with status {returncode}; no successful window run "
              f"was verified. Diagnostics: {launch.cwd / 'steam-0.log'}", file=sys.stderr)
    raise SystemExit(returncode if returncode >= 0 else 128 - returncode)


if __name__ == "__main__":
    main()
