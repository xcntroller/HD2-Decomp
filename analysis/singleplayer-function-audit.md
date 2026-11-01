# Single-player function ledger audit — 2026-11-01

This audit corrects the omitted ledger update for the integrated preparation/team
implementation in commit `13863a7`. Each entry below is a distinct internal
non-thunk client entry in the frozen client inventory, SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`; the linked
source contains the described behavioral subset. Existing entries are extended,
not counted twice. Research-only addresses, call sites, absent/thunk functions,
unimplemented game receivers and delegated engine bodies are excluded. All new
entries are **partial**, with no complete-function claim. See the ledger for
per-entry limitations. Desktop acceptance applies only to the bounded team
checkpoint; loadout screens are completely untested until accessible.

New partial entries: **90**. Existing entries extended: `00623DD0`,
`00655820`, `00655F10`, `006C20F0`. Original evidence, route contracts and
[prior native/sanitizer verification](singleplayer-pipeline.md) remain separate
from full end-to-end or desktop delivery claims.

| Original entry | Supported subset | Reconstructed source |
| --- | --- | --- |
| `0067D280` | Preset default equipment identifiers and quantities. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `0067D660` | Version-dependent preset record decoding and carried equipment arrays. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `0067E350` | Mission policy version header, active slots, assignment triplets and required items decoding. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007DC4E0` | Lazy item catalogue initialization and load gating. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007DC500` | 500-slot item catalogue loading and recognized/unknown class record traversal. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007DFCF0` | Common serialized item resource names, properties and typed payload decoding. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007E04B0` | Serialized class-zero item derived-field decoding. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007E0620` | Serialized class-one item derived-field decoding. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007E0750` | Serialized class-two item derived-field decoding. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007DFAB0` | Common item and nested-payload size calculations used to locate derived fields. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `007DF950` | Equipment slot masks derived from real item class, kind and category. | [pre_mission_data.h](../recovered/frontend/pre_mission_data.h) |
| `0067D930` | Version-three preset serialization and profile-owned preset write request. | [pre_mission_resources.h](../recovered/frontend/pre_mission_resources.h) |
| `0067DBF0` | Four groups of six default user preset captions and category masks. | [pre_mission_resources.h](../recovered/frontend/pre_mission_resources.h) |
| `0067E200` | Initialize mission policy slots using actual loaded game roots and mission counts. | [pre_mission_resources.h](../recovered/frontend/pre_mission_resources.h) |
| `0067A0F0` | Assign policy equipment bundles to soldiers using shooting, weight and weapon preferences. | [pre_mission_auto_equipment.h](../recovered/frontend/pre_mission_auto_equipment.h) |
| `006736D0` | Initialize subtype-three mission stock from descriptors and associated ammunition. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006743F0` | Check equipped and carried firearms for each active soldier. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `00677620` | Apply preset equipment through the actual store, slot and capacity operations. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `00677D00` | Capture fixed and carried equipment into original preset field order. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `00678AC0` | Find the first missing required item and apply squad-size requirement clamping. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `00679060` | Count required-item holdings and calculate surplus with retained restricted-holder state. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `00679490` | Return surplus optional equipment while preserving required items and their containers. | [pre_mission_flow.h](../recovered/frontend/pre_mission_flow.h) |
| `0067BDA0` | Return removed soldier equipment quantities to the store. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `0067BDD0` | Withdraw equipment from stock and return rejected, residual or swapped items. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A2330` | Compute absolute load-to-integer-capacity ratio for a live soldier. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A2340` | Assign a HumanStore identity and Might-derived carrying capacity to a live soldier. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A27C0` | Clear equipped slots, carried vectors and derived load bookkeeping. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A3080` | Transfer equipment to fixed and carried slots with item compatibility and capacity handling. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A3990` | Remove carried equipment and update quantity and derived load bookkeeping. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A3BC0` | Remove equipment by slot with fixed-slot and carried-item handling. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A5AA0` | Add quantities to the indexed equipment store. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A5B20` | Withdraw the accepted quantity from the indexed equipment store. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `006A5C30` | Query indexed equipment stock and return zero for unavailable entries. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `00671200` | Check item-slot compatibility and occupancy before equipment transfer. | [pre_mission_inventory.h](../recovered/frontend/pre_mission_inventory.h) |
| `00675900` | Select the secondary equipment store for campaign subtype-four preparation. | [pre_mission_flow.h](../recovered/frontend/pre_mission_flow.h) |
| `006777A0` | Run stock-limited ammunition and utility supplementation passes. | [pre_mission_flow.h](../recovered/frontend/pre_mission_flow.h) |
| `00694520` | Remove unavailable team members on entry and return their equipment to stock. | [pre_mission_flow.h](../recovered/frontend/pre_mission_flow.h) |
| `006908B0` | Apply single-soldier eligibility for Lone Wolf and specific standalone missions. | [pre_mission_flow.h](../recovered/frontend/pre_mission_flow.h) |
| `006B6530` | Reset and reload the preparation tree and transient discovery state. | [pre_mission_flow.h](../recovered/frontend/pre_mission_flow.h) |
| `006B8420` | Find a mission node by its ordinal within a campaign, skipping non-mission children. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B8510` | Find a campaign node by its ordinal within a game-data root, skipping non-campaign children. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B8660` | Resolve a flattened mission ordinal into campaign-relative campaign and mission ordinals. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B8A20` | Seek the next campaign or mission descriptor container for a fresh route. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B8AA0` | Advance descriptor traversal past containers to the next preparation/movie/game marker. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B8B20` | Traverse backward across permitted descriptor and container boundaries. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B9190` | Select the current mission descriptor or synthetic standalone mission for the owned request. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B93C0` | Aggregate equipment descriptor records from selected and earlier campaign missions. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `006B9680` | Build standalone mission descriptor data, eligible movies and preparation/game markers. | [pre_mission_route.h](../recovered/frontend/pre_mission_route.h) |
| `0064FEA0` | Decrease a hero stat delta and refund the linked point counter. | [pre_mission_controls.h](../recovered/frontend/pre_mission_controls.h) |
| `0064FF80` | Increase a hero stat delta with its 100-point cap and linked point budget. | [pre_mission_controls.h](../recovered/frontend/pre_mission_controls.h) |
| `00694C40` | Toggle ordered squad slots with candidate eligibility and replacement locks. | [pre_mission_squad.h](../recovered/frontend/pre_mission_squad.h) |
| `006953E0` | Retain up to five best candidates under the original strict score comparisons. | [pre_mission_squad.h](../recovered/frontend/pre_mission_squad.h) |
| `00695450` | Automatically fill squad roles from eligible HumanStore candidates. | [pre_mission_squad.h](../recovered/frontend/pre_mission_squad.h) |
| `00695B30` | Reconcile selected slots with live soldiers, preserving existing equipment and roles. | [pre_mission_squad.h](../recovered/frontend/pre_mission_squad.h) |
| `00691CB0` | Construct the authored team screen, candidate strip, slots and controls. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `00690DE0` | Bind team candidate name and rank to the original text styles and rows. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `006937F0` | Begin portrait drag from a selected candidate or squad slot with a moving visual copy. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `00693B60` | Update dragged portrait coordinates and foreground depth from pointer displacement. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `00693C10` | Apply portrait drop transfer and squad-slot reorder using the original half-screen boundary. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `00694AE0` | Select an eligible candidate and update the team selection display. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `00696740` | Hide unawarded medals and place awarded compact/full medal variants. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `00694640` | Toggle the selected soldier statistics and biography panels. | [pre_mission_team_menu.h](../recovered/host/pre_mission_team_menu.h) |
| `00691840` | Build the team four-photo selection screen and select an available soldier. | [pre_mission_soldier_selection.h](../recovered/host/pre_mission_soldier_selection.h) |
| `00675F30` | Reuse the authored four-photo selection screen for equipment's single-soldier route. | [pre_mission_soldier_selection.h](../recovered/host/pre_mission_soldier_selection.h) |
| `00697600` | Construct hero title, point counter, skill groups, accept and Back controls. | [pre_mission_hero_menu.h](../recovered/host/pre_mission_hero_menu.h) |
| `00697710` | Construct the authored hero specialization controls and their group. | [pre_mission_hero_menu.h](../recovered/host/pre_mission_hero_menu.h) |
| `00697A50` | Construct eighteen hero stat rows, base values and linked point counter. | [pre_mission_hero_menu.h](../recovered/host/pre_mission_hero_menu.h) |
| `00697F60` | Position and register hero stat row controls on cloned frames. | [pre_mission_hero_menu.h](../recovered/host/pre_mission_hero_menu.h) |
| `00674560` | Construct equipment screen control groups and their authored frame bindings. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `006755F0` | Build the equipment preset and assign-to-all controls using actual preset data. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `006761A0` | Build the authored equipment store panel and controls. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `006763B0` | Bind equipment soldier portraits, frames and isolated image materials. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `00676D10` | Construct equipment item and inventory control rows from actual data. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `007DFA10` | Group item kinds/classes into the equipment store categories. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `00652740` | Bind visual store item and ammunition image/text row controls. | [pre_mission_item_row.h](../recovered/host/pre_mission_item_row.h) |
| `00658770` | Bind preparation combo-box header, down button and retained list box. | [pre_mission_choice.h](../recovered/host/pre_mission_choice.h) |
| `0064FA80` | Initialize hero stat controls with authored caption, value and repeat-arrow frames. | [pre_mission_controls.h](../recovered/host/pre_mission_controls.h) |
| `00650730` | Bind actual image-control material bitmap for portraits and item icons. | [pre_mission_controls.h](../recovered/host/pre_mission_controls.h) |
| `00662B50` | Update the authored meter scale from its current stat/capacity value. | [pre_mission_controls.h](../recovered/host/pre_mission_controls.h) |
| `00662DC0` | Handle meter value, range and axis control messages. | [pre_mission_controls.h](../recovered/host/pre_mission_controls.h) |
| `006695C0` | Bind the nonnumbered item/medal description model and text controls. | [pre_mission_description.h](../recovered/host/pre_mission_description.h) |
| `006697A0` | Show and position description overlay after pointer idle with no held button. | [pre_mission_description.h](../recovered/host/pre_mission_description.h) |
| `00669B90` | Derive localized medal title and description captions from medal index. | [pre_mission_description.h](../recovered/host/pre_mission_description.h) |
| `006391E0` | Build the mission selection screen using actual GameData and profile progress. | [single_player_menu.h](../recovered/host/single_player_menu.h) |
| `00639A70` | Construct campaign/mission group and selection rows from real mission data. | [single_player_menu.h](../recovered/host/single_player_menu.h) |
| `0063A010` | Bind mission selection content and preview data to actual rows. | [single_player_menu.h](../recovered/host/single_player_menu.h) |
| `00674A60` | Bind the selected soldier statistics and meter values in the equipment screen. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `006782B0` | Populate required equipment text from actual mission policy and localized item names. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `0067C860` | Build paged visual store rows and bind actual item and ammunition images. | [pre_mission_equipment_menu.h](../recovered/host/pre_mission_equipment_menu.h) |
| `006A13B0` | Write selected-profile soldier names lazily using original username chunk tags. | [human_store.h](../recovered/frontend/human_store.h) |
