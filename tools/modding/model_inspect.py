#!/usr/bin/env python3
"""Deterministic read-only 4DS inspection from a local file or exact installation resource."""
import argparse
import json
from pathlib import Path
import sys
from model_resources import ModelResources,open_local
from resource_store import ResourceStore


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('model',help='Local path, or exact virtual resource with --installation')
    parser.add_argument('--installation',type=Path)
    parser.add_argument('--archives-first',action='store_true')
    parser.add_argument('--language',default='English')
    parser.add_argument('--output',type=Path,help='New report outside input folders/installation; default stdout')
    args=parser.parse_args()
    try:
        asset=ModelResources(ResourceStore(args.installation,args.archives_first,args.language)).open(args.model) if args.installation else open_local(args.model)
        encoder=json.JSONEncoder(sort_keys=True,indent=2,ensure_ascii=True,allow_nan=False)
        if args.output:
            if any(args.output.resolve().is_relative_to(root) for root in asset.protected_roots):raise ValueError('Export outside input folders/installation')
            with args.output.open('x',encoding='utf-8') as stream:stream.writelines(encoder.iterencode(asset.metadata()));stream.write('\n')
        else:sys.stdout.writelines(encoder.iterencode(asset.metadata()));sys.stdout.write('\n')
        return 1 if asset.model.status in ('invalid','unsupported') else 0
    except (ValueError,OSError) as exc:parser.exit(2,f'{exc}\n')


if __name__=='__main__':raise SystemExit(main())
