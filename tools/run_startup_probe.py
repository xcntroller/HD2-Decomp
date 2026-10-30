"""Stage and run the bounded rebuilt startup integration in an isolated workspace."""

import argparse
import hashlib
import os
from pathlib import Path
import shlex
import shutil
import sys
import tempfile

from run_engine_window import prepare_launch, run_launch, frame_count
from validate_engine_model import ENGINE_SHA256, PEImage
from validate_joystick_devices import JOY_SHA256
from validate_sound_codec import RW_DATA_SHA256


def checked_file(path: Path) -> None:
    if path.is_symlink() or not path.is_file() or path.stat().st_nlink != 1:
        raise ValueError(f"Expected an ordinary, non-linked runtime file: {path}")


def digest(path: Path) -> bytes:
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").digest()


def stage_startup(game: Path, executable: Path, work: Path) -> Path:
    # This program never launches the original client. Original archive bytes
    # are COPIED; writable config/profile files are copied once, never replaced.
    if work.is_symlink():
        raise ValueError("Startup workspace must not be a symlink")
    game, executable, work = (path.resolve() for path in (game, executable, work))
    if game.is_relative_to(work) or work.is_relative_to(game):
        raise ValueError("Startup workspace must be separate from the original installation")
    if executable.is_relative_to(game):
        raise ValueError("Startup requires the rebuilt executable outside the original installation")
    data = executable.read_bytes()
    PEImage(data)
    if b"Recovered startup integration probe" not in data:
        raise ValueError("Executable is not the rebuilt startup probe")
    for name, expected in (("LS3DF.dll", ENGINE_SHA256), ("rw_data.dll", RW_DATA_SHA256), ("IJoy.dll", JOY_SHA256)):
        if digest(game / name).hex() != expected:
            raise ValueError(f"Unsupported startup dependency: {name}")
    run = work / "run"
    if run.is_symlink() or not run.resolve().is_relative_to(work):
        raise ValueError("Startup runtime path escapes the workspace")
    sources = {p.name.casefold(): p for p in game.iterdir()}
    names = ("LangEnglish.dta", "Patch.dta", "Models.dta", "Missions.dta", "Maps.dta",
             "Sounds.dta", "Scripts.dta", "Others.dta", "Maps_C.dta", "Maps_U.dta")
    copies = [(sources[name.casefold()], run / name, False)
              for name in names if name.casefold() in sources]
    # Include any installed language archives; selected-language resolution is
    # performed by the recovered registry reader inside the rebuilt executable.
    for source in game.iterdir():
        if source.name.casefold().startswith("lang") and source.suffix.casefold() == ".dta" and source.name.casefold() != "langenglish.dta":
            copies.append((source, run / source.name, False))
    for name in ("Text", "GameData", "Tables", "PlayersProfiles"):
        source = sources.get(name.casefold())
        if source:
            if source.is_symlink() or not source.is_dir():
                raise ValueError(f"Unsupported startup data directory: {source}")
            for child in source.rglob("*"):
                if child.is_symlink():
                    raise ValueError(f"Linked startup data is not staged: {child}")
                if child.is_file():
                    copies.append((child, run / name / child.relative_to(source), name == "PlayersProfiles"))
    config = sources.get("game.cfg")
    if config:
        copies.append((config, run / "game.cfg", True))
    # Validate every destination before copying. Runtime profiles/config survive
    # subsequent runs; immutable resources reject mismatches instead of overwrites.
    for source, target, mutable in copies:
        checked_file(source)
        if not target.resolve().is_relative_to(run.resolve()):
            raise ValueError(f"Startup file escapes runtime: {target}")
        parent = target.parent
        while parent != run.parent:
            if parent.is_symlink():
                raise ValueError(f"Linked startup destination directory: {parent}")
            parent = parent.parent
        if target.exists() or target.is_symlink():
            checked_file(target)
            if not mutable and (source.stat().st_size != target.stat().st_size or
                    digest(source) != digest(target)):
                raise ValueError(f"Existing staged startup resource differs: {target}")
    output = run / "startup_probe.exe"
    marker = run / ".hd2-startup-workspace"
    for target in (output, marker):
        if target.exists() or target.is_symlink():
            checked_file(target)
    required = sum(source.stat().st_size for source, target, _ in copies if not target.exists()) + len(data)
    ancestor = work
    while not ancestor.exists():
        ancestor = ancestor.parent
    if shutil.disk_usage(ancestor).free < required:
        raise ValueError("Insufficient free space for isolated startup resource copies")
    run.mkdir(parents=True, exist_ok=True)
    for source, target, _ in copies:
        if not target.exists():
            target.parent.mkdir(parents=True, exist_ok=True)
            # Finish a copy before exposing it under its real name. An
            # interrupted staging pass cannot leave a partial resource that
            # blocks the next run, and a concurrent destination is not replaced.
            temporary = None
            try:
                with tempfile.NamedTemporaryFile(dir=target.parent, prefix=".hd2-copy-", delete=False) as stream:
                    temporary = Path(stream.name)
                shutil.copyfile(source, temporary)
                os.link(temporary, target)
            finally:
                if temporary is not None:
                    temporary.unlink(missing_ok=True)
    output.write_bytes(data)
    marker.write_text("hd2-rebuilt-startup-v1\n", encoding="ascii")
    return output


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    steam = Path.home() / ".local/share/Steam"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", type=Path, default=root / "Hidden and Dangerous 2")
    parser.add_argument("--executable", type=Path, default=root / "recovered/build/startup_probe.exe")
    parser.add_argument("--work-dir", type=Path, default=root / "analysis/work/startup-runtime")
    parser.add_argument("--steam-root", type=Path, default=steam)
    parser.add_argument("--proton", type=Path, default=steam / "steamapps/common/Proton - Experimental/proton")
    parser.add_argument("--frames", type=frame_count, default=180)
    parser.add_argument("--stage-only", action="store_true", help="Copy resources and print the command without executing any Windows code")
    args = parser.parse_args()
    try:
        executable = stage_startup(args.game_dir, args.executable, args.work_dir)
        launch = prepare_launch(args.game_dir, executable, args.proton, args.steam_root,
                                args.work_dir, args.frames, dict(os.environ))
        print("Startup integration command: " + shlex.join(launch.command))
        if not args.stage_only:
            raise SystemExit(run_launch(launch, sys.stdout))
    except (OSError, ValueError) as error:
        parser.exit(1, f"Startup staging failed: {error}\n")


if __name__ == "__main__":
    main()
