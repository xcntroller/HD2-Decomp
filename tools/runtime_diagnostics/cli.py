#!/usr/bin/env python3
"""Offline evidence analysis for reconstructed HD2 logs. Never executes game code."""
import argparse
import collections
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import sys

VERSION = 1
LIMIT = 2000
LINE_LIMIT = 65536
STATUSES = ['observed_success', 'observed_failure', 'not_observed', 'not_instrumented', 'not_applicable', 'indeterminate']
STAGES = ['startup', 'preparation', 'game_state', 'session_construction', 'resource_loading',
          'actor_module_loading', 'activation', 'first_camera', 'update', 'render', 'presentation', 'cleanup', 'shutdown']
INSTRUMENTED = {'startup', 'preparation', 'game_state', 'session_construction', 'activation', 'cleanup', 'shutdown'}
# Exact producer strings, rather than generic keyword-based success inference.
RULES = [
 ('startup_begin', r'^Recovered application controller launch;', None, 'indeterminate'),
 ('startup_prepare', r'^Startup: gate=\d+ .* prepare=(\d+)$', 'startup', 'prepare'),
 ('preparation_request', r'^Pre-mission handoff validated:', 'preparation', 'observed_success'),
 ('controller_state', r'^Controller active: type=(\d+) sequence=(\d+) status=(\d+) phase=(\d+)$', None, 'observed_success'),
 ('owner_constructed', r'^Game owner constructed: .* loaded=0$', 'session_construction', 'observed_success'),
 ('activation_declined', r'^Game activation declined:', 'activation', 'observed_failure'),
 ('owner_failed', r'^Game owner initialization failed:', 'session_construction', 'observed_failure'),
 ('entry_failed', r'^Game entry failure:', 'activation', 'observed_failure'),
 ('launch_failed', r'^Controller launch failed:', None, 'observed_failure'),
 ('loop_completed', r'^Controller launch completed; frames=(\d+) reason=([\w-]+)$', None, 'observed_success'),
 ('teardown', r'^Controller launch teardown completed; result=(\d+)$', 'shutdown', 'zero'),
 ('startup_cleanup', r'^Shutdown: input/sound/data/module cleanup completed=(\d+)$', 'cleanup', 'one'),
 ('owner_destroyed', r'^Shutdown: archives released; startup owner destroyed$', 'cleanup', 'observed_success'),
 ('cleanup_event', r'^(Shutdown:|3D driver closed$|Original sound driver closed:|Controller graphics closed$)', None, 'indeterminate'),
 ('input_capture', r'^Loading input (captured|absent):', None, 'indeterminate'),
 ('capture_only', r'^Loading-input capture completed; controller/game not started$', None, 'not_applicable'),
 ('graphics_error', r'^Graphics initialization result:', None, 'observed_failure'),
 ('item_rejected', r'^Mission item transfer rejected:', None, 'observed_failure'),
 ('item_unavailable', r'^Mission item definition is unavailable:', None, 'observed_failure'),
 ('sound_missing', r'^AESound :.* : not found$', None, 'indeterminate'),
]
RULES = [(a, re.compile(b), c, d) for a,b,c,d in RULES]


def read_json(path):
    if Path(path).stat().st_size > 65536:
        raise ValueError('JSON metadata exceeds 64 KiB')
    return json.loads(Path(path).read_text(encoding='utf-8'))


def metadata(path):
    data = read_json(path) if path else {}
    allowed = {'schema_version', 'scenario', 'build_commit', 'build_sha256', 'exit_code',
               'launch_arguments', 'conditions', 'outcome', 'screenshots', 'mission_expected', 'wine_output_reported'}
    if not isinstance(data, dict) or set(data) - allowed:
        raise ValueError('Unknown metadata fields or non-object metadata')
    for k,v in data.items():
        if k in {'mission_expected','wine_output_reported'}:
            valid = type(v) is bool
        elif k in {'exit_code', 'schema_version'}:
            valid = type(v) is int
        elif k in {'screenshots','launch_arguments'}:
            valid = isinstance(v,list) and all(isinstance(x,str) for x in v)
        else:
            valid = isinstance(v,str)
        if not valid:
            raise ValueError('Invalid metadata type: ' + k)
    if data.get('schema_version',1) != 1:
        raise ValueError('Unsupported metadata version')
    for k,n in [('build_commit',40),('build_sha256',64)]:
        if k in data and not re.fullmatch('[0-9a-fA-F]{'+str(n)+'}',data[k]):
            raise ValueError('Invalid ' + k)
    return data


