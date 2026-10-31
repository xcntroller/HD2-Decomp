"""Copy available original profile-catalog movies without changing unlock flags.

Missing installed files retain their original open-error behavior, not a movie
substitution. Africa/Czech intro catalog entries are unused development remnants (user-confirmed).
"""
import hashlib
import os
from pathlib import Path
import shutil
import tempfile
from stage_menu_text import _check_path

MOVIES = ("Intro.bik", "IntroArctic.bik", "IntroAfrica.bik", "IntroNorway.bik",
          "IntroBurma.bik", "IntroAlps.bik", "IntroNormandy.bik", "IntroCzech.avi",
          "Outro1.bik", "Outro2.bik")

#00667EA0 formats GameMovie\\%s2, including the existing extension.
PREVIEWS = tuple(name + "2" for name in MOVIES)
UNUSED_MOVIES = ("IntroAfrica.bik", "IntroCzech.avi")


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def stage_cinematics_movies(game: Path, run: Path) -> dict:
    game, run = Path(game).absolute(), Path(run).absolute()
    _check_path(game)
    _check_path(run)
    game, run = game.resolve(strict=True), run.resolve()
    if game == run or game in run.parents or run in game.parents:
        raise ValueError("Cinematics output must be isolated from the installation")
    folder = game / "Gamemovie"
    _check_path(folder)
    sources = {}
    for path in folder.iterdir():
        key = path.name.casefold()
        if key in sources:
            raise ValueError("Ambiguous installed Windows movie filename")
        sources[key] = path
    assets, missing = [], []
    for name in MOVIES + PREVIEWS:
        source = sources.get(name.casefold())
        if source is None:
            movie_name = name[:-1] if name in PREVIEWS else name
            if movie_name not in UNUSED_MOVIES:
                missing.append(name)
            continue
        _check_path(source, file=True)
        with source.open("rb") as stream:
            magic = stream.read(4)
        if not (magic[:3] == b"BIK" if name.lower().endswith((".bik", ".bik2")) else magic == b"RIFF"):
            raise ValueError(f"Unexpected original Cinematics media header: {name}")
        target = run / "GameMovie" / name
        _check_path(target, file=True)
        source_hash = digest(source)
        if target.exists() and (target.stat().st_size != source.stat().st_size or digest(target) != source_hash):
            raise ValueError(f"Refusing to overwrite modified Cinematics movie: {target}")
        assets.append((name, source, target, source_hash))
    for _, source, target, source_hash in assets:
        if target.exists():
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=target.parent, prefix=".hd2-cinematics-", delete=False) as stream:
                temporary = Path(stream.name)
                with source.open("rb") as original:
                    shutil.copyfileobj(original, stream)
            if digest(temporary) != source_hash:
                raise ValueError("Cinematics movie changed during copy")
            os.link(temporary, target)
        finally:
            if temporary is not None:
                temporary.unlink(missing_ok=True)
    return {"assets": [{"name": name, "sha256": sha, "size": source.stat().st_size}
                       for name, source, _, sha in assets], "missing": missing,
            "unused_catalog_entries": list(UNUSED_MOVIES)}
