"""Stage original ANSI/CE font sets for the compiled multilingual menu.

Original Text trees are copied by stage_startup. New atlas aliases are immutable
and do not replace the accepted preview's English Set2 resources.
"""
import hashlib
import os
from pathlib import Path
import struct
import tempfile
from stage_menu_text import _check_path, _read_source, _validate_font


BYTE_LANGUAGES = {"english", "englishus", "czech", "polish", "french", "german", "italian", "spanish"}


def installed_menu_languages(game: Path) -> list[str]:
    root = Path(game) / "Text"
    _check_path(root)
    result = []
    for folder in root.iterdir():
        if folder.name.casefold() not in BYTE_LANGUAGES:
            continue
        _check_path(folder)
        names = {p.name.casefold(): p for p in folder.iterdir()}
        if "fontconfig.hxml" in names or "texty.txt" not in names:
            continue
        _check_path(names["texty.txt"], file=True)
        if names["texty.txt"].stat().st_size:
            result.append(folder.name)
    return sorted(result, key=str.casefold)


def validate_dimensions(width: int, height: int) -> None:
    if not 320 <= width <= 16384 or not 200 <= height <= 16384:
        raise ValueError("Render dimensions must be between 320x200 and 16384x16384")


def validate_font_atlas(data: bytes) -> None:
    if len(data) < 18:
        raise ValueError("Truncated font atlas")
    ident, cmap, kind, start, count, bits, x, y, width, height, depth, flags = struct.unpack("<BBBHHBHHHHBB", data[:18])
    if (ident or cmap or kind != 2 or start or count or bits or x or y or depth != 32 or flags not in (0, 8)
            or width not in (256, 512, 1024) or height not in (256, 512, 1024)):
        raise ValueError("Unsupported original font atlas header")
    end = 18 + width * height * 4
    if len(data) == end:
        return
    # The original 512px atlases omit the footer signature's final NUL.
    signature = 18 if data.endswith(b"TRUEVISION-XFILE.\0") else 17 if data.endswith(b"TRUEVISION-XFILE.") else 0
    footer = 8 + signature
    if not signature or len(data) < end + footer:
        raise ValueError("Truncated or malformed original font atlas payload")
    extension, developer = struct.unpack("<II", data[-footer:-signature])
    if any(offset and not end <= offset < len(data) - footer for offset in (extension, developer)):
        raise ValueError("Font atlas metadata points outside the trailing metadata area")


def stage_menu_presentation(game: Path, run: Path) -> dict:
    game, run = Path(game).absolute(), Path(run).absolute()
    _check_path(game)
    _check_path(run)
    game, run = game.resolve(strict=True), run.resolve(strict=True)
    if game == run or game in run.parents or run in game.parents:
        raise ValueError("Game and presentation output directories must not overlap")
    languages = installed_menu_languages(game)
    outputs = {}
    for family in ("ANSI", "CE"):
        for font_set in (1, 2, 3):
            for size in ("BIG", "SMALL"):
                source = f"Text/Fonts/{family}/Set{font_set}/{size}"
                _validate_font(_read_source(game, source + ".tbl"))
                data = _read_source(game, source + ".tga")
                validate_font_atlas(data)
                relative = f"Maps/HD2_FONT_{family}_SET{font_set}_{size}.TGA"
                outputs[relative] = (source + ".tga", data)
    # Validate every output before writing; edits to staged immutable assets are
    # never overwritten. Atomic no-overwrite publication also handles races.
    for relative, (_, data) in outputs.items():
        target = run / relative
        _check_path(target, file=True)
        if target.exists() and target.read_bytes() != data:
            raise ValueError(f"Existing presentation resource differs: {target}")
    for relative, (_, data) in outputs.items():
        target = run / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists():
            continue
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=target.parent, prefix=".hd2-font-", delete=False) as stream:
                temporary = Path(stream.name)
                stream.write(data)
            os.link(temporary, target)
        finally:
            if temporary is not None:
                temporary.unlink(missing_ok=True)
    return {"languages": languages, "assets": [
        {"path": relative, "source": source, "sha256": hashlib.sha256(data).hexdigest()}
        for relative, (source, data) in outputs.items()]}