def normalized(text):
    text = re.sub(r'\b0x[0-9a-fA-F]+\b|\b[0-9a-fA-F]{8,16}\b', '<address-or-id>', text)
    text = re.sub(r'(?:[A-Za-z]:[\\/]|/)[^\s;]+', '<path>', text)
    return text


def analyze(path, meta, sanitized=False):
    stages = {s:{'status':'not_observed' if s in INSTRUMENTED else 'not_instrumented', 'evidence':[]} for s in STAGES}
    events, issues = [], []
    counts, errors = collections.Counter(), collections.Counter()
    digest = hashlib.sha256()
    lines = unknown = long_lines = 0
    session = 0
    seen = set()
    last = None
    capture_only = False
    def issue(kind, line):
        if len(issues) < LIMIT:
            issues.append({'kind':kind,'line':line})
    with Path(path).open('rb') as f:
        while True:
            raw = f.readline(LINE_LIMIT+1)
            if not raw:
                break
            lines += 1
            digest.update(raw)
            oversized = len(raw)>LINE_LIMIT
            if oversized:
                long_lines += 1
                while raw and not raw.endswith(b'\n'):
                    raw = f.readline(LINE_LIMIT+1)
                    digest.update(raw)
                issue('oversized_line_skipped',lines)
                continue
            text = raw.decode('utf-8',errors='replace').rstrip('\r\n')
            if '\ufffd' in text:
                issue('invalid_utf8',lines)
            for ident, regex, stage, status in RULES:
                m = regex.search(text)
                if not m:
                    continue
                if ident == 'startup_begin':
                    if session and 'teardown' not in seen and 'capture_only' not in seen:
                        issue('restart_without_recorded_teardown',lines)
                    session += 1
                    seen = set()
                if ident == 'controller_state' and m[1] == '1':
                    stage = 'game_state'
                if status in {'prepare','one','zero'}:
                    status = 'observed_success' if int(m[1]) == (0 if status == 'zero' else 1) else 'observed_failure'
                if ident in {'teardown','owner_destroyed','startup_cleanup'} and ident in seen:
                    issue('repeated_'+ident,lines)
                if ident == 'owner_constructed' and 'preparation_request' not in seen:
                    issue('owner_without_observed_preparation',lines)
                if ident == 'owner_constructed' and 'teardown' in seen:
                    issue('owner_after_teardown',lines)
                if ident == 'capture_only':
                    capture_only = True
                seen.add(ident)
                counts[ident] += 1
                event = {'id':ident,'line':lines,'session':session,'status':status}
                if ident in {'controller_state','loop_completed'}:
                    event['values'] = list(m.groups())
                    if sanitized and ident == 'loop_completed' and event['values'][1] not in {'window-close','frame-limit','state-exit'}:
                        event['values'][1] = 'unknown'
                if ident == 'cleanup_event':
                    cleanup_names = {
                        'Shutdown: launch window hook restored':'window_hook_restored',
                        'Shutdown: final cold control reset and startup locks released':'cold_control_reset',
                        'Shutdown: loaded-resource caches released before engine drivers':'caches_released',
                        '3D driver closed':'driver_closed',
                        'Original sound driver closed: initialized=0':'sound_closed',
                        'Controller graphics closed':'graphics_closed',
                        'Shutdown: window.cfg write attempted':'window_cfg_write_attempted',
                        'Shutdown: window.cfg open failed; skipped':'window_cfg_open_failed'}
                    event['values'] = [cleanup_names.get(text,'other_cleanup')]
                if not sanitized:
                    event['text'] = text[:2048]
                if len(events) < LIMIT:
                    events.append(event)
                if status == 'observed_failure':
                    # Bound distinct errors as well as event storage.
                    key = ident if sanitized else normalized(text[:2048])
                    if key in errors or len(errors)<LIMIT:
                        errors[key] += 1
                if stage:
                    previous = stages[stage]['status']
                    stages[stage]['status'] = status if previous in {'not_observed','not_instrumented',status} else 'indeterminate'
                    if len(stages[stage]['evidence'])<32:
                        stages[stage]['evidence'].append(lines)
                    if status == 'observed_success':
                        last = {'stage':stage,'line':lines}
                break
            else:
                unknown += 1
                if re.search(r'\b(warn(?:ing)?|error|fail(?:ed|ure)?|rejected|unavailable)\b',text,re.I):
                    counts['unclassified_warning'] += 1
                    if len(events)<LIMIT:
                        event={'id':'unclassified_warning','line':lines,'session':session,'status':'indeterminate'}
                        if not sanitized:
                            event['text']=text[:2048]
                        events.append(event)
    mission_attempt = bool(counts['preparation_request'] or counts['owner_constructed'] or counts['activation_declined'] or counts['entry_failed'])
    if capture_only and mission_attempt:
        issue('capture_only_and_mission_attempt_in_combined_input',0)
    if (capture_only and session <= 1 and not mission_attempt) or (meta.get('mission_expected') is False and not mission_attempt):
        for s in STAGES[1:11]:
            if not stages[s]['evidence']:
                stages[s]['status']='not_applicable'
    if not counts['teardown'] and not capture_only:
        issue('no_recorded_teardown_truncation_or_abnormal_exit_possible',lines)
    if meta.get('exit_code',0) != 0:
        issue('reported_nonzero_process_exit',0)
    public_meta = {k:v for k,v in meta.items() if k in {'schema_version','build_commit','build_sha256','exit_code','mission_expected','wine_output_reported'}} if sanitized else meta
    return {'schema_version':1,'kind':'run','sanitized':sanitized,
            'source':{'name':'log' if sanitized else str(path),'sha256':digest.hexdigest(),'lines':lines},
            'metadata':public_meta,'stages':stages,'events':events,'event_counts':dict(sorted(counts.items())),
            'errors':dict(sorted(errors.items())),'issues':issues,'last_confirmed_stage':last,
            'last_confirmed_mission_stage':next(({'stage':s,'line':stages[s]['evidence'][-1]} for s in reversed(STAGES[1:11]) if stages[s]['status']=='observed_success'),None),
            'limits':{'retained_events':LIMIT,'events_omitted':max(0,sum(counts.values())-len(events)), 'unknown_lines':unknown,'oversized_lines':long_lines},
            'next_investigation': (
                ['Inspect the recorded failure lines and the last completed operation before them.',
                 'If activation was declined, check whether the tested build integrates the real World/frame graph.']
                if errors else ['Resolve unobserved/instrumentation gaps before drawing runtime conclusions.']) +
                ['For camera/render/presentation gaps, use the focused instrumentation proposal and user desktop capture.',
                 'Check the executed build identity and wait through the original startup fade before evaluating black output.'],
            'uncertainty':['No desktop acceptance is inferred.',
                'Controller frame counts include menus/movies; no world presentation is established.',
                'Resource input capture is not mission resource loading.',
                'Black output may reflect the original approximately 3–4 second startup fade.',
                'Render may precede fixed/update. Missing logs never imply success.',
                'Multiple attempts aggregate to indeterminate on conflicting outcomes; inspect session and line references.',
                'Reported metadata is user supplied; build identity is not inferred from repository HEAD.',
                'Unknown records and future instrumentation require parser review; capped evidence is incomplete.']}


