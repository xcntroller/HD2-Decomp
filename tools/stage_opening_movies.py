"""Copy the four original opening Bink files into the isolated runtime directory."""

import hashlib
from pathlib import Path
import shutil

from stage_menu_text import _check_path


OPENING_MOVIES = ("Logo1.bik", "Logo2.bik", "Logo3.bik", "Intro.bik")


def _digest(path: Path) -> str:
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def stage_opening_movies(game: Path, run: Path) -> None:
    game, run = Path(game).absolute(), Path(run).absolute()
    _check_path(game)
    _check_path(run)
    if game == run or game in run.parents or run in game.parents:
        raise ValueError("Game and output directories must not overlap")
    folder = run / "GameMovie"
    _check_path(folder)
    copies = []
    for name in OPENING_MOVIES:
        source, target = game / "Gamemovie" / name, folder / name
        _check_path(source, file=True)
        _check_path(target, file=True)
        with source.open("rb") as stream:
            if stream.read(3) != b"BIK":
                raise ValueError(f"Expected original Bink file: {name}")
        source_hash = _digest(source)
        if target.exists():
            if _digest(target) != source_hash:
                raise ValueError(f"Refusing to overwrite modified opening movie: {target}")
        else:
            copies.append((source, target, source_hash))
    folder.mkdir(parents=True, exist_ok=True)
    for source, target, source_hash in copies:
        # Exclusive creation also refuses a destination appearing after preflight.
        with source.open("rb") as src, target.open("xb") as dst:
            shutil.copyfileobj(src, dst)
        if _digest(target) != source_hash:
            raise ValueError(f"Opening movie copy verification failed: {target}")
