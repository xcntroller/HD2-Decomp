#!/usr/bin/env python3
"""Reject stale, malformed or promoted progress claims before Vite runs.

The public schema is the exact allowlisted projection, compared with strict JSON
types. Independent checks retain the analyzer's ledger and graph claim levels.
Only checked-in text metadata and Git history are needed.
"""
import json
import os
from pathlib import Path
import re
import sys

sys.dont_write_bytecode = True
from export_public import ABSOLUTE, PRIVATE, ROOT, SITE, analyze, build_snapshot
from function_progress import unique_object
from progress_analyzer import reject_constant

URL = re.compile(r'https://github\.com/xcntroller/HD2-Decomp/(?:commit/[0-9a-f]{40}|blob/[0-9a-f]{40}/(?:[\w.-]+/)*[\w.-]+(?:#L[1-9][0-9]*)?)\Z')
POINTER = re.compile(r'(?:/(?:[^~/]|~[01])*)*\Z')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load_snapshot(path):
    return json.loads(Path(path).read_text(encoding='utf-8'),
                      object_pairs_hook=unique_object, parse_constant=reject_constant)


def canonical(value):
    # Unlike Python ==, this distinguishes false from 0 and 1 from 1.0.
    return json.dumps(value, sort_keys=True, separators=(',', ':'), allow_nan=False)


def validate_content(value, key=''):
    if isinstance(value, dict):
        for name, item in value.items():
            validate_content(item, name)
    elif isinstance(value, list):
        for item in value:
            validate_content(item, key)
    elif isinstance(value, str):
        if key == 'pointer':
            require(POINTER.fullmatch(value), 'Invalid evidence JSON pointer')
        else:
            require(not PRIVATE.search(value) and not ABSOLUTE.search(value) and 'file://' not in value,
                    'Private artifact reference or absolute machine path in public data')
        if key == 'url':
            require(URL.fullmatch(value) and '..' not in value.split('/'),
                    'Unsafe public source/commit URL')


def validate_public_directory(directory=SITE / 'public'):
    """Prevent Vite's public-directory copying from including unexpected files."""
    allowed = {'data', 'data/progress.json', 'font-licenses.txt'}
    require(directory.is_dir() and not directory.is_symlink(), 'Missing or symlinked public directory')
    for path in directory.rglob('*'):
        name = path.relative_to(directory).as_posix()
        require(name in allowed and not path.is_symlink(), 'Unexpected file or symlink in public directory')
        require(path.is_dir() or (path.is_file() and path.stat().st_nlink == 1),
                'Non-regular file or hard link in public directory')
    require((directory / 'data/progress.json').is_file()
            and (directory / 'font-licenses.txt').is_file(), 'Public snapshot or font licenses missing')


def validate_snapshot(document):
    require(isinstance(document, dict) and type(document.get('schema_version')) is int
            and document['schema_version'] == 1, 'Unsupported public snapshot schema')
    validate_content(document)
    raw = analyze(ROOT)
    expected = build_snapshot(raw)
    require(canonical(document) == canonical(expected),
            'Snapshot differs from the current safe schema/projection; run npm run data')

    # Independent claim checks guard against an accidental promotion in the adapter.
    require(document['counts'] == raw['functions']['totals']
            and document['by_program'] == raw['functions']['by_program'],
            'Published counts disagree with the frozen baseline/current ledger')
    require({(f['program'], f['entry'], f['status']) for f in document['functions']}
            == {(f['program'], f['entry'], f['status']) for f in raw['functions']['entries']},
            'Published function identities/statuses disagree with the ledger')
    graph = document['graph']
    source_nodes = {n['id']: n for n in raw['graph']['nodes']}
    nodes = {n['id']: n for n in graph['nodes']}
    require(len(nodes) == len(graph['nodes']) and nodes.keys() == source_nodes.keys(),
            'Graph node identities are duplicated or changed')
    for identity, node in nodes.items():
        source = source_nodes[identity]
        require(node['type'] == source['type'] and node['data']['status'] == source['data']['status'],
                'Graph reconstruction status was promoted or changed')
        require({stage: v['status'] for stage, v in node['data']['verification_levels'].items()}
                == {stage: v['status'] for stage, v in source['data']['verification_levels'].items()},
                'Graph verification level was promoted or changed')
    source_edges = {e['id']: e for e in raw['graph']['edges']}
    require(len({e['id'] for e in graph['edges']}) == len(graph['edges'])
            and {e['id'] for e in graph['edges']} == source_edges.keys(), 'Graph edge identities changed')
    for edge in graph['edges']:
        source = source_edges[edge['id']]
        require(edge['source'] in nodes and edge['target'] in nodes, 'Graph edge has an absent endpoint')
        require((edge['source'], edge['target'], edge['type'])
                == (source['source'], source['target'], source['type'])
                and all(edge['data'][key] == source['data'][key]
                        for key in ('relationship', 'basis', 'evidence_state')),
                'Graph relationship direction or evidence strength was changed')
    require({m['id']: m['reported_status'] for m in document['milestones']}
            == {m['id']: m['reported_status'] for m in raw['milestones']['items']},
            'Milestone completion changed from repository evidence')
    for area in document['areas']:
        require(set(area) == {'id', 'name', 'status', 'milestone', 'evidence'},
                'Unexpected project-area completion metric')
    if os.environ.get('GITHUB_ACTIONS') == 'true':
        sha = os.environ.get('GITHUB_SHA')
        require(sha and document['snapshot']['source_commit'] == sha
                and document['snapshot']['published_commit'] == sha,
                'CI snapshot/source links must identify the exact GitHub workflow SHA')
    return document


def main():
    try:
        validate_public_directory()
        document = validate_snapshot(load_snapshot(sys.argv[1] if len(sys.argv) > 1
                                                  else SITE / 'public/data/progress.json'))
        notices = sum(document['diagnostics'].values())
        print(f'Snapshot validation PASS: schema, current inputs, ledger counts, statuses and '
              f'graph provenance; {notices} documentary notices preserved without promotion.')
        return 0
    except (ValueError, KeyError, TypeError, OSError) as error:
        print(f'Snapshot validation failed: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
