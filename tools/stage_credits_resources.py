"""Stage the two original CE atlases needed by the scrolling Credits branch.

Startup staging already copies the original Text tree, including credits.txt and
CE metrics. No credits.bik is required: it is the original text-branch selector.
"""
import hashlib
import os
import tempfile
from pathlib import Path
from stage_menu_text import _check_path, _read_source, _validate_atlas, _validate_font


def stage_credits_resources(game: Path, run: Path) -> dict:
    game, run = Path(game).absolute(), Path(run).absolute()
    _check_path(game)
    _check_path(run)
    game, run = game.resolve(strict=True), run.resolve()
    if game == run or game in run.parents or run in game.parents:
        raise ValueError("Game and Credits output directories must not overlap")
    contents = {}
    for size in ("BIG", "SMALL"):
        source = f"Text/Fonts/CE/Set2/{size}.tga"
        data = _read_source(game, source)
        if size == "BIG":
            # Original CE BIG is512x512 with a legacy extended TGA tail. Preserve
            # it byte-for-byte; the ANSI validator deliberately accepts only256.
            if hashlib.sha256(data).hexdigest() != "62b0dce3f95d0332333e869f7c3b19c9117d70603a9a22992b76d2e9bbae47ad":
                raise ValueError("Unsupported original CE BIG atlas")
        else:
            _validate_atlas(data)
        _validate_font(_read_source(game, f"Text/Fonts/CE/Set2/{size}.tbl"))
        contents[f"Maps/HD2_CREDITS_CE_{size}.TGA"] = (source, data)
    # Immutable outputs can be reused only with identical bytes. Preflight both
    # before creating either; never replace an existing file or user modification.
    for relative, (_, data) in contents.items():
        target = run / relative
        _check_path(target, file=True)
        if target.exists() and target.read_bytes() != data:
            raise ValueError(f"Existing Credits resource differs: {target}")
    for relative, (_, data) in contents.items():
        target = run / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if not target.exists():
            temporary = None
            try:
                with tempfile.NamedTemporaryFile(dir=target.parent, prefix=".hd2-credits-", delete=False) as stream:
                    temporary = Path(stream.name)
                    stream.write(data)
                os.link(temporary, target)  # Atomic no-overwrite publication.
            finally:
                if temporary is not None:
                    temporary.unlink(missing_ok=True)
    return {"assets": [{"path": relative, "source": source, "size": len(data),
                        "sha256": hashlib.sha256(data).hexdigest()}
                       for relative, (source, data) in contents.items()]}
