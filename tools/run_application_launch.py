"""Stage the rebuilt application controller, original opening movies and bounded Main/Exit menu."""

import argparse
from contextlib import contextmanager
import os
from pathlib import Path
import shlex
import sys
import tempfile

from run_startup_probe import stage_startup
from run_engine_window import prepare_launch, run_launch, frame_count
from stage_menu_resources import stage_menu_resources
from stage_menu_text import stage_menu_text
from stage_opening_movies import OPENING_MOVIES, stage_opening_movies
from stage_menu_text import _check_path


@contextmanager
def omitted_staged_movies(run: Path, names: list[str]):
    """Hide only validated staged copies for a run and restore even on failure."""
    if len(set(names)) != len(names) or any(name not in OPENING_MOVIES for name in names):
        raise ValueError("Missing-movie check requires distinct original opening filenames")
    _check_path(run)
    _check_path(run / ".hd2-launch-workspace", file=True)
    if not (run / ".hd2-launch-workspace").is_file():
        raise ValueError("Missing-movie check requires a rebuilt launch workspace")
    folder = run / "GameMovie"
    _check_path(folder)
    for name in names:
        _check_path(folder / name, file=True)
        if not (folder / name).is_file():
            raise ValueError(f"Staged movie is already missing: {name}")
    if not names:
        yield
        return
    backup = Path(tempfile.mkdtemp(prefix=".omitted-movies-", dir=run.parent))
    moved = []
    try:
        for name in names:
            (folder / name).rename(backup / name)
            moved.append(name)
        yield
    finally:
        collisions = []
        for name in reversed(moved):
            target = folder / name
            if target.exists() or target.is_symlink():
                collisions.append(name)
            else:
                (backup / name).rename(target)
        if collisions:
            raise ValueError(f"Movie path reappeared; preserved backups in {backup}: {collisions}")
        backup.rmdir()


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    steam = Path.home() / ".local/share/Steam"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", type=Path, default=root / "Hidden and Dangerous 2")
    parser.add_argument("--executable", type=Path, default=root / "recovered/build/application_launch.exe")
    # Reuse the accepted isolated copies and their writable profile/config.
    parser.add_argument("--work-dir", type=Path, default=root / "analysis/work/startup-runtime")
    parser.add_argument("--steam-root", type=Path, default=steam)
    parser.add_argument("--proton", type=Path, default=steam / "steamapps/common/Proton - Experimental/proton")
    parser.add_argument("--frames", type=frame_count, default=0)
    parser.add_argument("--stage-only", action="store_true", help="Prepare resources and command without executing Windows code")
    parser.add_argument("--omit-movie", action="append", choices=OPENING_MOVIES, default=[],
                        help="Temporarily hide this staged movie during a desktop run; repeatable")
    args = parser.parse_args()
    if args.stage_only and args.omit_movie:
        parser.error("--omit-movie applies during a desktop run and cannot be combined with --stage-only")
    try:
        executable = stage_startup(args.game_dir, args.executable, args.work_dir, application_launch=True)
        run = executable.parent
        stage_menu_resources(args.game_dir, run, include_confirm=True,
                             include_cursor_model=True, include_click_sound=True)
        stage_menu_text(args.game_dir, run, include_confirm=True, include_control_text=True)
        stage_opening_movies(args.game_dir, run)
        launch = prepare_launch(args.game_dir, executable, args.proton, args.steam_root,
                                args.work_dir, args.frames, dict(os.environ))
        print("Controller launch command: " + shlex.join(launch.command))
        print("Runtime log: " + str(run / "host-runtime.log"))
        if not args.stage_only:
            if args.omit_movie:
                print("Missing-movie check: " + ", ".join(args.omit_movie))
            with omitted_staged_movies(run, args.omit_movie):
                result = run_launch(launch, sys.stdout)
            raise SystemExit(result)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Controller launch staging failed: {error}\n")


if __name__ == "__main__":
    main()
