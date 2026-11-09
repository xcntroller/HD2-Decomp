#!/usr/bin/env python3
"""Allowlisted publication adapter. The read-only analyzer remains unchanged."""
from collections import Counter
import json
import os
from pathlib import Path
import re
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
SITE = ROOT / 'website'
sys.path.insert(0, str(ROOT / 'tools'))
from progress_analyzer import analyze

REPOSITORY = 'https://github.com/xcntroller/HD2-Decomp'
ALLOWED_SUFFIXES = {'.md', '.json', '.py', '.h', '.cpp', '.c', '.java'}
PRIVATE = re.compile(r'(?:analysis/(?:work|ghidra)/|(?<!\w)work/|\.tools/|extracted/|captures/|assets/)[^\s`\]\)<>"\']*')
ABSOLUTE = re.compile(r'(?<![\w:])/(?:home|tmp|workspace|Users|opt|mnt|usr|var)/[^\s`\]\)<>"\']*|(?<!\w)[A-Za-z]:[\\/](?!/)[^\s`\]\)<>"\']*')


def git(*args, required=True):
    result = subprocess.run(['git', '-C', str(ROOT), *args], capture_output=True, text=True)
    if required and result.returncode:
        raise ValueError('Required Git metadata unavailable: ' + args[0])
    return result.stdout.strip() if result.returncode == 0 else ''


def prose(value):
    text = str(value or '')
    text = re.sub(r'\[([^\]]+)\]\([^)]*\)', r'\1', text)
    text = PRIVATE.sub('[local artifact omitted]', text)
    text = ABSOLUTE.sub('[local path omitted]', text)
    return text


def report_text(value):
    if isinstance(value, dict):
        return {k: report_text(v) for k, v in value.items()
                if k in {'result', 'tools', 'flags', 'boundary', 'executed', 'tests', 'totals'}}
    if isinstance(value, list):
        return [report_text(v) for v in value]
    return prose(value) if isinstance(value, str) else value


class Publication:
    def __init__(self):
        self.head = git('rev-parse', 'HEAD')
        requested = os.environ.get('HD2_PUBLIC_REF')
        upstream = git('rev-parse', '--symbolic-full-name', '@{upstream}', required=False)
        self.ref = git('rev-parse', '--verify', (requested or upstream) + '^{commit}', required=False) if requested or upstream else ''
        self.tracked = set(git('ls-files', '-z').split('\0'))
        self.published = {}
        if self.ref:
            for record in git('ls-tree', '-r', '-z', self.ref).split('\0'):
                if '\t' in record:
                    meta, path = record.split('\t', 1)
                    self.published[path] = meta.split()[2]
        self.links = {}

    def link(self, item):
        if isinstance(item, str):
            item = {'path': item}
        name = item.get('path', '')
        path = Path(name)
        if not name or path.is_absolute() or '..' in path.parts or path.as_posix() != name:
            return None
        if name not in self.tracked or path.suffix.lower() not in ALLOWED_SUFFIXES:
            return None
        if path.parts[0] not in {'analysis', 'recovered', 'tools', 'docs', 'README.md', 'RESEARCH.md', 'plan.md'}:
            return None
        if {'work', 'ghidra', 'assets', 'extracted', 'captures', 'build'} & set(path.parts):
            return None
        actual = ROOT / path
        if actual.resolve() != actual or not actual.is_file():
            return None
        if name not in self.links:
            # Compare working bytes to the public tree, not just its filename.
            same = self.published.get(name) == git('hash-object', '--', name)
            from urllib.parse import quote
            self.links[name] = f'{REPOSITORY}/blob/{self.ref}/{quote(name, safe="/")}' if same else None
        line = item.get('line')
        line = line if type(line) is int and line > 0 else None
        return {'path': name, 'url': self.links[name] + (f'#L{line}' if line else '') if self.links[name] else None,
                'line': line, 'pointer': item.get('json_pointer'),
                'availability': 'published' if self.links[name] else 'unpublished'}

    def evidence(self, items):
        links = [self.link(item) for item in items]
        # Preserve distinct locations without repeated rendering of identical links.
        return list({json.dumps(link, sort_keys=True): link for link in links if link}.values())


