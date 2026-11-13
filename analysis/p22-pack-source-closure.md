# P22-PACK source closure — 2026-11-13

**The selected PACK source boundary is complete.** The Human effects that blocked
PACK are implemented on actual source owners and integrated from
`feature/p22-hum-h8` at51082a2. This completes source recovery and affected native
verification, while application loading remains disabled. Full H8 simulation,
mission population, real transport and user desktop acceptance remain separate.
[Receipt](checkpoints/p22-pack-source-20261113.json) · [Checklist](p22-pack-plan.md).

## Dependency takeover and integration

The earlier PACK assessment at14d59d3 was correct for that revision and is
preserved in16919be and [the historical review](p22-pack-hum-h8-review.md).
The user then authorized completing the dependencies in the HUM H8 worktree.
Their contracts/receipts are retained in the merged history:

| Packet effect | Actual source now bound |
| --- | --- |
|1/10/17|Human received damage/event, actual runtime drop and damage/retirement owners from H2; no successful combat substitute.|
|2|[Active clothing](human-h8-pack-effects-source.md), SAME model/inline/DD0/equipment/Sight/Players/camera.|
|3|[Common command policy](human-h8-pack-common-source.md), target/timer/progress, Door key/close, typed focus/loans, paired Humans, document23, immediate DD0 starts and existing seats/ladder.|
|7/8|[General transfer/swap](human-h8-pack-effects-source.md#commands78-general-inventory-source-effect-checkpoint) over SAME Items and source/destination storage; real Player/Remote deferred producers remain.|
|9/11|[Pickup/automatic placement and SupplyBox](human-h8-pack-effects-source.md) admission, actual item creation/removal, ownership handoff and depletion.|
|12|[Mapped action/Players/speech](human-h8-pack-action-resume-source.md) and [HeavyFire16](human-h8-pack-heavy-support-source.md) configuration, requests, lists, sounds/models, particles and same-World blast link.|
|14/16|Existing PACK notification preserved; rank now uses actual typed process3C, Human holder4/item definition44, profile stats and SAME message/console owners.|
|18/20|[Named fourth-track animation and live equipment](human-h8-pack-effects-source.md), including referenced Remote equipment replacement during discard.|
|19|Actual mode-independent received speech and required nonoffline flags1 publication.|
|Other admitted cases|Existing input/buffer, direction, Ladder6/feedback15, snapshot13, request21, Script suspension22 and Remote action23 policies retained.|

The containing host binds actual Human composition, not a second packet-state
owner. Source payload values own their data; queued commands hold identities,
not raw Human/item pointers. Receiver resolution remains fresh at every command,
with independent command20 reference lookup. Missing reserved actors use their
native discard grammar; a retired nonreserved owner cannot silently retarget the
next command to a different actor. Actor/B0 retirement guards and actual Human
loan cleanup remain in use.

## Merge decisions and corrected original contracts

Shared bodies use later original-qualified Human work. The only remaining source
differences from51082a2 are PACK's Script SetOn append rollback and the unified
rank label/formatting integration. PACK's old `human_packet_rank.h` implementation
is retired; its stats and publication behavior is preserved by
`human_score_presentation.h` and actual World services. Record3C is a typed loan,
never manufactured from a DWORD or another Players roster. The label policy is
independently callable and the production method passes SAME Scene2/Items/text
resolvers. Stats still use Player's new rank and Remote's old rank; Player's
preceding54 store still affects demotion classification. Lock0 remains held
through message publication. The existing bounded PACK formatter now handles
rank as well as notifications, copying values before callback-visible HUD work.

The prior PACK contract had four incorrect or incomplete shared assumptions.
Fresh integration preserves the corrected original evidence already retained in
[Human's contract](human-ragdoll-source.md):

- 005B20A7 writes mask15 to[S+08]. After PUSH005B20AB, FIMUL[ESP+0C] reads that
  same15, not the high zero at[S+0C]. Drop10 therefore writes `(speed-1)*.25*15`.
- 0053F370/00577B30 lazily open ordinary priority0 and motion4 priority3, clamped2.
  Motion finalizes before ordinary. In authority FIRST both can alias2BC; native
  nested headers and two terminators are retained. The audit checks exact bytes,
  without pretending that this contrived alias is a valid live protocol.
- Remote00592220 FC publishes only7/8/10, then retires the whole queue after
  success. Filtered commands never reach late preparation/snapshot callbacks.
- Player input0 stores both buffers before queue allocation; allocation failure
  preserves the native stores. Nonreserved Remote authority00583220 is RET4 and
  consumes no command tags. Actual authority Scene2 dispatch preserves it.

Received speech19 is mode-independent. Module13 authority uses actual reserved
versus nonreserved actor receivers. Module16 uses actual HeavyFire dispatch;
existing runtime/version receivers retain their original owner guards. PACK's
transactional Script SetOn producer survives the merge. No unsupported module or
actor gains a successful default service.

## Verification and evidence

The [receipt](checkpoints/p22-pack-source-20261113.json) fingerprints merged source,
logs, compiler/link artifacts and temporary audits before removal. The native
ASan/UBSan/leak audit runs100 lifetimes of actual codecs, quantizers, command
writers/ranges, process streams, SessionLocalPackets, registered
GameLoadingModules/module13 dispatch and HumanPacketReceiver. Human/effect/engine
endpoints are explicit doubles; actual Human effect bodies are covered by their
retained per-family receipts and the fresh concrete host link. The audit checks:

- All23 payload families, independent fixed widths, detailed/optional/raw mode
  branches, byte truncation, every receiver grammar and genuine switch holes.
- Broadcast/motion routing, Remote range filtering, both distinct-stream and
  shared-alias rollback, exact alias bytes, enqueue failure/retry, loaded gates,
  missing/default/replaced module registrations and FIFO ownership after failure.
- Reserved callback destruction/discard continuation, nonreserved destruction/
  next-command rejection, no decoding under queue lock, and independent20 effects
  despite a missing reserved target. Native Remote authority consumes only actor10.

A separate fresh rank/notification suite, in the same native run, uses actual
codecs, ControlStartupRecord3C, rank statistics/label/publication policy, bounded
formatter and HudMessagesLoading queue. It checks the five notification cases,
Player/Remote ordering, skill preservation, clothing alignment/absent clothing/
configuration bypass, messages/colours and Human+B0 plus process-record destruction
while rank publication holds the lock. It repeats100 lifetimes. These checks
exercise source ownership and dispatch, not real rendering or gameplay.

The final strict `-Wall -Wextra -Wpedantic -Werror` static i386 FULL LINK instantiates
actual Human packet/common/World owners, module13 authority/presentation, module16
and actual Scene2 actor activation/deactivation. Final application launcher and
startup probe compile/link pass. Windows binaries are never run. Existing27
tracker tests, exact frozen-inventory/evidence/README checks and diff checks pass.
Temporary native sources/binaries are fingerprinted and removed; no permanent
new tests are added.

## Accounting and execution limits

The merged ledger preserves35 PACK rows absent from the Human branch and
reconciles19 overlapping rows without duplicate credit. No new function credit
is awarded for merging, delegated engine calls, formatter reuse or a copied
helper. Totals are34 complete/3472 partial (3467 client+5 engine); frozen inventory
counts and denominators are unchanged. Historical receipts remain historical.

Full containing Human00755500/00787270/Active decisions, async combat, absent
actor families, conditional nonoffline diagnostic13/feedback15/medical14 and real
transport are still their actual H8/Phase3 requirements. PACK does not replace
them with successful placeholders or claim full H8 completion. Native unsafe
HeavyFire descriptor/writer/empty/side cases and DocumentPlace's zero-bit event
are preserved as described in their contracts. No live resource reachability,
mission population, new rendering behavior or user desktop acceptance is inferred
from native fixtures. Application loading was not enabled; nothing was pushed.
