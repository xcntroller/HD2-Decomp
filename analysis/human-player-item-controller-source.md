# H6 Player item controller source — 2026-11-12

H6 remains OPEN. This checkpoint completes held-item, shot-math and item-
notification/dry-fire source, and reconstructs the positive shot coordinator.
It does not supply the required item24/28 projectile receiver or close the full
Player frame. [H6 plan and remaining D ownership](p22-hum-h6-plan.md).
[Receipt](checkpoints/p2.2-human-player-item-controller-20261112.json).
Based on H2 `85593e8`; no active H4 source imported.

## Original contracts and ordering

- `00542E20` tests538 before touching the Human. Signed wrapping53C accumulation
  clamps only ABOVE1000 and clears Human17C/180/184. Suppression cancels without
  arming540. Action13 release tests FIRST input40, obtains the actual kind9
  holster, publishes charge98/model/action, notifies16, requests FPV21 and sets
  SAME hand holder18 to game28+1000. Action6/7 release tests FIRST18/1C and a
  current kind9, notifies14/15, then sets holder18 to+1500. Missing selected item
  still clears held/title and arms540=2500; an unreleased action returns true.
  A required failure retains the earlier prefix. The holder deadline is
  `007D68B0`, not weapon14 or a DD0 timer. Definition14 is the existing caption.
- `00544D20` has ECX direction, EDX first basis, one stack second-basis pointer.
  It uses three normalization/cross stages, native FLOAT stores and XYZ only;
  the original's unwritten W receives no initialization/ABI credit.
- `00545140` reads primary payload virtual10 BEFORE its offline guard. Signed
  game3C>=9 and integrity stage100 mismatch force50; otherwise zero returns.
  Three LS3DF draws, separate basis stores and the zero-vector normalization
  fallback are preserved. Final X remains extended for the last squared length;
  Y/Z offsets store FLOAT before addition. No CRT draw replaces LS3DF.
- `005454D0` observes console6/difficulty before actual Player EC=`00542B10`.
  Native kind factors, secondary payload exception, zoom, `005A62A0` stamina and
  offline difficulty follow in order. Each draw publishes one B0+10 component
  BEFORE the next draw; a later failure preserves those stores. The absent EC
  getter uses the same flags/FC metric previously recovered for the HUD.
- `00542920` handles command2's >500 first-shot notification,534=10000 and
  alignment-30; command5 resets zoom before cached-A8 payload duration and530
  subtraction, then falls through1;8/9/14/15/16 align-10. Only AFTER local effects
  does non7 allocate kind0 and append to the SAME input Record. Scalars/vectors
  retain native30=second/40=first, signed68/100,54/58/5C/60 and actual managerDC
  timestamp ordering. This is a pending producer, not admission/live delivery.
- First-shot singleton virtual24=`00459360` walks the real roster in reverse,
  skipping selected/dead actors. Actual active-AI mode0 stores1 then runs
  `00538640`; it does not call broader005380C0. Source rejects other B0 families
  or an unproduced active mode instead of blindly reading the native layout.
- `007D4E90` reuses process0098AFE0. Bounded007D45B0 proves CRT initialization
  by004374D0. Dry-fire changes only position/group/entry/source/model/flags;
  ranges/scales/volume remain. Existing007D4850 reload now shares that SAME
  request and explicitly resets its three position components as the original
  does. The request belongs to Startup and survives World replacement.
- The empty branch of `005433B0` retries actual reload on a trigger. Otherwise
  signed shot age stores FLOAT BEFORE the single random threshold. Equality
  emits; unordered returns. SOUND precedes the cached item reread/58 store,
  FPV7 and command13. Reload5 reads raw loaded50, not amount after its time gate.
- Positive005433B0 uses actual muzzle/camera and loaded-World ray/exclusions,
  or the existing FPV vectors, then spread and REQUIRED item24. It refreshes A8,
  applies recoil to B0+10, requests Look action1/FPV2 or3, sets58 from payload18,
  notifies2, resets B0 shot age and observes stage77's conditional CRT/drop.
  Each late A8 read is preserved. Ray exclusion restoration precedes result
  inspection; >9 compares the extended squared length before the FLOAT store.

Existing getters/owners are reused. New C++ helpers, absent00459360/00542B10/
007D45B0/007D68B0/payload getters and delegated engine/cache bodies earn no new
function credit. Five frozen partials are added, two extended, no complete
promotions. New tally34 complete/3250 partial (3245client+5engine).

## Verification and explicit limits

Native C++17 warnings-as-errors ASan/UBSan/LSan PASS100 over actual Player/Human,
input manager/Record, hand-holder and RuntimeItem ownership. Authored catalogue
bytes go through the real parser. Checks cover held/frame dispatch, title/charge/
release/cancel/deadline ordering; a notification failure; basis/spread/recoil
including the centered-draw fallback and partial B0 stores; dry-fire equality/
SOUND failure and reused request fields; actual pending-message ordering/retirement;
shot coordinator, item disappearance, retry, recoil failure and missing Record
failure after local mutations. Items/event pool outlive Humans; input profiles/
callbacks outlive records; retained frame counts balance at teardown.

Inline/Look, FPV, camera/engine observations, first-shot callback and the selected
item24 projectile effect are explicit boundary doubles. The positive coordinator
check does NOT demonstrate actual projectile production, damage or a complete
World/Player frame. No private-field/access-control override is used. Public
state/input/cooldown values and catalogue data are authored branch inputs.

Strict actual nested-host i386 instantiates held/empty/math/notification, full
005433B0 including the shot coordinator, actual World aim/ray and first-shot/
active-AI signatures. Five other Runtime declarations always throw; compile-only
is not successful installation. Latest launcher/probe rebuild/link PASS. New
native/i386 audit sources/binary/object are fingerprinted then removed; logs and
dedicated Windows snapshots remain. No original EXE/DLL, Proton, Windows output
or desktop execution occurred. H6 full-owner verification remains required.

## Continue the offline path

Next: actual shared item firing and shot-sound borrowers `007D4FD0/007D5210`,
`007D5A80/007D5C70/007D6330`, reached projectile creation and owned retirement;
melee/repair and first command-queue producers; then consolidated actual-owner
verification. `007D5C70` creates SOUND frames, not visual flash models; reuse
SAME category2 resource cache/catalogue. Full asynchronous World frame is H8.

The user prioritized finishing P22-HUM/getting in game. Do not expand DirectPlay
admission, full non7 input-history/interpolation or remote packet replay merely
because those conditional branches exist. Existing required failures remain;
keep these under their selected D/OWN/H8/Phase3 integration cases. H4 stays
external. P2.2 still requires actual application/frame/render/user acceptance.