def export():
    raw = analyze(ROOT)
    pub = Publication()
    graph = raw['graph']
    nodes = []
    for item in graph['nodes']:
        data = item['data']
        levels = {}
        for stage, value in data['verification_levels'].items():
            levels[stage] = {'status': value['status'], 'evidence': pub.evidence(value['provenance'])}
            if value.get('scope'):
                levels[stage]['scope'] = prose(value['scope'])
            if value.get('reports'):
                levels[stage]['reports'] = [report_text(v) for v in value['reports']]
        nodes.append({'id': item['id'], 'type': item['type'], 'data': {
            **{key: prose(data[key]) for key in ['label', 'status', 'claim', 'coverage', 'limitations', 'scope', 'date', 'program', 'original_entry'] if key in data and not (item['type'] == 'function' and key in {'coverage', 'limitations'})},
            'evidence_state': data.get('evidence_state', 'unknown'),
            'evidence': pub.evidence(data.get('provenance', [])),
            'verification_levels': levels}})
    edges = []
    for item in graph['edges']:
        data = item['data']
        edges.append({key: item[key] for key in ['id', 'source', 'target', 'type']} | {'data': {
            'relationship': data['relationship'], 'basis': data['basis'], 'evidence_state': data['evidence_state'],
            'rationale': prose(data.get('rationale')), 'evidence': pub.evidence(data['provenance'])}})
    functions = []
    for row in raw['functions']['entries']:
        functions.append({'id': f"function:{row['program']}:{row['entry']}",
                          **{k: prose(row[k]) for k in ['program', 'entry', 'status', 'coverage', 'limitations']},
                          'verification': prose(row.get('verification')),
                          'evidence': pub.evidence([row['record'], *row['evidence']])})
    checklist = []
    for item in raw['checklist']['items']:
        evidence = [{'path': item['path'], 'line': item['line']}]
        evidence.extend({'path': p} for p in re.findall(r'\]\(([^)#]+)(?:#[^)]*)?\)', item['text']))
        checklist.append({**{key: item[key] for key in ['id', 'line', 'checked', 'explicit_source_only']},
                          'headings': [prose(h) for h in item['headings']], 'text': prose(item['text']),
                          'evidence': pub.evidence(evidence)})
    milestones = []
    for item in raw['milestones']['items']:
        requirements = [c for c in checklist if c['headings'] and 'Phase 2' in c['headings'][0]
                        and len(c['headings']) > 1 and c['headings'][1].startswith(item['id'].split('.')[1] + '.')]
        milestones.append({**{key: prose(item[key]) for key in ['id', 'title', 'reported_status', 'acceptance']},
                           'prerequisites': [e['prerequisite'] for e in raw['milestones']['dependencies'] if e['dependent'] == item['id']],
                           'requirements': [c['id'] for c in requirements], 'evidence': pub.evidence([item['record']])})
    checkpoints = []
    for row in raw['checkpoints']:
        checkpoints.append({'id': 'subsystem:checkpoint:' + Path(row['path']).stem,
                            'title': re.sub(r'-[0-9]{8}$', '', Path(row['path']).stem).replace('-', ' '),
                            'date': prose(row['date']), 'scope': prose(row['scope']),
                            'source_only': row['source_only'], 'desktop_verified': row['desktop_verified'],
                            'evidence': pub.evidence([row['record']]),
                            'observations': [{'stage': v['stage'], 'reported': report_text(v['reported']),
                                              'evidence': pub.evidence([v['record']])} for v in row['observations']]})
    commits = []
    for line in git('log', '-30', '--format=%H%x09%cI%x09%s').splitlines():
        sha, date, subject = line.split('\t', 2)
        public = subprocess.run(['git', '-C', str(ROOT), 'merge-base', '--is-ancestor', sha, pub.ref], capture_output=True).returncode == 0 if pub.ref else False
        commits.append({'sha': sha, 'date': date, 'subject': prose(subject), 'url': f'{REPOSITORY}/commit/{sha}' if public else None,
                        'status': 'committed', 'publication': 'published' if public else 'local'})
    # Extra scope statements are quoted and validated, not copied frontend status.
    scope_text = (ROOT / 'plan.md').read_text(encoding='utf-8')
    scope_anchor = 'local assets through Proton; Sabre Squadron and engine replacement are separate.'
    scope_known = scope_text.count(scope_anchor) == 1
    server = [c for c in checklist if c['headings'] and c['headings'][0].startswith('Phase 3') and not any('Postponed' in h for h in c['headings'])]
    areas = [{'id': 'client', 'name': 'Game Client', 'status': next(m['reported_status'] for m in raw['milestones']['items'] if m['id'] == raw['milestones']['active_milestone'])
              if raw['milestones']['active_milestone'] in {m['id'] for m in raw['milestones']['items']} else 'unknown',
              'milestone': raw['milestones']['active_milestone'], 'evidence': pub.evidence([{'path': 'analysis/phase2-delivery-plan.json', 'json_pointer': '/active_milestone'}])},
             {'id': 'server', 'name': 'Dedicated Server', 'status': 'planned' if server and not any(c['checked'] for c in server) else 'unknown',
              'milestone': 'Phase 3' if server else None, 'evidence': [e for c in server[:1] for e in c['evidence']]},
             {'id': 'replacement', 'name': 'Engine Replacement', 'status': 'separate_scope' if scope_known else 'unknown',
              'milestone': None, 'evidence': pub.evidence([{'path': 'plan.md', 'line': scope_text[:scope_text.index(scope_anchor)].count('\n') + 1}]) if scope_known else []}]
    output = {'schema_version': 1, 'snapshot': {'source_commit': pub.head, 'published_commit': pub.ref or None,
              'repository': REPOSITORY, 'latest_receipt_date': max((c['date'] for c in checkpoints if c['date']), default=None)},
              'areas': areas, 'active_milestone': raw['milestones']['active_milestone'],
              'counts': raw['functions']['totals'], 'by_program': raw['functions']['by_program'],
              'functions': functions, 'milestones': milestones, 'checklist': checklist, 'checkpoints': checkpoints,
              'commits': commits, 'graph': {'nodes': nodes, 'edges': edges, 'groups': graph['groups'],
               'unknown_relationships': [{**{k: prose(v) for k, v in u.items() if k in {'id', 'source', 'original_entry', 'relationship', 'basis', 'reason'}},
                                          'target': None, 'evidence': pub.evidence(u['provenance'])} for u in graph['unknown_relationships']]},
              'diagnostics': dict(sorted(Counter(d['code'] for d in raw['diagnostics']).items())),
              'inputs': raw['inputs']}
    encoded = json.dumps(output, sort_keys=True, separators=(',', ':'), ensure_ascii=True, allow_nan=False) + '\n'
    if PRIVATE.search(encoded) or ABSOLUTE.search(encoded):
        raise ValueError('Publication contains a private/local path')
    destination = SITE / 'public/data/progress.json'
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(encoded, encoding='utf-8')
    print(f'Public snapshot: {len(functions)} credited entries, {len(nodes)} graph nodes; private artifact fields omitted.')


if __name__ == '__main__':
    export()
