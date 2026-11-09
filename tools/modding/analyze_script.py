#!/usr/bin/env python3
"""Read-only structured script CLI; optional explicit installation include context."""
import argparse
import json
from pathlib import Path
from script_client import inspect_bytes, inspect_file
from script_source import InstallationIncludes


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source',help='Local path, or virtual resource path with --installation')
    parser.add_argument('--inspector',type=Path,default=Path('/tmp/hd2-inspect'))
    parser.add_argument('--installation',type=Path)
    parser.add_argument('--resource',help='Mission namespace for original include lookup')
    parser.add_argument('--archives-first',action='store_true')
    parser.add_argument('--language',default='English')
    args=parser.parse_args()
    try:
        if args.installation:
            if not args.resource:parser.error('--installation requires explicit --resource')
            from resource_store import ResourceStore,virtual_name
            store=ResourceStore(args.installation,args.archives_first,args.language)
            context=InstallationIncludes(store,args.resource);name=virtual_name(args.source)
            report=inspect_bytes(args.inspector,store.read(name),'resource/'+name,context)
        else:
            if args.resource:parser.error('--resource requires --installation')
            _,report=inspect_file(args.inspector,Path(args.source).resolve())
        print(json.dumps(report,sort_keys=True,ensure_ascii=True,indent=2))
        return 0 if report['syntax_accepted'] else 1
    except (ValueError,OSError,RuntimeError) as exc:parser.exit(2,f'{exc}\n')


if __name__=='__main__':raise SystemExit(main())
