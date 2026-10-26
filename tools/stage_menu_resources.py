"""Stage the baseline menu model, its textures, scene-default Snow, and cursor.

This is a resource slice, not a complete UI: patch/expansion overlays, scripts,
animation dependencies, and runtime behavior are not reconstructed.
"""

import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

from extract_dta_member import extract_member, read_index
from inspect_model_table import decode_model_table


MODEL_KEY = struct.pack("<II", 0x5D805259, 0x10ACB252)
MAPS_KEY = struct.pack("<II", 0xB438D0A5, 0xF26527FA)
MODEL_MEMBER = r"MODELS\main_menu.4ds"
CURSOR_TEXTURE = "X_KURSOR.TGA"
MANIFEST = "menu-resources.json"
FORMAT = "hd2-menu-resources-v1"
LIMITATION = (
    "Baseline resource slice only; no patch/expansion overlays, scripts, "
    "animation dependencies, or full UI/runtime reconstruction."
)


def _sha256(stream) -> str:
    stream.seek(0)
    digest = hashlib.sha256()
    for block in iter(lambda: stream.read(1024 * 1024), b""):
        digest.update(block)
    return digest.hexdigest()


def _texture_name(raw: bytes) -> str:
    try:
        name = raw.decode("ascii")
    except UnicodeDecodeError as error:
        raise ValueError(f"Non-ASCII texture name: {raw!r}") from error
    if (not re.fullmatch(r"[A-Za-z0-9_][A-Za-z0-9_.-]*", name)
            or ".." in name or name.endswith(".")
            or name.split(".")[0].upper() in {
                "CON", "PRN", "AUX", "NUL",
                *(f"COM{i}" for i in range(1, 10)),
                *(f"LPT{i}" for i in range(1, 10)),
            }):
        raise ValueError(f"Unsafe texture name: {name!r}")
    return name.upper()


def _check_output(run: Path, relative: str) -> Path:
    target = run / relative
    if run.is_symlink():
        raise ValueError(f"Output root is a symlink: {run}")
    current = run
    for part in Path(relative).parts:
        current = current / part
        if current.is_symlink():
            raise ValueError(f"Output path is a symlink: {current}")
        if current.exists() and current != target and not current.is_dir():
            raise ValueError(f"Output parent is not a directory: {current}")
    if target.exists() and (not target.is_file() or target.stat().st_nlink != 1):
        raise ValueError(f"Output is not an unlinked regular file: {target}")
    return target


def _owned_outputs(run: Path) -> set[str]:
    path = _check_output(run, MANIFEST)
    if not path.exists():
        return set()
    try:
        previous = json.loads(path.read_text(encoding="utf-8"))
        if previous["format"] != FORMAT:
            raise ValueError("Unknown manifest format")
        assets = previous["assets"]
        if not isinstance(assets, list) or not assets:
            raise ValueError("Missing manifest assets")
        owned = set()
        for asset in assets:
            relative = asset["path"]
            valid_model = (relative == "Models/main_menu.4ds"
                           and asset["archive"] == "models.dta"
                           and asset["member"] == MODEL_MEMBER)
            valid_texture = (
                isinstance(relative, str) and relative.startswith("Maps/")
                and relative == "Maps/" + _texture_name(relative[5:].encode("ascii"))
                and asset["archive"] == "Maps.dta"
                and asset["member"].casefold() == ("MAPS\\" + relative[5:]).casefold()
            )
            if not (valid_model or valid_texture) or relative in owned:
                raise ValueError("Invalid manifest asset")
            output = _check_output(run, relative)
            if output.exists():
                with output.open("rb") as stream:
                    digest = _sha256(stream)
                if output.stat().st_size != asset["size"] or digest != asset["sha256"]:
                    raise ValueError(f"Previously staged file was modified: {relative}")
            owned.add(relative)
        if "Models/main_menu.4ds" not in owned:
            raise ValueError("Missing manifest model")
        return owned | {MANIFEST}
    except (KeyError, TypeError, UnicodeError, AttributeError, ValueError) as error:
        raise ValueError(f"Invalid existing {MANIFEST}: {error}") from error


