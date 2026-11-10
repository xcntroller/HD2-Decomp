# Session loading consumers — source checkpoint, 2026-11-10

Full first P2.2 checkbox remains open. No application loading/first world frame/
desktop delivery. [Receipt](checkpoints/p2.2-session-loading-consumers-20261110.json).
Includes [ShotMgr](shot-manager-loading.md); builds on [Human/VM composition](human-vm-world-composition.md).

## Actual activation APIs

Fingerprint-checked original return-address mappings are preserved in WORK
`session-activation-api-resolution.json` and `session-activation-api-boundaries.asm`.
00698AAE resolves GetTickCount before activation progress5=.05.
00698C09 resolves IsWindowVisible after progress5=.8; native GetIGraph and graph
virtual48 supply the actual window.00698C5D resolves GetCapture immediately
before status5/timer500/spawning.006E9EC2's initializer seed is GetTickCount.

`activate_offline_session` now invokes these actual payload endpoints in that
order. `GameLoadingQueries` also supplies loader GetParent006CA913,
IsWindowVisible006CAAC0 and IsIconic006CAB29. It validates the real GetIGraph
export and queries graph/window afresh for window-bearing calls; GetCapture
has no window lookup. Zero/false/null API results do not fabricate a load failure
or success. Required stages still decide loaded status independently.

These calls do NOT implement private integrity-table side effects or publish
raw API results into guard tables. Startup tables remain uninitialized; no
production equality or successful protection substitute is introduced. The
original-address/per-thread VM remains outside accepted startup scope.

Focused native ASan/UBSan/LSan runs actual activation policy with explicit file/
actor/module/presentation/API doubles: payload order relative to progress and
status publication, each of the three query exception prefixes, graphics flags
restoration, failure-before-query and fresh-owner repeat. No native loaded-world
claim. Strict actual i386 query endpoints compile; Windows APIs not executed.

## Actual mission effects file/cache/scene lifetime

`MissionEffectsLoading` supplies00445D90's actual file boundary using the live
resolver, Startup's shared EffectDefinitions cache and0089A8F4 borrowed scene.
Publish the SAME actual world scene after collision loading, before effects;
load rejects missing/foreign/changed publication. Existing nonempty definitions
skip reopening tables/effects.def. A missing definition/mission file diagnoses
and returns as in the original. The existing version201 reader handles the
actual mission file, resolves a provided frame name through scene7C maskFFFF,
and preserves00445AF0's invalid-definition return. A valid populated effect
still rejects until the real particle/resource producer exists. Version101
remains unsupported. No particle instances or successful drawing service exist.
Close retracts only this published scene while retaining process definitions.
The later [module17 lifecycle](module-loading-lifetimes.md) now implements the
separate initialized finish that clears those definitions via00445980.
The containing world must close it before scene reset/retirement.

Native sanitizers cover both captured empty201 roots, named populated-producer
rejection, invalid definition, cache persistence, missing/legacy/short reads,
foreign scene, retirement and fresh-owner repeat. Frame/file endpoints doubled;
`assets/tables_effects.def` is the existing BASE definition fixture, not a claim
that it is the runtime-selected override. Production reads live files.

Original00444140/004464A0 exports are in loader-module-composition/;
00445D90 in loader/;00445AF0 in module-loading-tail/. Existing parser audits
were not repeated broadly. Strict i386 composes actual file/frame/process owners
with the complete Human/VM source graph; application link and27 tracker tests
pass. Temporary fixtures/binaries hashed and removed. No original/DLL/Proton/
Windows fixture execution and no desktop claim.

Six new ShotMgr partials; loader/effects source coverage extended. Totals34
complete/1,900 partial (1,895 client/five engine)/18,783 frozen. No complete
promotions. Remaining actual module receivers, containing session/ordered loader,
integrity observed publication, first camera/update/render and failure/repeat
remain required before user desktop acceptance.
