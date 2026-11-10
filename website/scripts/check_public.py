#!/usr/bin/env python3
"""Fail-closed audit of Vite's output, before Pages packages any files."""
from pathlib import Path
import re
import sys

sys.dont_write_bytecode = True
from validate_snapshot import load_snapshot, require, validate_snapshot

ASSET = re.compile(r'assets/[\w-]+-[\w-]{8,}\.(?:js|css|woff2?)\Z')
MACHINE = re.compile(r'(?<![\w:/])/(?:home|tmp|Users|workspace|github/workspace|opt|mnt|usr|var|root|etc|private)/|file://|(?<!\w)[A-Za-z]:[\\/](?!/)')


def audit(root):
    require(root.is_dir() and not root.is_symlink(), 'Missing or symlinked static output')
    paths = list(root.rglob('*'))
    for path in paths:
        require(not path.is_symlink(), 'Symlinks are forbidden in the Pages artifact')
        require(path.is_dir() or (path.is_file() and path.stat().st_nlink == 1),
                'Non-regular files and hard links are forbidden in the Pages artifact')
        if path.is_dir():
            require(path.relative_to(root).as_posix() in {'assets', 'data', '.vite'},
                    'Unexpected directory in the Pages artifact')

    manifest = load_snapshot(root / '.vite/manifest.json')
    require(isinstance(manifest, dict) and manifest, 'Missing/invalid Vite asset manifest')
    assets = set()
    for entry in manifest.values():
        require(isinstance(entry, dict), 'Invalid Vite manifest entry')
        for name in [entry.get('file'), *entry.get('css', []), *entry.get('assets', [])]:
            require(isinstance(name, str) and ASSET.fullmatch(name), 'Unexpected Vite asset path/type')
            assets.add(name)
    allowed = assets | {'index.html', 'font-licenses.txt', 'data/progress.json', 'data/research.json', '.vite/manifest.json'}
    actual = {p.relative_to(root).as_posix() for p in paths if p.is_file()}
    require(actual == allowed, 'Artifact has missing or undeclared files (only Vite assets, snapshot and licenses allowed)')
    for name in sorted(actual):
        path = root / name
        if path.suffix in {'.woff', '.woff2'}:
            require(path.name.startswith(('barlow-condensed-', 'ibm-plex-mono-')),
                    'Only the licensed dashboard fonts may be published')
            magic = b'wOF2' if path.suffix == '.woff2' else b'wOFF'
            with path.open('rb') as stream:
                require(stream.read(4) == magic, 'Invalid font asset')
            continue
        require(not MACHINE.search(path.read_text(encoding='utf-8')),
                'Private machine path in public output')
    from research_adapter import build_research
    require(load_snapshot(root / 'data/research.json') == build_research(), 'Stale research snapshot')
    index = (root / 'index.html').read_text(encoding='utf-8')
    require('/HD2-Decomp/assets/' in index and '/src/' not in index, 'Incorrect Pages base or unbuilt index.html')
    validate_snapshot(load_snapshot(root / 'data/progress.json'))
    print(f'Public artifact audit PASS: {len(actual)} declared static files; current validated '
          'snapshot; no symlinks, raw exports, logs, commercial assets or machine paths.')


def main():
    try:
        audit(Path(sys.argv[1] if len(sys.argv) > 1 else 'dist'))
        return 0
    except (ValueError, KeyError, TypeError, OSError) as error:
        print(f'Public artifact audit failed: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