def validate_report(r):
    if not isinstance(r,dict) or r.get('schema_version') != 1 or r.get('kind') != 'run':
        raise ValueError('Expected a version 1 run report')
    for k in ['source','metadata','stages','errors','event_counts']:
        if not isinstance(r.get(k),dict):
            raise ValueError('Invalid report field: '+k)
    if type(r.get('sanitized')) is not bool or not isinstance(r.get('events'),list):
        raise ValueError('Invalid report events or sanitized flag')
    if set(r['stages']) != set(STAGES):
        raise ValueError('Invalid report stages')
    for stage in r['stages'].values():
        if not isinstance(stage,dict) or stage.get('status') not in STATUSES or not isinstance(stage.get('evidence'),list):
            raise ValueError('Invalid report stage')
    for e in r['events']:
        if not isinstance(e,dict) or not isinstance(e.get('id'),str) or e.get('status') not in STATUSES:
            raise ValueError('Invalid report event')
    for k in ['errors','event_counts']:
        if any(type(v) is not int or v<0 for v in r[k].values()):
            raise ValueError('Invalid report counts')


def compare(a,b):
    validate_report(a)
    validate_report(b)
    if a.get('kind')!='run' or b.get('kind')!='run':
        raise ValueError('Comparison requires run reports')
    def delta(key):
        av,bv=a[key],b[key]
        return {k:{'before':av.get(k),'after':bv.get(k)} for k in sorted(set(av)|set(bv)) if av.get(k)!=bv.get(k)}
    return {'schema_version':1,'kind':'comparison','sanitized':a['sanitized'] and b['sanitized'],
            'sources':[a['source'],b['source']],
            'newly_observed':[s for s in STAGES if not a['stages'][s]['evidence'] and b['stages'][s]['evidence']],
            'no_longer_observed':[s for s in STAGES if a['stages'][s]['evidence'] and not b['stages'][s]['evidence']],
            'stage_changes':{s:{'before':a['stages'][s],'after':b['stages'][s]} for s in STAGES if a['stages'][s]['status']!=b['stages'][s]['status']},
            'cleanup_changes': {
                'before':[{'id':e['id'],'status':e['status'],'values':e.get('values',[])} for e in a['events'] if e['id'] in {'cleanup_event','startup_cleanup','owner_destroyed','teardown'}],
                'after':[{'id':e['id'],'status':e['status'],'values':e.get('values',[])} for e in b['events'] if e['id'] in {'cleanup_event','startup_cleanup','owner_destroyed','teardown'}]},
            'error_changes':delta('errors'),'event_changes':delta('event_counts'),'metadata_changes':delta('metadata'),
            'uncertainty':['Identical logs do not establish behavioral equivalence.', 'Counts compare recognized events; ordering and raw variable values are not compared.', 'Sanitized errors compare marker categories only.']}


