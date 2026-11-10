"""Deterministic, read-only join of versioned research and regenerated progress."""
import hashlib
import json
import re
from pathlib import Path
from export_public import ROOT, SITE, Publication, prose, build_snapshot, git


def require(ok, message):
    if not ok:
        raise ValueError(message)


def load(path):
    def unique(pairs):
        out = {}
        for k, v in pairs:
            require(k not in out, 'Duplicate JSON key')
            out[k] = v
        return out
    return json.loads(path.read_text(), object_pairs_hook=unique)


def build_research(progress=None):
    path = ROOT / 'analysis/future-dependencies/phase2-graph.json'
    if not path.exists():
        return {'schema_version': 1, 'available': False}
    raw = load(path)
    require(raw['schema_id'] == 'hd2.future-dependencies' and raw['schema_version'] == 1, 'Unsupported research schema')
    pub = Publication()
    progress = progress or build_snapshot()
    require(re.fullmatch(r'[0-9a-f]{40}', raw['source_commit_sha']), 'Invalid pinned source SHA')
    verification = load(ROOT / 'analysis/future-dependencies/evidence/verification.json')
    require(verification['graph_sha256'] == hashlib.sha256(path.read_bytes()).hexdigest(), 'Research verification graph digest mismatch')
    diagnostics = []
    for item in raw['inputs']:
        p = ROOT / item['path']
        require(not Path(item['path']).is_absolute() and '..' not in Path(item['path']).parts, 'Unsafe input')
        digest = hashlib.sha256(p.read_bytes()).hexdigest() if p.is_file() else None
        if digest != item['sha256']:
            diagnostics.append({'kind': 'stale_input', 'path': item['path'], 'research_digest': item['sha256'], 'current_digest': digest})
    def safe(value):
        if isinstance(value, dict):
            return {k: pub.evidence(v) if k == 'provenance' else safe(v) for k, v in value.items()}
        if isinstance(value, list):
            return [safe(v) for v in value]
        return prose(value) if isinstance(value, str) else value
    def check_evidence(value):
        if isinstance(value, dict):
            for e in value.get('provenance', []):
                require(pub.link(e) is not None, 'Missing or unsafe tracked evidence: ' + e.get('path', ''))
                if 'json_pointer' in e:
                    current = load(ROOT / e['path'])
                    for token in e['json_pointer'].split('/')[1:]:
                        token = token.replace('~1', '/').replace('~0', '~')
                        current = current[int(token)] if isinstance(current, list) else current[token]
            for v in value.values():
                check_evidence(v)
        elif isinstance(value, list):
            for v in value:
                check_evidence(v)
    check_evidence(raw['graph'])
    function_records = {f['id']: f for f in progress['functions']}
    nodes = {}
    for source, graph in [('implementation', progress['graph']), ('research', raw['graph'])]:
        seen = set()
        for node in graph['nodes']:
            key = node['id']
            require(key not in seen and ':' in key, 'Duplicate or invalid node ID')
            seen.add(key)
            record = nodes.setdefault(key, {'id': key, 'type': node['type'], 'snapshots': {}})
            require(record['type'] == node['type'], 'Conflicting node types')
            record['snapshots'][source] = safe(node['data']) if source == 'research' else {**node['data'], **{k: v for k, v in function_records.get(key, {}).items() if k in {'coverage', 'limitations', 'verification', 'evidence'}}}
            if source == 'research' and node['data'].get('source_mapping'):
                record['snapshots'][source]['source_evidence'] = pub.evidence(node['data']['source_mapping'])
    edges = {}
    for source, graph in [('implementation', progress['graph']), ('research', raw['graph'])]:
        seen = set()
        for edge in graph['edges']:
            key = edge['id']
            require(key not in seen, 'Duplicate edge ID')
            seen.add(key)
            d = edge['data']
            require(d.get('evidence_state') in {'verified', 'unknown', 'unresolved', 'inferred'}, 'Invalid evidence state')
            require(isinstance(d.get('relationship'), str), 'Invalid relationship')
            if source == 'research':
                require(type(d.get('blocking')) is bool, 'Invalid blocking classification')
                require(not d['blocking'] or d['relationship'] == 'depends_on', 'Calls cannot block scheduling')
            require(edge['source'] in nodes and edge['target'] in nodes, 'Dangling edge')
            record = edges.setdefault(key, {k: edge[k] for k in ['id', 'source', 'target'] } | {'snapshots': {}})
            require((record['source'], record['target']) == (edge['source'], edge['target']), 'Conflicting edge identity')
            require(all(d['relationship'] == edge['data']['relationship'] for d in record['snapshots'].values()), 'Conflicting edge semantics')
            record['snapshots'][source] = safe(edge['data']) if source == 'research' else edge['data']
    for n in nodes.values():
        a, b = n['snapshots'].get('implementation'), n['snapshots'].get('research')
        if n['type'] == 'function' and a and b and a.get('status') != b.get('implementation_status'):
            diagnostics.append({'kind': 'state_conflict', 'id': n['id'], 'current': a.get('status'), 'research': b.get('implementation_status')})
    guide = (ROOT / 'analysis/future-dependencies/implementation-guide.md').read_text()
    batches = []
    sections = re.split(r'(?m)^## ', guide)[1:]
    batch_pattern = r'B\d+(?:\.[\w-]+)+|P2\.\d+'
    batch_ids = set()
    for line, row in enumerate(guide.splitlines(), 1):
        if not re.match(r'\| B\d', row):
            continue
        cells = [v.strip() for v in row.strip('|').split('|')]
        require(len(cells) in {3, 4}, 'Unsupported batch table columns')
        name, predecessors, readiness = cells[:3]
        require(re.fullmatch(r'B\d+(?:\.[\w-]+)+', name), 'Invalid batch ID')
        require(name not in batch_ids, 'Duplicate batch ID')
        batch_ids.add(name)
        purpose = cells[3] if len(cells) == 4 else readiness
        section = next((v for v in sections if name in re.findall(batch_pattern, v.split('\n', 1)[0])), None)
        if section is None:
            # A child batch need not own a heading. Keep its exact table contract
            # and link associated machine-declared research contracts instead.
            body = purpose + '\nRequired predecessors and owner constraints: ' + predecessors + '\nResearch state: ' + readiness
        else:
            body = section.split('\n', 1)[1]
        associated = [n for n in raw['graph']['nodes'] if name in n['data'].get('batch_ids', [])]
        addresses = {a for n in associated for a in n['data'].get('original_addresses', [])}
        if not associated:
            addresses.update(a.lower() for a in re.findall(r'(?i)(?<![0-9a-f])00[0-9a-f]{6}(?![0-9a-f])', body))
        evidence = [{'path': 'analysis/future-dependencies/implementation-guide.md', 'line': line}]
        evidence.extend(e for n in associated for e in n['data'].get('provenance', []))
        batches.append({'id': 'batch:' + name, 'label': name,
                        'predecessors': re.findall(batch_pattern, predecessors),
                        'prerequisite_text': prose(predecessors),
                        'conditional_predecessors': re.findall(batch_pattern, predecessors) if 'where actually called' in predecessors else [],
                        'readiness': readiness, 'purpose': purpose, 'contract': prose(body),
                        'node_ids': sorted(n['id'] for n in associated),
                        'addresses': sorted(addresses), 'evidence': pub.evidence(evidence)})
    # Table order groups parent/child contracts, not necessarily execution order.
    pending_batches = {b['label']: set(b['predecessors']) - {p for p in b['predecessors'] if p.startswith('P2.')} for b in batches}
    require(all(p in batch_ids for deps in pending_batches.values() for p in deps), 'Unknown batch prerequisite')
    while pending_batches:
        ready = {name for name, deps in pending_batches.items() if not deps}
        require(ready, 'Batch prerequisite cycle')
        pending_batches = {name: deps - ready for name, deps in pending_batches.items() if name not in ready}
    # Fail cycles only for established blocking dependencies; never calls/ownership.
    blocking = [e for e in raw['graph']['edges'] if e['data'].get('blocking') is True and e['data']['evidence_state'] == 'verified' and e['data']['relationship'] == 'depends_on' and not e['data'].get('condition')]
    pending = {n: set() for n in nodes}
    for e in blocking:
        pending[e['target']].add(e['source'])
    while pending:
        ready = {n for n, deps in pending.items() if not deps}
        require(ready, 'Blocking dependency cycle')
        pending = {n: deps - ready for n, deps in pending.items() if n not in ready}
    return {'schema_version': 1, 'available': True, 'implementation_commit': pub.head, 'research_checkpoint_commit': git('log', '-1', '--format=%H', '--', 'analysis/future-dependencies/phase2-graph.json'), 'research_implementation_commit': raw['source_commit_sha'], 'research_graph_sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'policy': raw['epistemic_policy'], 'diagnostics': diagnostics, 'nodes': sorted(nodes.values(), key=lambda n: n['id']), 'edges': sorted(edges.values(), key=lambda n: n['id']), 'implementation_groups': progress['graph']['groups'], 'unknown_relationships': safe(raw['graph']['unknown_relationships']), 'batches': batches, 'verification': safe(load(ROOT / 'analysis/future-dependencies/evidence/verification.json'))}


def export():
    out = SITE / 'public/data/research.json'
    out.parent.mkdir(parents=True, exist_ok=True)
    require(not out.is_symlink(), 'Symlinked research output')
    out.write_text(json.dumps(build_research(), sort_keys=True, separators=(',', ':'), allow_nan=False) + '\n')

if __name__ == '__main__':
    export()
