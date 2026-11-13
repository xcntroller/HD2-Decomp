# Historical P22-PACK dependency reassessment — 2026-11-12

**Historical snapshot: PACK was blocked at14d59d3.** The selected dependencies
were completed through51082a2 and integrated in the [final PACK source closure](p22-pack-source-closure.md).
The revision-specific review below is preserved as evidence.

PACK was blocked. Read-only review of committed
`feature/p22-hum-h8` at `14d59d33b028c3bab1c350cbe7b98bce286fc69a`;
local and `origin/feature/p22-hum-h8` refs agree. PACK HEAD is
`31b87862a49e8c43394ff7469d1a37a607facd6e`. No fetch, merge, source import,
other-worktree mutation or original/Windows execution was performed.

The user's instruction was to finish PACK if HUM's PACK dependencies are resolved.
They are not: the source checks below identify missing positive effects on actual
owners, independently of H8's still-open containing-frame milestone. H8 completion
as a whole is not imposed as a new PACK gate. No partial source import is needed
to establish these failures. PACK's mandatory effect-binding checkbox remains open.

## Resolved or advanced on H8, pending PACK integration

| Command | Committed source available on H8 |
| --- | --- |
| 1 | `human_damage_presentation.h` implements received005A15D0; actual Human services dispatch `present_human_damage`. |
| 3 | `human_common_command.h` supplies shared00586200 subtypes3/5/14/1B/1E/1F and actual producer/receiver policy. Other subtypes still throw. This is partial resolution of PACK's broad control blocker. |
| 10 | Actual services dispatch005A3490 identity drop and005A3750 presentation through `drop_human_damage_item_identity` / `present_human_dropped_item`. |
| 16 | `ControlStartupRecord::human_actor` provides typed access to record3C; `session_peer_records.h` publishes the borrower and `session_peer_lifetime.h` clears it. Services `human_score_rank_label` implements00725B00 through SAME holder4/item definition44; `receive_human_score_rank` preserves Player/Remote rank ordering. PACK's older missing-association guard can be replaced when integrating these owners. |
| 17 | Services dispatch actual `damage_human`005A1C90 and the reaction/retirement graph. |

These are source-availability findings, not fresh integration or sanitizer results.
H8's retained receipts describe its verification limits. PACK's prior rank helper
is separate from H8's score implementation; eventual integration must reconcile
the existing PACK behavior and lifetime checks rather than blindly copying files.

## Still blocking full PACK source closure

At this exact H8 revision, `recovered/game/human_world_loading_services.h`
contains explicit missing-effect exceptions for every family below:

| Command | Missing effect / original entry | Supporting owner check |
| --- | --- | --- |
| 2 | Active clothing005A6930/005A6A00 | `PreparedHumanClothing::apply_prepared_human_clothing` still rejects active DD0/Sight/Players refresh. Cold model replacement cannot satisfy packet application. |
| 7/8 | General inventory validate/apply005A2D50/005A2E10 and005A2E90/005A2F50 | `HumanLoadingInventoryCommands` still queues Player/Remote requests, and immediate AI effects support only the verified same-Human loading weapon slots. Cross-Human/general application remains absent. |
| 9 | Item validation005A39D0 and apply005A3A40 | Both authority and presentation cases throw; no matching positive implementation found in recovered game/host source. |
| 11 | Item validation005A37C0 and apply005A38F0 | Both authority and presentation cases throw; no matching positive implementation found in recovered game/host source. |
| 12 | Player action0053D5D0/0053D870 and Remote00594110 | All three receiver variants still throw. |
| 18 | Named special animation005A9C20/005AC4A0/0077CE10 and setup007757E0 | Remote packet case still throws. `HumanAnimationTracks::fade_fourth`00775970 and reaction retirement now supply reset-side work, but not named-animation setup/loading and full packet effect. |
| 20 | Active Remote equipment replacement0059EB60 | `receive_human_equipment_packet` still throws for an actual Remote target, including discard routing. `populate_prepared_human_storage_live` is a cold preparation body; its clothing path retains the active-owner rejection above. |

Command3 additionally retains `Human common DC requires its actual selected
subtype receiver` outside the recovered subtype set. It must not be marked fully
closed merely because its packet dispatcher now calls a typed owner.

## Next work and verification

Human must supply the missing active/general effect contracts above, with actual
owner/producer/retirement paths. Then integrate its committed source into PACK,
reconcile notification/rank helpers, bind all mandatory effects, and perform the
full affected stream/FIFO/module/Human positive/failure/lifetime sanitizer sweep
and concrete i386/application compile/link before checking PACK source closure.
No successful application loading or desktop delivery is implied.

This reassessment changed documentation only. Verification: exact Git source
inspection, original-address searches across recovered game/host source, and
`git diff --check`. No build/sanitizer rerun was warranted for unchanged source;
no new function claims or ledger totals were added. Earlier receipts remain
historical, not rerun results.
