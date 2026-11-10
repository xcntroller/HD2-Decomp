# Human selection, visibility and shared shadow — source checkpoint, 2026-11-10

Full first P2.2 checkbox remains open. Application loading stays disabled; no
loaded world, first frame or desktop acceptance. [Receipt](checkpoints/p2.2-human-selection-visibility-20261110.json).

## Original contracts and reconstruction

005A8740 shows only when Human150 is true;005A86F0 hides only when false.
Both call actual GEffects virtual74 before mutating150, then005AC0F0 and
0078B180. Common Human-world services now supply actual selection acquire/
release feedback and HUD marker callbacks. Previously retracted abstract-only
credits for005A8740/005A86F0 now correspond to receiver bodies.

00489180 owns GEffects228's separate borrowed-Human queue. Zero duration sets
the CURRENT model virtual54 immediately and leaves existing queued work intact.
Nonzero signed duration reads model child `a` with mask1, uses FLOAT1/duration,
and updates the reverse-first matching Human or appends; absent child starts
at1. Neither Human nor model is retained.00487240 traverses in reverse, converts
signed elapsed to FLOAT, computes rate*time plus/minus amount with extended
intermediate precision, stores FLOAT and completes show>=1/hide<=0. NaN remains
queued. It rereads the actual Human model each update, and erases in order only
after the model call succeeds. A200ms rate can leave2.235e-8 at200ms; the next
update finishes. Queue update is source-supplied, not integrated into a live
world update. The containing scene clears these borrowers before Human deletion.

005AC0F0 controls actual DD0+A9C shadow-frame virtual30, not SOUND. Physical
suspension formerly used a separate zero-only A9C borrower; it now resolves the
SAME HumanShadowModel owner, including refreshed frames. Animation cold null
initialization verifies this typed owner is empty rather than copying a pointer
into scalar state. Existing0075C2D0/00757DC0 coverage is extended without new credit.

0078B180 gates mode2, walks primary projection list0098AD3C in forward order,
resolves each signed Human identity TWICE using00475E00, and sets matching
projection virtual30 to the inverse of the requested Human visibility. Null
target matches every valid Human.00475E00 checks physical bounds before any
pending-retirement search; a current kind3 Human wins, else pending IDs are
searched in reverse. Actual host lookup validates the Human family.

HumanPresentationShared owns the actual cold empty primary projection list.
Last-user0078A5F0 retires it before the400-word reset and textures: unlink,
scene84 removal, owning-reference release, then row retirement. Runtime
projection/mark production and pool payloads remain unreconstructed; no append
API or successful drawing service is supplied. Secondary marks remain empty on
the recovered cold path.0078A2C0 constructor-only metadata receives no credit.

## Evidence and verification

Original exports are under ignored durable `analysis/work/mission-world-20261103/`:
`human-shared-player-feedback-required/{005a8740,005a86f0}.c`,
`human-feedback-consumers-required/{005ac0f0,0078b180}.c`,
`human-animation-world-dependencies/00758c60.c`,
`human-equipment-visibility-dependencies/00758bf0.c`,
`human-animation-activation/{00757dc0,0078a5f0}.c` and
`physics-effects-fade-lifetime/00487240.c`.
New captures: `human-selection-visibility-targets.json`,
`human-selection-effects-fade-abi.asm`, `human-selection-effects-update-abi.asm`,
`human-selection-show-hide-abi.asm`, `human-selection-projection-owner-refs.asm`
and `human-selection-projection-actor-lookup.asm`.
Read-only Ghidra reports no function at00489180; static original instructions
and vtable008101F0 slot74 are primary evidence.00489180 is absent from the
frozen inventory and receives no credit.

Focused native ASan/UBSan/LSan covers source fade queue and failure prefixes,
live physical shadow lookup, actual selection callbacks, populated projection
consumer via fixture records and registry identity/pending behavior. Native
Human/frame/inline/registry actor endpoints are doubles. Strict i386 checks use
actual host scene/WorldActors/Startup/Players/HUD/inline/physical/shadow types;
containing composition/presentation aggregate boundaries remain declarations.
Application compile/link passes. No Windows fixture or original binary executed.
Temporary fixtures/binaries are hashed and removed; logs remain with the receipt.

Five new client partials, no complete promotions:34 complete/1,894 partial
(1,889 client/five engine)/18,783 frozen. Continue actual Human factory/common
stance/area/Sight receivers, containing world/session/VM and ordered loader.