def stage_menu_resources(game: Path, run: Path) -> dict:
    """Preflight and stage an isolated, deterministic baseline menu resource slice.

    Existing unrelated files (including runtime symlinks) are left untouched.
    Only unchanged files identified by a previous provenance manifest may be
    overwritten. Targeted symlinks are rejected, even if they stay inside run.
    """
    game = Path(game).resolve(strict=True)
    requested_run = Path(run).absolute()
    if requested_run.is_symlink():
        raise ValueError("Output root is a symlink")
    run = requested_run.resolve()
    if game == run or game in run.parents or run in game.parents:
        raise ValueError("Game and output directories must be isolated in both directions")
    if not game.is_dir() or (run.exists() and not run.is_dir()):
        raise ValueError("Game and output paths must be directories")

    assets = []
    contents = {}
    archives = []

    def collect(archive: str, member, data: bytes, relative: str):
        assets.append({
            "archive": archive, "member": member.name, "path": relative,
            "sha256": hashlib.sha256(data).hexdigest(), "size": len(data),
        })
        contents[relative] = data

    with (game / "models.dta").open("rb") as stream:
        members = read_index(stream, MODEL_KEY)
        matches = [member for member in members if member.name == MODEL_MEMBER]
        if len(matches) != 1:
            raise ValueError(f"Missing exact model member: {MODEL_MEMBER}")
        data = extract_member(stream, matches[0], MODEL_KEY)
        archives.append({"path": "models.dta", "sha256": _sha256(stream)})
        collect("models.dta", matches[0], data, "Models/main_menu.4ds")
    _, records, _ = decode_model_table(data)
    textures = sorted({
        _texture_name(name) for record in records
        for name in (record.primary_name, record.secondary_name, record.extra_name)
        if name
    })
    with (game / "Maps.dta").open("rb") as stream:
        members = read_index(stream, MAPS_KEY)
        indexed = {member.name.casefold(): member for member in members}
        scene_default = next((
            indexed[candidate.casefold()]
            for candidate in (r"MAPS\Snow.dds", r"MAPS\Snow.tga")
            if candidate.casefold() in indexed
        ), None)
        if scene_default is None:
            raise ValueError("Missing scene-default texture: MAPS\\Snow.dds or MAPS\\Snow.tga")
        default_name = _texture_name(scene_default.name.split("\\")[-1].encode("ascii"))
        for name in sorted(set(textures) | {default_name, CURSOR_TEXTURE}):
            member = indexed.get(("MAPS\\" + name).casefold())
            if member is None:
                raise ValueError(f"Missing exact texture member: MAPS\\{name}")
            data = extract_member(stream, member, MAPS_KEY)
            collect("Maps.dta", member, data, "Maps/" + name)
            if name == default_name:
                assets[-1]["reason"] = "scene_default"
            if name == CURSOR_TEXTURE:
                assets[-1]["reason"] = "menu_cursor"
        archives.append({"path": "Maps.dta", "sha256": _sha256(stream)})

    manifest = {
        "format": FORMAT, "limitation": LIMITATION,
        "archives": archives, "assets": assets,
    }
    contents[MANIFEST] = (json.dumps(manifest, indent=2) + "\n").encode("utf-8")
    owned = _owned_outputs(run)
    for relative in contents:
        target = _check_output(run, relative)
        if target.exists() and relative not in owned:
            raise ValueError(f"Refusing to overwrite unrelated file: {target}")
    for relative, data in contents.items():
        target = _check_output(run, relative)
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open("wb") as stream:
            stream.write(data)
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = stage_menu_resources(args.game_dir, args.output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Menu resource staging failed: {error}\n")
    print(f"Staged {len(result['assets'])} baseline menu assets -> {args.output}")


if __name__ == "__main__":
    main()
