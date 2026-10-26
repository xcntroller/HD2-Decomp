"""Stage original loose English menu labels and ANSI Set2 atlases (800x600 only).

TEXTY.txt has CRLF lines, semicolon comments and decimal-ID/quoted-text records.
Unrelated text includes legacy single-byte characters and multiline records.
This deliberately parses only the static, single-line ASCII main-menu slice.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import stat
import struct


LABEL_IDS = (2101, 2102, 2103, 2104, 2105, 2106, 2107,
             2109, 2110, 2111, 2112, 2115, 2116)
MANIFEST = "menu-text-resources.json"
FORMAT = "hd2-menu-text-resources-v1"
LIMITATION = "Original loose English static main-menu slice; ANSI Set2 for 800x600 only."
TEXT_SOURCE = "Text/english/TEXTY.txt"
SOURCES = {
    "Maps/HD2_MENU_BIG.TGA": "Text/Fonts/ANSI/Set2/BIG.tga",
    "Maps/HD2_MENU_SMALL.TGA": "Text/Fonts/ANSI/Set2/SMALL.tga",
    "MenuText/BIG.tbl": "Text/Fonts/ANSI/Set2/BIG.tbl",
    "MenuText/SMALL.tbl": "Text/Fonts/ANSI/Set2/SMALL.tbl",
    "MenuText/labels.bin": TEXT_SOURCE,
}


def parse_menu_labels(data: bytes) -> dict[int, str]:
    """Read only selected decimal-ID quoted records, without transcoding labels."""
    if not data or len(data) > 4 * 1024 * 1024 or b"\0" in data:
        raise ValueError("Invalid TEXTY.txt size or embedded NUL")
    labels = {}
    quoted = False
    comment_after_quote = re.compile(rb"[ \t]*;")

    def scan_quotes(payload: bytes, inside: bool) -> bool:
        # Original prose contains unescaped paired quotes, including within
        # multiline records. Keep parity across lines; never resync on an ID.
        last_quote = -1
        for index, value in enumerate(payload):
            if value != ord('"'):
                continue
            inside = not inside
            last_quote = index
            if not inside and comment_after_quote.match(payload, index + 1):
                return False
        if not inside and payload[last_quote + 1:].strip(b" \t"):
            raise ValueError("Malformed localization record suffix")
        return inside

    for line in data.splitlines():
        if quoted:
            quoted = scan_quotes(line, True)
            continue
        match = re.match(rb"^[ \t]*(\d+)(?=[ \t]|$)(.*)$", line)
        if not match:
            continue
        # Bound conversion even for hostile, arbitrarily long decimal tokens.
        if len(match[1]) > 10:
            raise ValueError("Localization ID exceeds uint32")
        label_id = int(match[1])
        if label_id > 0xffffffff:
            raise ValueError("Localization ID exceeds uint32")
        if label_id not in LABEL_IDS:
            payload = match[2].lstrip(b" \t")
            # The original file also contains bare numeric section markers.
            if payload and not payload.startswith(b";"):
                if not payload.startswith(b'"'):
                    raise ValueError(f"Malformed localization record: {label_id}")
                quoted = scan_quotes(payload, False)
            continue
        record = re.fullmatch(rb'[ \t]*\d+[ \t]+"([^"\r\n]*)"[ \t]*(?:;[^\r\n]*)?', line)
        if not record or not 1 <= len(record[1]) <= 4096:
            raise ValueError(f"Malformed static menu label: {label_id}")
        if label_id in labels:
            raise ValueError(f"Duplicate menu label ID: {label_id}")
        if any(value < 0x20 or value > 0x7e for value in record[1]):
            raise ValueError(f"Menu label is not printable ASCII: {label_id}")
        labels[label_id] = record[1].decode("ascii")
    if quoted:
        raise ValueError("Unterminated multiline localization record")
    if set(labels) != set(LABEL_IDS):
        raise ValueError(f"Missing static menu labels: {sorted(set(LABEL_IDS) - labels.keys())}")
    return {label_id: labels[label_id] for label_id in LABEL_IDS}


def encode_menu_labels(labels: dict[int, str]) -> bytes:
    output = bytearray(b"H2MT" + struct.pack("<I", len(labels)))
    for label_id, label in sorted(labels.items()):
        raw = label.encode("ascii")
        output += struct.pack("<II", label_id, len(raw)) + raw
    return bytes(output)


def _check_path(path: Path, *, file: bool = False) -> None:
    for current in [*reversed(path.parents), path]:
        if current.is_symlink():
            raise ValueError(f"Symlink is not allowed: {current}")
        if current.exists():
            info = current.stat()
            if current == path and file:
                if not stat.S_ISREG(info.st_mode) or info.st_nlink != 1:
                    raise ValueError(f"Expected unlinked regular file: {current}")
            elif not stat.S_ISDIR(info.st_mode):
                raise ValueError(f"Expected directory: {current}")


def _read_source(game: Path, relative: str) -> bytes:
    source = game / relative
    _check_path(source, file=True)
    if source.stat().st_size > 4 * 1024 * 1024:
        raise ValueError(f"Source exceeds slice size limit: {relative}")
    return source.read_bytes()


def _validate_font(data: bytes) -> None:
    if len(data) != 5120:
        raise ValueError("Font TBL must contain exactly 5120 bytes")
    for left, top, right, bottom, aspect in struct.iter_unpack("<5f", data):
        if (not all(math.isfinite(v) for v in (left, top, right, bottom, aspect))
                or not 0 <= left <= right <= 1 or not 0 <= top <= bottom <= 1
                or not 0 <= aspect <= 16):
            raise ValueError("Invalid font metric")


def _validate_atlas(data: bytes) -> None:
    if len(data) < 18:
        raise ValueError("Truncated TGA atlas")
    ident, cmap, kind, start, count, bits, x, y, width, height, depth, flags = \
        struct.unpack("<BBBHHBHHHHBB", data[:18])
    if (ident or cmap or kind != 2 or start or count or bits or x or y
            or width != 256 or height != 256 or depth != 32 or flags != 8):
        raise ValueError("Expected original Set2 256x256 32-bit TGA atlas")
    end = 18 + width * height * 4
    # Original files include the standard 26-byte TGA 2.0 footer.
    if len(data) != end and not (
            len(data) == end + 26 and data[-18:] == b"TRUEVISION-XFILE.\0"):
        raise ValueError("Unexpected TGA atlas payload or footer")


def _owned_outputs(run: Path) -> set[str]:
    path = run / MANIFEST
    _check_path(path, file=True)
    if not path.exists():
        return set()
    if path.stat().st_size > 64 * 1024:
        raise ValueError("Oversized menu text manifest")
    try:
        previous = json.loads(path.read_text(encoding="utf-8"))
        if previous["format"] != FORMAT:
            raise ValueError("Unknown menu text manifest format")
        assets = previous["assets"]
        if not isinstance(assets, list) or len(assets) != len(SOURCES):
            raise ValueError("Incomplete menu text manifest")
        owned = set()
        for asset in assets:
            relative = asset["path"]
            if (relative not in SOURCES or relative in owned
                    or asset["source"] != SOURCES[relative]):
                raise ValueError("Invalid menu text manifest asset")
            target = run / relative
            _check_path(target, file=True)
            if target.exists():
                if (target.stat().st_size > 4 * 1024 * 1024
                        or target.stat().st_size != asset["size"]
                        or hashlib.sha256(target.read_bytes()).hexdigest() != asset["sha256"]):
                    raise ValueError(f"Previously staged file was modified: {relative}")
            owned.add(relative)
        return owned | {MANIFEST}
    except (KeyError, TypeError, UnicodeError, json.JSONDecodeError) as error:
        raise ValueError("Invalid menu text manifest") from error


def stage_menu_text(game: Path, run: Path) -> dict:
    """Preflight sources/owned destinations, then stage a separate provenance slice."""
    game = Path(game).absolute()
    run = Path(run).absolute()
    # Check the lexical paths before resolving: resolving alone hides symlinks.
    _check_path(game)
    _check_path(run)
    game = game.resolve(strict=True)
    run = run.resolve()
    if game == run or game in run.parents or run in game.parents:
        raise ValueError("Game and output directories must not overlap")
    source_data = {relative: _read_source(game, relative) for relative in sorted(set(SOURCES.values()))}
    labels = parse_menu_labels(source_data[TEXT_SOURCE])
    for relative, data in source_data.items():
        if relative.endswith(".tbl"):
            _validate_font(data)
        elif relative.endswith(".tga"):
            _validate_atlas(data)
    contents = {
        relative: encode_menu_labels(labels) if relative.endswith("labels.bin") else source_data[source]
        for relative, source in SOURCES.items()
    }
    manifest = {
        "format": FORMAT,
        "limitation": LIMITATION,
        "sources": [{"path": path, "size": len(data),
                     "sha256": hashlib.sha256(data).hexdigest()}
                    for path, data in sorted(source_data.items())],
        "assets": [{"path": path, "source": SOURCES[path], "size": len(data),
                    "sha256": hashlib.sha256(data).hexdigest()}
                   for path, data in contents.items()],
    }
    contents[MANIFEST] = (json.dumps(manifest, indent=2) + "\n").encode("utf-8")
    owned = _owned_outputs(run)
    for relative in contents:
        target = run / relative
        _check_path(target, file=True)
        if target.exists() and relative not in owned:
            raise ValueError(f"Refusing to overwrite unrelated file: {relative}")
    for relative, data in contents.items():
        target = run / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    try:
        result = stage_menu_text(args.game_dir, args.output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Menu text staging failed: {error}\n")
    print(f"Staged {len(result['assets'])} menu text assets -> {args.output}")


if __name__ == "__main__":
    main()
