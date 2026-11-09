#!/usr/bin/env python3
"""Build-time public artifact audit; never reads commercial assets or local logs."""
import json
from pathlib import Path
import re
import sys

root = Path(sys.argv[1] if len(sys.argv) > 1 else 'dist')
for file in root.rglob('*'):
    if not file.is_file():
        continue
    if file.suffix.lower() not in {'.html', '.js', '.css', '.json', '.svg', '.txt', '.woff2', '.woff'}:
        raise SystemExit('Unexpected public artifact type: ' + file.name)
    if file.suffix.lower() in {'.woff2', '.woff'}:
        continue
    text = file.read_text(encoding='utf-8')
    if re.search(r'(?:/home/|/tmp/|/Users/|analysis/work/|analysis/ghidra/|file://|[A-Za-z]:\\\\)', text):
        raise SystemExit('Private path in public artifact: ' + file.name)
data = json.loads((root / 'data/progress.json').read_text(encoding='utf-8'))
links = []
def visit(item):
    if isinstance(item, dict):
        if 'url' in item and item['url']:
            links.append(item['url'])
        for value in item.values(): visit(value)
    elif isinstance(item, list):
        for value in item: visit(value)
visit(data)
pattern = r'https://github\.com/xcntroller/HD2-Decomp/(?:commit/[0-9a-f]{40}|blob/[0-9a-f]{40}/(?:[\w.-]+/)*[\w.-]+(?:#L[1-9][0-9]*)?)'
if any(not re.fullmatch(pattern, url) for url in links):
    raise SystemExit('Unsafe repository link in public data')
print(f'Public artifact audit PASS: {len(links)} allowlisted source/commit links; no private paths or disallowed artifact types.')
