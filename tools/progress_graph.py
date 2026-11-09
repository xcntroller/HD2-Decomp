"""Static, layout-free React Flow records derived from analyzer evidence.

Edges include typed grouping/reference relationships as well as dependencies.
No function call edges are synthesized from source filenames or shared receipts.
"""

import hashlib
import json


def graph_export(functions, milestones, checkpoints, first_frame, stages):
    nodes, edges = {}, {}
    unknown = []

    def unknown_levels():
        return {stage: {"status": "unknown", "provenance": []} for stage in stages}

    def node(key, kind, label, status, provenance, levels=None, **details):
        nodes[key] = {"id": key, "type": kind, "data": {
            "label": label, "status": status, "provenance": provenance,
            "verification_levels": levels or unknown_levels(), **details}}

    def relationship(source, target, relation_type, provenance, *, basis="explicit",
                     evidence_state="verified", **details):
        # Identity depends on semantic endpoints/type, never input row order or status.
        identity = json.dumps([source, target, relation_type], separators=(",", ":"))
        key = "relationship:" + hashlib.sha256(identity.encode("utf-8")).hexdigest()
        value = {"id": key, "source": source, "target": target,
                 "type": "dependency" if relation_type == "depends_on" else "relationship",
                 "data": {"relationship": relation_type, "basis": basis,
                          "evidence_state": evidence_state, "provenance": provenance, **details}}
        if key in edges:
            raise ValueError(f"Duplicate graph relationship: {identity}")
        edges[key] = value
        return key

    def function(program, entry):
        key = f"function:{program}:{entry}"
        if key not in nodes:
            node(key, "function", f"{program}:{entry}", "no_ledger_claim", [],
                 program=program, original_entry=entry, inventory_membership="unknown")
        return key

    def unresolved(source, descriptor, relation_type, provenance, reason, *, basis="unknown"):
        identity = json.dumps([source, descriptor, relation_type], separators=(",", ":"), sort_keys=True)
        unknown.append({"id": "unknown:" + hashlib.sha256(identity.encode("utf-8")).hexdigest(),
                        "source": source, "target": None, "relationship": relation_type,
                        "basis": basis, **descriptor, "provenance": provenance, "reason": reason})

    for row in functions["entries"]:
        key = function(row["program"], row["entry"])
        levels = unknown_levels()
        levels["source_recovery"] = {"status": row["status"], "provenance": [row["record"]],
                                     "scope": "Original-function reconstruction claim only."}
        node(key, "function", f"{row['program']}:{row['entry']}", row["status"], [row["record"]],
             levels, program=row["program"], original_entry=row["entry"],
             inventory_membership="unknown", coverage=row["coverage"],
             limitations=row["limitations"], evidence_links=row["evidence"])

    for row in milestones["items"]:
        key = "milestone:" + row["id"]
        node(key, "milestone", row["title"], row["reported_status"], [row["record"]],
             acceptance=row["acceptance"],
             status_scope="Reported milestone boundary, not original-function completion.")
        for index, seed in enumerate(row["function_seeds"]):
            target = function(seed["program"], seed["entry"])
            relationship(key, target, "original_function_seed", [{
                **row["record"], "json_pointer": row["record"]["json_pointer"]
                + f"/original_function_seeds/{index}"}, seed["reference_record"]])
        for index, original in enumerate(row["references"]):
            provenance = [{
                **row["record"], "json_pointer": row["record"]["json_pointer"]
                + f"/reference_addresses/{index}"}, original["record"]]
            if original["reported_inventory_membership"] == "reference-only-no-standalone-counted-entry":
                unresolved(key, {"program": original["program"], "original_entry": original["entry"]},
                           "original_address_reference", provenance,
                           "Index reports a reference-only address with no standalone counted entry; "
                           "no function node synthesized.", basis="explicit")
            else:
                relationship(key, function(original["program"], original["entry"]),
                             "original_address_reference", provenance)
    for row in milestones["dependencies"]:
        relationship("milestone:" + row["prerequisite"], "milestone:" + row["dependent"],
                     "depends_on", [row["evidence"]], scope="milestone")

    milestone_key = "milestone:" + first_frame["milestone"]
    if milestone_key not in nodes:
        raise ValueError("Dependency catalog milestone absent from phase index")
    for row in first_frame["nodes"]:
        key = "subsystem:" + row["id"]
        levels = unknown_levels()
        levels[row["stage"]] = {"status": row["status"], "provenance": row["evidence"],
                                 "scope": row["claim"]}
        node(key, "subsystem", row["title"], row["status"], row["evidence"], levels,
             claim=row["claim"], evidence_state=row["evidence_state"],
             catalog_claim=row["catalog_claim"])
        relationship(milestone_key, key, "contains_subsystem", [row["catalog_record"],
                     {"path": "tools/progress-dependencies.json", "json_pointer": "/milestone"}],
                     scope="Catalog grouping; not an original call dependency.")
        for entry in row.get("original_entries", []):
            relationship(key, function(first_frame["function_program"], entry),
                         "original_function_reference", [row["catalog_record"], {
                             "path": "tools/progress-dependencies.json",
                             "json_pointer": "/function_program"}, *row["evidence"]],
                         evidence_state=row["evidence_state"],
                         scope="Documented address reference; no additional reconstruction credit.")
    for row in first_frame["dependencies"]:
        relationship("subsystem:" + row["prerequisite"], "subsystem:" + row["dependent"],
                     "depends_on", row["evidence"], basis=row["basis"],
                     evidence_state=row["evidence_state"], kind=row["kind"],
                     rationale=row.get("rationale"), scope="first_mission_frame")

    # Scope-only checkpoint membership is a group, never a function call edge.
    for row in checkpoints:
        key = "subsystem:checkpoint:" + row["path"].removeprefix("analysis/checkpoints/").removesuffix(".json")
        levels = unknown_levels()
        for observation in row["observations"]:
            level = levels[observation["stage"]]
            level["status"] = "reported"
            level.setdefault("reports", []).append(observation["reported"])
            level["provenance"].append(observation["record"])
            level["scope"] = row["scope"]
        if row["source_only"] is True:
            levels["source_recovery"] = {"status": "source_only_checkpoint",
                "provenance": [{"path": row["path"], "json_pointer": "/source_only"}]}
        if type(row["desktop_verified"]) is bool:
            levels["desktop_acceptance"] = {
                "status": "reported_verified" if row["desktop_verified"] else "reported_not_verified",
                "provenance": [{"path": row["path"], "json_pointer": "/desktop_verified"}],
                "scope": row["scope"]}
        node(key, "subsystem", row["path"].split("/")[-1], "recorded_checkpoint", [row["record"]],
             levels, scope=row["scope"], date=row["date"])
        for index, entry in enumerate(row["original_entries"]):
            candidates = [key for key, n in nodes.items() if n["type"] == "function"
                          and n["data"]["original_entry"] == entry]
            provenance = [{"path": row["path"], "json_pointer": f"/original_entries/{index}"}]
            if len(candidates) == 1:
                relationship(key, candidates[0], "checkpoint_function_scope", provenance,
                             basis="inferred",
                             rationale="Receipt lists an unqualified address; joined to the unique "
                                       "program-qualified node with that address in this snapshot.",
                             scope="Receipt address association only; not per-function verification.")
            else:
                unresolved(key, {"original_entry": entry}, "checkpoint_function_scope", provenance,
                           "Receipt address has no unique program-qualified node; "
                           "no target or function credit synthesized.")

    for edge in edges.values():
        if edge["source"] not in nodes or edge["target"] not in nodes:
            raise ValueError("Graph relationship has an absent endpoint")
    groups = []
    for key in sorted(nodes):
        members = [e for e in edges.values() if e["source"] == key and e["data"]["relationship"]
                   in ("contains_subsystem", "original_function_seed", "original_function_reference",
                       "checkpoint_function_scope")]
        if members:
            groups.append({"id": key, "member_ids": sorted({e["target"] for e in members}),
                           "relationship_ids": sorted(e["id"] for e in members)})
    return {"nodes": [nodes[key] for key in sorted(nodes)],
            "edges": [edges[key] for key in sorted(edges)], "groups": groups,
            "unknown_relationships": sorted(unknown, key=lambda row: json.dumps(row, sort_keys=True)),
            "dependency_direction": "prerequisite -> dependent",
            "group_direction": "group -> member/reference",
            "layout": "No positions assigned; node data and typed edges are layout-independent.",
            "unknown_call_graph": "Function-level dependencies are unknown. Ledger evidence links, "
                                  "seed lists and shared verification receipts do not prove call edges."}
