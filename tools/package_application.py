"""Build an isolated portable Windows front-end folder and ZIP from local assets.

Reads original PE imports and resources; never executes Windows code. Existing
outputs are refused. Profiles, config, keys, logs and compatibility prefixes are
not imported. Run from the repository; the resulting folder needs no Python.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile
import zipfile

from run_startup_probe import stage_startup, digest
from stage_menu_text import _check_path
from stage_opening_movies import stage_opening_movies
from stage_menu_presentation import stage_menu_presentation, installed_menu_languages
from stage_cinematics_movies import stage_cinematics_movies
from validate_engine_model import PEImage, ENGINE_SHA256
from validate_joystick_devices import JOY_SHA256
from validate_sound_codec import RW_DATA_SHA256


CORE_DLLS = {"ls3df.dll": ENGINE_SHA256, "rw_data.dll": RW_DATA_SHA256, "ijoy.dll": JOY_SHA256}
SYSTEM_DLLS = {
    "kernel32.dll", "user32.dll", "gdi32.dll", "ole32.dll", "oleaut32.dll",
    "advapi32.dll", "shell32.dll", "comdlg32.dll", "comctl32.dll", "version.dll",
    "winmm.dll", "ws2_32.dll", "msvcrt.dll", "ucrtbase.dll", "d3d8.dll",
    "dinput.dll", "dinput8.dll", "dsound.dll", "imm32.dll", "ntdll.dll",
}


def pe_imports(data: bytes) -> list[str]:
    """Read import DLL names from file-backed PE32 sections, without loading it."""
    image = PEImage(data)
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    if struct.unpack_from("<H", data, pe + 20)[0] < 112:
        raise ValueError("PE32 import directory is missing")
    optional = pe + 24
    rva, size = struct.unpack_from("<II", data, optional + 104)
    if not rva and not size:
        return []
    if not rva or size < 20 or size > 20 * 4096:
        raise ValueError("Invalid PE32 import directory")
    names = []
    for offset in range(0, size - 19, 20):
        descriptor = image.read_va(image.image_base + rva + offset, 20)
        if descriptor == bytes(20):
            return names
        name_rva = struct.unpack_from("<I", descriptor, 12)[0]
        raw = bytearray()
        for index in range(260):
            byte = image.read_va(image.image_base + name_rva + index, 1)[0]
            if not byte:
                break
            raw.append(byte)
        else:
            raise ValueError("Unterminated imported DLL name")
        name = raw.decode("ascii")
        if not re.fullmatch(r"[A-Za-z0-9_.-]+\.[dD][lL][lL]", name) or ".." in name:
            raise ValueError("Unsafe imported DLL name")
        names.append(name)
    raise ValueError("PE32 import directory has no terminator")


def dependency_closure(game: Path, executable: Path) -> tuple[list[dict], list[str]]:
    local = {}
    for path in game.iterdir():
        if path.suffix.casefold() != ".dll":
            continue
        key = path.name.casefold()
        if key in local:
            raise ValueError(f"Ambiguous installed Windows DLL name: {path.name}")
        local[key] = path
    pending = list(CORE_DLLS)
    system = {}
    records = {}

    def imports(data):
        result = pe_imports(data)
        for name in result:
            key = name.casefold()
            if key in SYSTEM_DLLS or key.startswith(("api-ms-", "ext-ms-")):
                system.setdefault(key, name)
            elif key in local:
                pending.append(key)
            else:
                raise ValueError(f"Missing imported dependency: {name}")
        return result

    imports(executable.read_bytes())
    while pending:
        key = pending.pop()
        if key in records:
            continue
        source = local.get(key)
        if source is None:
            raise ValueError(f"Missing original dependency: {key}")
        _check_path(source, file=True)
        data = source.read_bytes()
        fingerprint = hashlib.sha256(data).hexdigest()
        if key in CORE_DLLS and fingerprint != CORE_DLLS[key]:
            raise ValueError(f"Unsupported original dependency fingerprint: {source.name}")
        records[key] = {"path": source.name, "sha256": fingerprint, "bytes": len(data),
                        "imports": imports(data)}
    return sorted(records.values(), key=lambda row: row["path"].casefold()), sorted(system.values(), key=str.casefold)


def package_application(game: Path, executable: Path, output: Path,
                        readme: Path, *, make_zip: bool = True) -> dict:
    game, executable, output, readme = (Path(p).absolute() for p in (game, executable, output, readme))
    _check_path(game)
    _check_path(executable, file=True)
    _check_path(readme, file=True)
    _check_path(output.parent)
    if output.exists() or output.is_symlink():
        raise ValueError("Package output already exists; choose a new directory to preserve user files")
    archive = output.with_suffix(output.suffix + ".zip")
    if make_zip and (archive.exists() or archive.is_symlink()):
        raise ValueError("Package archive already exists; choose a new output")
    if game.resolve().is_relative_to(output.resolve()) or output.resolve().is_relative_to(game.resolve()):
        raise ValueError("Package output must be separate from the original installation")
    if executable.resolve().is_relative_to(output.resolve()):
        raise ValueError("Input executable must be outside the package output")
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_. -]*", output.name) or output.name.endswith((".", " ")):
        raise ValueError("Package directory needs a Windows-compatible name")
    if output.name.split(".")[0].upper() in {"CON", "PRN", "AUX", "NUL",
                                            *(f"COM{i}" for i in range(1, 10)),
                                            *(f"LPT{i}" for i in range(1, 10))}:
        raise ValueError("Package directory uses a reserved Windows device name")
    dlls, system = dependency_closure(game, executable)
    languages = installed_menu_languages(game)
    # Fresh private assembly: reuse real staging, omit installed user state.
    # Existing development workspaces and writable copies are never touched.
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".hd2-package-", dir=output.parent) as temporary:
        work = Path(temporary)
        staged = stage_startup(game, executable, work, application_launch=True, seed_user_state=False)
        run = staged.parent
        stage_opening_movies(game, run)
        presentation = stage_menu_presentation(game, run, aliases=False)
        media = stage_cinematics_movies(game, run)
        staged.rename(run / "HD2FrontEnd.exe")
        for record in dlls:
            source, target = game / record["path"], run / record["path"]
            shutil.copyfile(source, target)
            if digest(target).hex() != record["sha256"]:
                raise ValueError(f"Dependency copy failed: {record['path']}")
        shutil.copyfile(readme, run / "README.md")
        (run / "Launch.cmd").write_bytes(
            b'@echo off\r\npushd "%~dp0"\r\nif errorlevel 1 exit /b 1\r\n'
            b'"%~dp0HD2FrontEnd.exe" %*\r\nset "HD2_LAUNCH_RESULT=%ERRORLEVEL%"\r\n'
            b'popd\r\nexit /b %HD2_LAUNCH_RESULT%\r\n')
        (run / "LaunchFullscreen.cmd").write_bytes(
            b'@echo off\r\ncall "%~dp0Launch.cmd" --fullscreen %*\r\nexit /b %ERRORLEVEL%\r\n')
        files = []
        windows_names = set()
        for path in sorted(run.rglob("*")):
            relative = path.relative_to(run).as_posix()
            if relative.split("/")[0].casefold() in {"models", "sounds", "menutext", "maps"}:
                raise ValueError("Package unexpectedly contains development resource extracts or aliases")
            if relative.casefold() in windows_names:
                raise ValueError(f"Ambiguous packaged Windows path: {relative}")
            windows_names.add(relative.casefold())
            if path.is_dir():
                continue
            _check_path(path, file=True)
            if relative.casefold().startswith("playersprofiles/") or path.suffix.casefold() == ".cfg":
                raise ValueError("Package unexpectedly contains user profile/configuration data")
            if path.suffix.casefold() == ".exe" and relative != "HD2FrontEnd.exe":
                raise ValueError("Package unexpectedly contains an additional executable")
            files.append({"path": relative, "bytes": path.stat().st_size, "sha256": digest(path).hex()})
        # Empty profiles are deliberate: original missing-game.cfg/empty-name
        # fallback leads to Main/Create Profile, not a fabricated seeded profile.
        manifest = {"format": "hd2-portable-frontend-v1", "entry": "HD2FrontEnd.exe",
                    "client_sha256": digest(run / "HD2FrontEnd.exe").hex(),
                    "original_game_executable_included": False,
                    "user_profiles_or_configuration_included": False,
                    "languages": languages, "dlls": dlls, "system_imports": system,
                    "resource_layout": "Original DTA archives and Text/Fonts paths; no development extracts or aliases",
                    "presentation": presentation,
                    "missing_optional_cinematics": media["missing"], "files": files,
                    "verification": "Static packaging verified; portable desktop launch pending user confirmation",
                    "limits": ["Full single-player completion and game/save receivers remain Phase2",
                               "Loadout screens completely untested until accessible",
                               "Ready-server UI and session services untested until Phase3",
                               "Broader save metadata/preview/compatibility recovery remains open"]}
        (run / "package-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        zip_path = work / "package.zip"
        if make_zip:
            with zipfile.ZipFile(zip_path, "w", allowZip64=True) as bundle:
                # Include empty directories, especially PlayersProfiles.
                for path in [run, *sorted(run.rglob("*"))]:
                    name = output.name + "/" + (path.relative_to(run).as_posix() if path != run else "")
                    if path.is_dir():
                        name = name.rstrip("/") + "/"
                    info = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
                    info.external_attr = (0o40755 if path.is_dir() else 0o100644) << 16
                    if path.is_dir():
                        bundle.writestr(info, b"")
                    else:
                        with path.open("rb") as source, bundle.open(info, "w", force_zip64=True) as target:
                            shutil.copyfileobj(source, target, 1024 * 1024)
        # Nothing replaces an existing nonempty package or its writable state.
        if output.exists() or output.is_symlink():
            raise ValueError("Package destination appeared during assembly")
        run.rename(output)
        if make_zip:
            os.link(zip_path, archive)  # Exclusive publication, never overwrite.
    return {"directory": str(output), "archive": str(archive) if make_zip else None,
            "client_sha256": manifest["client_sha256"], "files": len(files) + 1,
            "payload_bytes": sum(row["bytes"] for row in files), "dlls": [d["path"] for d in dlls]}


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", type=Path, default=root / "Hidden and Dangerous 2")
    parser.add_argument("--executable", type=Path, default=root / "recovered/build/application_launch.exe")
    parser.add_argument("--output", type=Path, default=root / "dist/HD2-Phase1")
    parser.add_argument("--no-zip", action="store_true", help="Create the portable folder only")
    args = parser.parse_args()
    try:
        result = package_application(args.game_dir, args.executable, args.output,
                                     root / "docs/STANDALONE.md", make_zip=not args.no_zip)
    except (OSError, ValueError, UnicodeError, struct.error, zipfile.BadZipFile) as error:
        parser.exit(1, f"Packaging failed: {error}\n")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
