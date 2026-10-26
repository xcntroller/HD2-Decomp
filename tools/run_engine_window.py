"""Run the rebuilt engine host, optionally previewing the original static menu model."""

import argparse
from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import subprocess
import sys

from validate_engine_model import ENGINE_SHA256, PEImage
from stage_menu_resources import stage_menu_resources


@dataclass
class Launch:
    command: list[str]
    cwd: Path
    env: dict[str, str]


def prepare_launch(game: Path, executable: Path, proton: Path, steam: Path,
                   work: Path, frames: int, environment: dict[str, str],
                   menu_model: bool = False) -> Launch:
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
    for name in ("run", "compat", "compat/pfx", "shaders"):
        if not (work / name).resolve().is_relative_to(work):
            raise ValueError(f"Runtime path escapes the isolated workspace: {work / name}")
    run = work / "run"
    compat = work / "compat"
    run.mkdir(parents=True, exist_ok=True)
    compat.mkdir(parents=True, exist_ok=True)
    if menu_model:
        stage_menu_resources(game, run)
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
    if menu_model:
        command.append("--menu-model")
    return Launch(command, run, env)


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
    parser.add_argument("--menu-model", action="store_true",
                        help="Stage and preview the original base-archive menu artwork; "
                             "not an interactive menu")
    args = parser.parse_args()
    try:
        launch = prepare_launch(args.game_dir, args.executable, args.proton, args.steam_root,
                                root / "analysis/work/engine-window", args.frames, dict(os.environ),
                                menu_model=args.menu_model)
        print("Original static menu-model preview: text/input/movies are not implemented."
              if args.menu_model else
              "Graphics bootstrap only: no original menu or movies are loaded.", flush=True)
        print(f"Isolated runtime files: {launch.cwd}", flush=True)
        result = subprocess.run(launch.command, cwd=launch.cwd, env=launch.env, check=False)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Engine-window launch failed: {error}\n")
    if result.returncode != 0:
        print(f"Proton exited with status {result.returncode}; no successful window run "
              f"was verified. Diagnostics: {launch.cwd / 'steam-0.log'}", file=sys.stderr)
    raise SystemExit(result.returncode if result.returncode >= 0 else 128 - result.returncode)


if __name__ == "__main__":
    main()
