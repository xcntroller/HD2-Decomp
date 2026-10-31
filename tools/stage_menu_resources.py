"""Stage baseline menu resources, optionally including original confirmation art.

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
SOUNDS_KEY = struct.pack("<II", 0x4FE85106, 0x8D2965CA)
CLICK_SOUND_MEMBER = r"SOUNDS\button.wav"
MODEL_MEMBER = r"MODELS\main_menu.4ds"
CONFIRM_MEMBER = r"MODELS\confirm.4ds"
CURSOR_MODEL_MEMBER = r"MODELS\kurzor_menu.4ds"
MODEL_OUTPUTS = {
    "Models/main_menu.4ds": MODEL_MEMBER,
    "Models/confirm.4ds": CONFIRM_MEMBER,
    "Models/kurzor_menu.4ds": CURSOR_MODEL_MEMBER,
    "Models/single mission 2.4ds": r"MODELS\single mission 2.4ds",
    "Models/change_profile.4ds": r"MODELS\change_profile.4ds",
    "Models/options.4ds": r"MODELS\options.4ds",
}
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
            valid_model = (isinstance(relative, str) and relative in MODEL_OUTPUTS
                           and asset["archive"] == "models.dta"
                           and asset["member"] == MODEL_OUTPUTS[relative])
            valid_texture = (
                isinstance(relative, str) and relative.startswith("Maps/")
                and relative == "Maps/" + _texture_name(relative[5:].encode("ascii"))
                and asset["archive"] == "Maps.dta"
                and asset["member"].casefold() == ("MAPS\\" + relative[5:]).casefold()
            )
            valid_sound = (relative == "Sounds/button.wav" and asset["archive"] == "Sounds.dta"
                           and asset["member"] == CLICK_SOUND_MEMBER)
            if not (valid_model or valid_texture or valid_sound) or relative in owned:
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


def stage_menu_resources(game: Path, run: Path, *, include_confirm: bool = False,
                         include_cursor_model: bool = False, include_click_sound: bool = False,
                         include_cinematics: bool = False, include_options: bool = False, include_profiles: bool = False) -> dict:
    """Preflight and stage an isolated, deterministic baseline menu resource slice.

    Existing unrelated files (including runtime symlinks) are left untouched.
    Only unchanged files identified by a previous provenance manifest may be
    overwritten. Targeted symlinks are rejected, even if they stay inside run.
    Confirmation, cursor-model and click-sound assets are opt-in and remain owned/staged on subsequent calls,
    including calls by the unchanged launcher without include_confirm.
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

    owned = _owned_outputs(run)
    models = ["Models/main_menu.4ds"]
    if include_confirm or "Models/confirm.4ds" in owned:
        models.append("Models/confirm.4ds")
    if include_cursor_model or "Models/kurzor_menu.4ds" in owned:
        models.append("Models/kurzor_menu.4ds")
    if include_cinematics or "Models/single mission 2.4ds" in owned:
        models.append("Models/single mission 2.4ds")
    if include_options or "Models/options.4ds" in owned:
        models.append("Models/options.4ds")
    if include_profiles or "Models/change_profile.4ds" in owned:
        models.append("Models/change_profile.4ds")
    assets = []
    contents = {}
    archives = []
    textures = set()

    def collect(archive: str, member, data: bytes, relative: str):
        assets.append({
            "archive": archive, "member": member.name, "path": relative,
            "sha256": hashlib.sha256(data).hexdigest(), "size": len(data),
        })
        contents[relative] = data

    with (game / "models.dta").open("rb") as stream:
        members = read_index(stream, MODEL_KEY)
        for relative in models:
            name = MODEL_OUTPUTS[relative]
            matches = [member for member in members if member.name == name]
            if len(matches) != 1:
                raise ValueError(f"Missing exact model member: {name}")
            data = extract_member(stream, matches[0], MODEL_KEY)
            collect("models.dta", matches[0], data, relative)
            _, records, _ = decode_model_table(data)
            textures.update(
                _texture_name(name) for record in records
                for name in (record.primary_name, record.secondary_name, record.extra_name)
                if name
            )
        archives.append({"path": "models.dta", "sha256": _sha256(stream)})
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

    if include_click_sound or "Sounds/button.wav" in owned:
        with (game / "Sounds.dta").open("rb") as stream:
            matches = [member for member in read_index(stream, SOUNDS_KEY)
                       if member.name == CLICK_SOUND_MEMBER]
            if len(matches) != 1:
                raise ValueError("Missing exact click-sound member: " + CLICK_SOUND_MEMBER)
            data = extract_member(stream, matches[0], SOUNDS_KEY)
            collect("Sounds.dta", matches[0], data, "Sounds/button.wav")
            assets[-1]["reason"] = "menu_click_sound"
            archives.append({"path": "Sounds.dta", "sha256": _sha256(stream)})

    manifest = {
        "format": FORMAT, "limitation": LIMITATION,
        "archives": archives, "assets": assets,
    }
    contents[MANIFEST] = (json.dumps(manifest, indent=2) + "\n").encode("utf-8")
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
    parser.add_argument("--include-confirm", action="store_true",
                        help="Also stage confirm.4ds and its textures; retain on later runs")
    parser.add_argument("--include-cursor-model", action="store_true",
                        help="Stage original kurzor_menu.4ds; retain ownership on later runs")
    parser.add_argument("--include-click-sound", action="store_true",
                        help="Decode/stage original button.wav; retain ownership on later runs")
    args = parser.parse_args()
    try:
        result = stage_menu_resources(args.game_dir, args.output,
                                      include_confirm=args.include_confirm,
                                      include_cursor_model=args.include_cursor_model,
                                      include_click_sound=args.include_click_sound)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Menu resource staging failed: {error}\n")
    print(f"Staged {len(result['assets'])} baseline menu assets -> {args.output}")


if __name__ == "__main__":
    main()
