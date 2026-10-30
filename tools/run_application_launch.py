"""Stage the rebuilt application controller, original opening movies and bounded Main/Exit menu."""

import argparse
import os
from pathlib import Path
import shlex
import sys

from run_startup_probe import stage_startup
from run_engine_window import prepare_launch, run_launch, frame_count
from stage_menu_resources import stage_menu_resources
from stage_menu_text import stage_menu_text
from stage_opening_movies import stage_opening_movies


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
    args = parser.parse_args()
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
            raise SystemExit(run_launch(launch, sys.stdout))
    except (OSError, ValueError) as error:
        parser.exit(1, f"Controller launch staging failed: {error}\n")


if __name__ == "__main__":
    main()
