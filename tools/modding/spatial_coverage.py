"""Comparable static coverage, not a measurement of runtime/visual equivalence."""
from collections import Counter
import math
from mission import walk_records


def report(scene):
    def mesh(node):
        return bool(node.geometry and node.geometry.lods and node.geometry.lods[0].vertex_count and any(g.triangle_count for g in node.geometry.lods[0].groups))
    omissions=[];reasons=Counter();parents=Counter();materials=Counter();textures=Counter();references=Counter();partial=Counter()
    for node in scene.nodes.values():
        parents[node.parent_status]+=1;why=[]
        if node.world is None:why.append('unresolved_world_transform')
        if not node.display_visible:why.append('serialized_or_ancestor_visibility_off')
        if not mesh(node):why.append('unsupported_or_empty_visual_geometry' if node.kind==1 else 'non_mesh_frame_annotation_only')
        if node.visual_type==6:why.append('visual6_effect_records_have_no_serialized_triangle_mesh')
        if node.visual_type in (2,3):partial['undeformed_skin_instances']+=1
        if node.visual_type in (3,5):partial['animation_tails_not_applied']+=1
        if node.visual_type==4:partial['visual4_runtime_behavior_not_applied']+=1
        if node.model_resource is not None:
            selection=node.model_resolution or {};references[selection.get('status',node.status)]+=1
        if why:
            reasons.update(why);omissions.append({'identity':node.identity,'reasons':why,'status':node.status,'parent_status':node.parent_status})
    assets=[]
    for path,asset in sorted(scene.assets.items()):
        model=asset.model
        materials.update(v.get('rendering_status','not_interpreted') for v in asset.material_preview.values())
        textures.update(t['status'] for t in asset.textures)
        assets.append({'identity':path,'sha256':model.sha256,'status':model.status,
            'declared_frames':model.header.get('frame_count'),'decoded_frames':len(model.frames),
            'unconsumed_records':[{'offset':u.offset,'size':u.size,'reason':u.reason} for u in model.unknown],
            'diagnostics':model.diagnostics})
    lightmaps=[];lights=[];registrations=[]
    for entry in scene.session.report.files:
        for record in walk_records(entry.records):
            if record.kind.lower() in ('0xae00','0xb810','0x40a0'):
                lightmaps.append({'identity':record.identity,'offset':record.offset,'size':record.size,'kind':record.kind,'status':'opaque_not_rendered'})
            elif record.kind.lower()=='0xb820':
                registrations.append({'identity':record.identity,'offset':record.offset,'size':record.size,'kind':record.kind,'status':'scene_frame_registration_not_applied'})
            elif record.kind.lower()=='0x4040':lights.append(record.identity)
    parent_unknown=sum(n.parent_status in ('automatic_sector_identity_unresolved','automatic_sector_unresolved','missing_name_or_scene_path','ambiguous_name','cycle_or_depth_limit') for n in scene.nodes.values())
    discrepancies=[{'identity':n.identity,'name':n.name,'parent':n.parent,'world_position':n.world[12:15],
        'serialized_fallback':n.fallback,'distance':math.dist(n.world[12:15],n.fallback),
        'status':'serialized_fallback_disagrees_not_used_as_placement'} for n in scene.nodes.values()
        if n.world is not None and n.fallback is not None and math.dist(n.world[12:15],n.fallback)>=.02]
    return {'schema_version':1,'scope':'static serialized coverage; counts do not imply visual or runtime fidelity',
        'mission':scene.mission,'inputs':[{'identity':d['identity'],'sha256':d['sha256'],'origin':(d['resolution'].get('selected') or {}).get('origin')} for d in scene.documents],
        'cache_directory_profile':scene.resources.lookup.metadata(),
        'spatial':{'total_nodes':len(scene.nodes),'resolved_world':sum(n.world is not None for n in scene.nodes.values()),
            'unresolved_world':sum(n.world is None for n in scene.nodes.values()),'unresolved_parent_identity':parent_unknown,
            'world_known_parent_unknown':sum(n.world_status=='verified_translation_only_sector_invariant' for n in scene.nodes.values()),
            'parent_states':dict(sorted(parents.items()))},
        'geometry':{'supported_mesh_instances':sum(mesh(n) for n in scene.nodes.values()),
            'placed_visible_mesh_instances':sum(mesh(n) and n.world is not None and n.display_visible for n in scene.nodes.values()),
            'partial_instance_reasons':dict(sorted(partial.items())),
            'omission_reason_counts':dict(sorted(reasons.items())),'omissions':omissions,'assets':assets,'asset_failures':scene.asset_failures},
        'resources':{'model_selection_states':dict(sorted(references.items())),'certainty':'Selections within explicit ordered profiles; warm-cache/global/directory-enumeration state excluded',
            'texture_states':dict(sorted(textures.items()))},
        'materials':{'preview_states':dict(sorted(materials.items())),'lighting':'viewer directional light; no original light consumers',
            'light_records_not_applied':lights,'lightmap_records':lightmaps,'lightmaps_rendered':0,'scene_registration_records':registrations,
            'remaining':['device-dependent color-key quantization','original blend/sector/triangle order','full material flags','lightmaps','skin/animation','original automatic LOD']},
        'actor_definitions':{'total':len(scene.actor_links),'unresolved':sum(a['target'] is None for a in scene.actor_links)},
        'verification':{'world_fallback_matches':scene.counts['world_fallback_matches'],'world_fallback_comparisons':scene.counts['world_fallback_comparisons'],
            'world_fallback_discrepancies':discrepancies,'interpretation':'Fallback is editor/serialized data, not a substitute transform or runtime oracle; disagreements remain visible'}}