def markdown(r):
    # Fence raw/user strings using JSON escaping and escape Markdown/HTML delimiters.
    def safe(v):
        return json.dumps(v,ensure_ascii=True).replace('<','&lt;').replace('>','&gt;').replace('`','&#96;').replace('|','&#124;')
    out=['# Runtime diagnostic report', '']
    if r['kind']=='run':
        out += ['Last confirmed operation: '+safe(r['last_confirmed_stage']),
                '', 'Last confirmed mission stage: '+safe(r['last_confirmed_mission_stage']),
                '', 'No desktop acceptance is inferred.', '', '## Next investigation', '']
        out += ['- '+x for x in r['next_investigation']]
        out += ['', '## Evidence and limits', '']
    out += ['```json', json.dumps({k:v for k,v in r.items() if k not in {'events','stages'}},indent=2,sort_keys=True,ensure_ascii=True).replace('`','\\u0060').replace('<','\\u003c'), '```','']
    if r['kind']=='run':
        out += ['| Stage | Status | Source lines |','| --- | --- | --- |']
        for stage,item in r['stages'].items():
            out.append(f"| {stage} | {item['status']} | {', '.join(map(str,item['evidence']))} |")
        out += ['', '## Retained events', '']
        out += [f"- Line {e['line']}, session {e['session']}: {e['id']} ({e['status']}) " + safe(e.get('text',e.get('values',''))) for e in r['events']]
    return '\n'.join(out)+'\n'


def write_report(r,directory):
    directory=Path(directory)
    directory.mkdir(parents=True,exist_ok=False,mode=0o700)
    for name,body in [('report.json',json.dumps(r,indent=2,sort_keys=True,ensure_ascii=True)+'\n'),('report.md',markdown(r))]:
        with (directory/name).open('x',encoding='utf-8') as f:
            os.chmod(f.name,0o600)
            f.write(body)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    sub=p.add_subparsers(dest='command',required=True)
    for name in ['analyze','capture']:
        q=sub.add_parser(name)
        q.add_argument('log',type=Path)
        q.add_argument('--metadata',type=Path)
        q.add_argument('--out',required=True,type=Path)
        q.add_argument('--sanitized',action='store_true')
        if name=='capture':
            q.add_argument('--exit-code',type=int)
            q.add_argument('--scenario')
            q.add_argument('--wine-output',type=Path,help='Explicitly supplied Wine/Proton output, private copy only')
    q=sub.add_parser('compare')
    q.add_argument('before',type=Path)
    q.add_argument('after',type=Path)
    q.add_argument('--out',required=True,type=Path)
    args=p.parse_args()
    try:
        if args.command=='compare':
            # Reports are bounded by the parser; allow up to 16 MiB for private reports.
            def report(path):
                if path.stat().st_size>16*1024*1024:
                    raise ValueError('Report too large')
                return json.loads(path.read_text(encoding='utf-8'))
            r=compare(report(args.before),report(args.after))
        else:
            meta=metadata(args.metadata)
            if args.command=='capture':
                if args.exit_code is not None: meta['exit_code']=args.exit_code
                if args.scenario is not None: meta['scenario']=args.scenario
                if args.wine_output: meta['wine_output_reported']=True
            r=analyze(args.log,meta,args.sanitized)
        write_report(r,args.out)
        if args.command=='capture' and not args.sanitized:
            for source,name in [(args.log,'host-runtime.log'),(args.wine_output,'wine-output.log')]:
                if source:
                    with source.open('rb') as src, (args.out/name).open('xb') as dst:
                        os.chmod(dst.name,0o600)
                        shutil.copyfileobj(src,dst,length=65536)
        print(str(args.out/'report.md'))
    except (OSError,ValueError,KeyError,TypeError) as error:
        p.exit(2,'Diagnostics failed: '+str(error)+'\n')

if __name__=='__main__':
    main()
