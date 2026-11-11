#!/usr/bin/env python3
"""Read-only serialized mission composition with deterministic metadata export."""
import argparse
import json
from pathlib import Path
import sys
from resource_store import ResourceStore
from installation import InstallationSession
from spatial_scene import SpatialScene,freeze_session


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('installation',type=Path)
    parser.add_argument('mission',help='Discovered mission namespace, such as tutorial')
    parser.add_argument('--archives-first',action='store_true')
    parser.add_argument('--language',default='English')
    parser.add_argument('--inspector',type=Path,help='Optional script inspector; otherwise script bindings are indexed')
    parser.add_argument('--output',type=Path,help='New metadata report outside installation; default stdout')
    parser.add_argument('--coverage-only',action='store_true',help='Comparable static coverage without full node metadata')
    parser.add_argument('--script-object',help='Bounded script neighborhood of an exact spatial ID or unique frame name')
    parser.add_argument('--relationship-search',default='',help='Filter the script neighborhood')
    args=parser.parse_args()
    try:
        store=ResourceStore(args.installation,args.archives_first,args.language)
        scene=SpatialScene(freeze_session(InstallationSession(store,args.mission,args.inspector)))
        neighborhood=None
        if args.script_object:
            if args.coverage_only:raise ValueError('Choose coverage-only or script-object')
            matches=[n.identity for n in scene.nodes.values() if n.name==args.script_object]
            identity=args.script_object if args.script_object in scene.nodes else matches[0] if len(matches)==1 else None
            if identity is None:raise ValueError('Script object has no unique exact spatial identity')
            neighborhood=scene.script_relations.snapshot(identity,args.relationship_search)
        if args.output:
            if neighborhood is not None:scene.script_relations.export(args.output,neighborhood)
            else:scene.export(args.output,args.coverage_only)
        else:
            sys.stdout.writelines(json.JSONEncoder(sort_keys=True,indent=2,ensure_ascii=True,allow_nan=False).iterencode(neighborhood if neighborhood is not None else scene.coverage if args.coverage_only else scene.metadata()))
            sys.stdout.write('\n')
        # Reports always retain a partial status; this exit code is only a
        # successful inspection/export, never a world-compatibility assertion.
        return 0
    except (ValueError,OSError) as exc:parser.exit(2,f'{exc}\n')


if __name__=='__main__':raise SystemExit(main())
