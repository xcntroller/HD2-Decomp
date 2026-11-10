#!/usr/bin/env python3
"""Additive schema1 P2.8 research graph. No ledger/website/game writes."""
import copy
import hashlib
import importlib.util
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
PREFIX = 'analysis/multiplayer-world/'
SOURCE = 'c09c22c01d1049d7f087142064a3c2c53488f582'


def read(path):
    return json.loads((ROOT / path).read_text())


def claim_data(address, ledger, ledger_bytes):
    row = ledger.get(('client', address))
    status = row[1]['status'] if row else 'no_ledger_claim'
    provenance = [{'path': 'analysis/function-status.json', 'revision': SOURCE,
                   'json_pointer': '/functions/' + str(row[0])}] if row else []
    return dict(status=status, implementation_status=status,
                source_commit_sha=SOURCE,
                source_mapping=[p for p in row[1].get('evidence', []) if p.startswith('recovered/')] if row else [],
                implementation_limitations=row[1].get('limitations') if row else 'No ledger claim; absence is not proof of no source',
                claim_provenance=provenance)


def extend(graph):
    """Keep historical IDs/data; new source observations never refresh old claims."""
    catalog_path = PREFIX + 'evidence/contracts.json'
    manifest_path = PREFIX + 'evidence/manifest.json'
    review_path = PREFIX + 'evidence/source-review.json'
    catalog, manifest = read(catalog_path), read(manifest_path)
    nodes = {n['id']: n for n in graph['graph']['nodes']}
    edges = {e['id']: e for e in graph['graph']['edges']}
    ledger_bytes = subprocess.check_output(['git', 'show', SOURCE + ':analysis/function-status.json'], cwd=ROOT)
    ledger = {(r['program'], r['entry']): (i, r) for i, r in enumerate(json.loads(ledger_bytes)['functions'])}
    exports = {r['entry']: (i, r) for i, r in enumerate(manifest['exports'])}
    observation = [{'path': review_path, 'revision_of_observation': SOURCE,
                    'json_pointer': '/sources'}, {'path': review_path, 'json_pointer': '/findings'}]

    def prov(path, **extra):
        return {'path': path, **extra}

    def unique(records):
        seen = set()
        result = []
        for record in records:
            key = json.dumps(record, sort_keys=True)
            if key not in seen:
                result.append(record)
                seen.add(key)
        return result

    def node(key, kind, **data):
        data.setdefault('source_commit_sha', SOURCE)
        data.setdefault('status', data.get('implementation_status', 'unknown'))
        data.setdefault('verification_levels', {s: {'status': 'unknown', 'provenance': []}
                                               for s in ['source_recovery', 'integration', 'native', 'i386', 'desktop']})
        data.setdefault('prerequisites', [])
        data.setdefault('ordering_constraints', [])
        data.setdefault('batch_ids', [])
        data['source_observations'] = copy.deepcopy(observation)
        nodes[key] = dict(id=key, type=kind, data=data)

    def edge(source, target, role, evidence, blocking=False, condition=None, confidence='confirmed', **extra):
        key = 'relationship:' + hashlib.sha256(json.dumps([source, target, role], separators=(',', ':')).encode()).hexdigest()
        if key in edges:
            data = edges[key]['data']
            data['provenance'] = unique(data['provenance'] + evidence)
            if role == 'virtual_dispatch':
                selector = dict(condition=condition, **extra)
                data.setdefault('dispatches', []).append(selector)
                data['dispatches'] = sorted({json.dumps(p, sort_keys=True): p for p in data['dispatches']}.values(), key=lambda p: json.dumps(p, sort_keys=True))
            return
        if role == 'virtual_dispatch':
            extra['dispatches'] = [dict(condition=condition, **extra)]
        edges[key] = dict(id=key, source=source, target=target,
                         type='dependency' if role == 'depends_on' else 'relationship',
                         data=dict(relationship=role, blocking=blocking, condition=condition, confidence=confidence,
                                   basis='explicit' if confidence == 'confirmed' else 'inferred',
                                   evidence_state='verified' if confidence == 'confirmed' else 'unknown', provenance=evidence, **extra))

    node('milestone:P2.8', 'milestone', label='P2.8', milestones=['P2.8'], subsystem=None,
         original_addresses=[], owner='Same application/game/process owners', lifetime='Phase2 bounded source receiver',
         source_mapping=[], implementation_status='not_delivered', research_readiness='bounded_architecture_contract_available',
         implementation_readiness='blocked', confidence='confirmed', priority='high',
         priority_reason='User-directed shared multiplayer client world research',
         blockers=['Actual P2.3 owners and bounded request/cleanup integration; Phase3 live transport excluded'],
         provenance=[prov(PREFIX + 'implementation-guide.md'), prov('analysis/phase2-delivery-plan.md', revision=SOURCE)])

    for collection in ['contracts', 'batches']:
        for i, contract in enumerate(catalog[collection]):
            evidence = [prov(catalog_path, json_pointer=f'/{collection}/{i}'), prov(contract['contract'])]
            data = {k: v for k, v in contract.items() if k not in ('id', 'contract', 'predecessor_batches')}
            data.setdefault('source_mapping', [])
            node(contract['id'], 'subsystem', **data,
                 implementation_status='research_only_no_new_implementation', provenance=evidence)
            edge('milestone:P2.8', contract['id'], 'contains_subsystem', evidence)
            for address in contract['original_addresses']:
                fk = 'function:client:' + address
                if fk not in nodes:
                    claims = claim_data(address, ledger, ledger_bytes)
                    claim_provenance = claims.pop('claim_provenance')
                    indexed = exports.get(address)
                    provenance = evidence + claim_provenance
                    if indexed:
                        provenance.append(prov(manifest_path, json_pointer=f'/exports/{indexed[0]}'))
                    node(fk, 'function', label='client:' + address, subsystem=contract['subsystem'],
                         milestones=['P2.8'], original_addresses=[address], owner=contract['owner'],
                         lifetime='Actual containing owner; see focused contract', **claims,
                         research_readiness='partial_contract' if indexed else 'address_or_existing_contract_only',
                         implementation_readiness='blocked', confidence='confirmed',
                         priority='high', priority_reason='Reached shared-world receiver/dispatch',
                         blockers=['Actual reached downstream services and batch integration required; architecture evidence alone is not full format recovery'],
                         provenance=provenance)
                    nodes[fk]['data']['verification_levels']['source_recovery'] = dict(status=claims['status'], provenance=claim_provenance)
                function_data = nodes[fk]['data']
                function_data.setdefault('milestones', []).append('P2.8')
                function_data.setdefault('batch_ids', []).extend(contract['batch_ids'])
                # Keep old research/implementation claims; append P2.8 provenance separately.
                # Historical exporter may share an observation list across nodes.
                # Copy before adding; never attach P2.8 observations to unrelated nodes.
                old_observations = function_data.get('source_observations', [])
                function_data['source_observations'] = old_observations + [p for p in copy.deepcopy(observation) if p not in old_observations]
                function_data['provenance'] = function_data['provenance'] + [p for p in evidence if p not in function_data['provenance']]
                edge(contract['id'], fk, 'original_function_reference', evidence)
            for predecessor in contract.get('predecessor_batches', []):
                edge(predecessor, contract['id'], 'depends_on', evidence, True,
                     condition='Mandatory predecessor for this bounded batch; see conditional runtime scope')

    edge('milestone:P2.3', 'subsystem:future:B28.entry', 'depends_on',
         [prov(PREFIX + 'implementation-guide.md')], True,
         condition='Shared game receiver needs actual world owners; metadata-only research/native parsing can precede desktop P2.3')
    for i, relation in enumerate(catalog['relationships']):
        extra = {k: v for k, v in relation.items() if k not in ('source', 'target', 'relationship', 'contract', 'blocking', 'condition', 'confidence')}
        edge(relation['source'], relation['target'], relation['relationship'],
             [prov(catalog_path, json_pointer=f'/relationships/{i}'), prov(relation['contract'])],
             relation['blocking'], relation['condition'], relation['confidence'], **extra)
    for address, (i, export) in exports.items():
        source = 'function:client:' + address
        if source not in nodes:
            continue
        calls = {}
        for call in export['direct_calls']:
            calls.setdefault(call['target'], []).append(call['site'])
        for target, sites in sorted(calls.items()):
            dest = 'function:client:' + target
            if dest in nodes:
                edge(source, dest, 'direct_call', [prov(manifest_path, json_pointer=f'/exports/{i}/direct_calls')],
                     call_sites=sorted(set(sites)))

    for e in edges.values():
        if e['data']['relationship'] == 'depends_on':
            nodes[e['target']]['data']['prerequisites'].append(e['source'])
        elif e['data']['relationship'] in ('initialization_order', 'cleanup_order'):
            nodes[e['target']]['data']['ordering_constraints'].append(e['id'])
    for n in nodes.values():
        for key in ('prerequisites', 'ordering_constraints', 'milestones', 'batch_ids'):
            if key in n['data']:
                n['data'][key] = sorted(set(n['data'][key]))
    graph['graph']['nodes'] = sorted(nodes.values(), key=lambda n: n['id'])
    graph['graph']['edges'] = sorted(edges.values(), key=lambda e: e['id'])
    graph['graph']['unknown_relationships'].extend(catalog['unresolved'])
    paths = sorted(p for p in HERE.rglob('*') if p.is_file() and p.suffix in ('.md', '.json', '.py')
                   and p.name not in ('p2.8-graph.json', 'verification.json'))
    for path in paths:
        graph['inputs'].append(dict(path=str(path.relative_to(ROOT)), sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
    graph['inputs'].sort(key=lambda r: (r['path'], r.get('revision', '')))
    return graph


def focused(combined):
    result = copy.deepcopy(combined)
    selected = {n['id'] for n in result['graph']['nodes'] if 'P2.8' in n['data'].get('milestones', [])}
    selected.add('milestone:P2.3')
    result['graph']['nodes'] = [n for n in result['graph']['nodes'] if n['id'] in selected]
    result['graph']['edges'] = [e for e in result['graph']['edges'] if e['source'] in selected and e['target'] in selected]
    result['graph']['unknown_relationships'] = read(PREFIX + 'evidence/contracts.json')['unresolved']
    ledger_bytes = subprocess.check_output(['git', 'show', SOURCE + ':analysis/function-status.json'], cwd=ROOT)
    ledger = {(r['program'], r['entry']): (i, r) for i, r in enumerate(json.loads(ledger_bytes)['functions'])}
    contracts = read(PREFIX + 'evidence/contracts.json')['contracts']
    for n in result['graph']['nodes']:
        data = n['data']
        data['prerequisites'] = [p for p in data['prerequisites'] if p in selected]
        valid_edges = {e['id'] for e in result['graph']['edges']}
        data['ordering_constraints'] = [e for e in data['ordering_constraints'] if e in valid_edges]
        if n['type'] != 'function':
            continue
        claims = claim_data(data['original_addresses'][0], ledger, ledger_bytes)
        provenance = claims.pop('claim_provenance')
        data.update(claims)
        associations = [c for c in contracts if data['original_addresses'][0] in c['original_addresses']]
        # A P2.6/P2.4 bounded body may still lack P2.8 nonoffline branches.
        # Preserve its historical record in the combined graph, not as P2.8 readiness.
        data['research_readiness'] = 'partial_contract'
        data['implementation_readiness'] = 'blocked'
        data['blockers'] = sorted({b for c in associations for b in c['blockers']})
        if associations:
            data['owner'] = associations[0]['owner']
            data['lifetime'] = associations[0]['lifetime']
            data['subsystem'] = associations[0]['subsystem']
        data['verification_levels'] = {s: {'status': 'unknown', 'provenance': []}
                                       for s in ['source_recovery', 'integration', 'native', 'i386', 'desktop']}
        data['verification_levels']['source_recovery'] = dict(status=claims['status'], provenance=provenance)
        data['provenance'] = [p for p in data['provenance'] if p['path'] != 'analysis/function-status.json'] + provenance
        # Old source mentions remain historical/provenanced; never imply a new match.
    result['source_commit_sha'] = SOURCE
    result['inputs'] = [r for r in result['inputs'] if r['path'] != 'analysis/function-status.json']
    result['inputs'].append(dict(path='analysis/function-status.json', revision=SOURCE, sha256=hashlib.sha256(ledger_bytes).hexdigest()))
    result['inputs'].sort(key=lambda r: (r['path'], r.get('revision', '')))
    return result


if __name__ == '__main__':
    spec = importlib.util.spec_from_file_location('future_graph', ROOT / 'analysis/future-dependencies/export_graph.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    output = focused(module.generate())
    (HERE / 'p2.8-graph.json').write_text(json.dumps(output, indent=2, sort_keys=True) + '\n')
    print(f"{len(output['graph']['nodes'])} nodes, {len(output['graph']['edges'])} edges")
