#!/usr/bin/env python3
"""Verify historical resolver capture bytes, then call production native decoders.

Does not inspect an installation or resolve resources. No runtime/engine claim.
"""
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import subprocess


def analyze(root, validator):
    root = root.resolve(strict=True)
    fingerprint_bytes = (root / 'fingerprints.json').read_bytes()
    fingerprints = json.loads(fingerprint_bytes)
    verified = {}
    # Verify every fingerprint, including the completion manifest, before parsing.
    for name, expected in fingerprints.items():
        if Path(name).name != name or name in ('.', '..'):
            raise ValueError('Unsafe capture member name')
        path = root / name
        if path.is_symlink():
            raise ValueError('Capture member is a symlink')
        data = path.read_bytes()
        digest = hashlib.sha256(data).hexdigest()
        if len(data) != expected['bytes'] or digest != expected['sha256']:
            raise ValueError(f'Capture fingerprint mismatch: {name}')
        verified[name] = {'bytes': len(data), 'sha256': digest}
    if 'manifest.tsv' not in verified:
        raise ValueError('Completion manifest has no fingerprint')
    rows = list(csv.DictReader(io.StringIO((root / 'manifest.tsv').read_text()), delimiter='\t'))
    result = {'schema': 1, 'evidence_level': 'historical resolver bytes and static requirements',
              'capture': str(root), 'fingerprints_sha256': hashlib.sha256(fingerprint_bytes).hexdigest(),
              'manifest': verified['manifest.tsv'], 'loaded_populations': None, 'executed_branches': None,
              'current_installation': None, 'context': None, 'resources': [], 'validation_failed': False}
    if 'context.tsv' in verified:
        result['context'] = list(csv.reader(io.StringIO((root / 'context.tsv').read_text()), delimiter='\t'))
    seen = set()
    for row in rows:
        resource, name = row['resource'], row['file']
        if resource in seen:
            raise ValueError('Duplicate logical resource in completion manifest')
        seen.add(resource)
        if row['bytes'] == 'missing':
            if name != '-':
                raise ValueError('Invalid captured absence')
            result['resources'].append({'resource': resource, 'outcome': 'absent in historical capture',
                                        'requiredness': 'loader policy, not inferred from absence'})
            continue
        if name not in verified or str(verified[name]['bytes']) != row['bytes']:
            raise ValueError('Manifest member has no matching verified fingerprint')
        entry = {'resource': resource, 'file': name, **verified[name],
                 'origin': 'unavailable: selected Loose/Patch/language/archive not recorded by resolver API'}
        member = resource.replace('\\', '/').rsplit('/', 1)[-1].lower()
        kind = {'scene2.bin': 'scene', 'actors.bin': 'scene', 'sounds.bin': 'scene',
                'check2.bin': 'tables', 'anim.sav': 'animation', 'gamedata00.gdt': 'gamedata',
                'scripts.dta': 'bindings', 'mpscripts.dta': 'bindings', 'items.dat': 'items', 'tracks.dat': 'tracks'}.get(member)
        if kind:
            run = subprocess.run([str(validator), kind, str(root / name), entry['sha256']],
                                 text=True, capture_output=True, check=False)
            entry['analysis'] = {'format': kind, 'accepted_in_declared_parser_scope': run.returncode == 0,
                                 'requirements': list(csv.DictReader(io.StringIO(run.stdout), delimiter='\t')),
                                 'failure': run.stderr.strip() or None}
            result['validation_failed'] |= run.returncode != 0
        else:
            entry['analysis'] = {'accepted_in_declared_parser_scope': None,
                                 'scope': 'bytes verified; concrete engine/resource decoder not invoked'}
        result['resources'].append(entry)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capture', type=Path)
    parser.add_argument('--validator', type=Path, required=True)
    args = parser.parse_args()
    try:
        report = analyze(args.capture, args.validator.resolve(strict=True))
        print(json.dumps(report, indent=2))
        return int(report['validation_failed'])
    except (OSError, ValueError, KeyError, csv.Error) as exc:
        parser.exit(2, f'{exc}\n')


if __name__ == '__main__':
    raise SystemExit(main())
