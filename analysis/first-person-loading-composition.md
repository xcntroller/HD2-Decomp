# First-person/player loading composition — 2026-11-10

Source subsystem of the full first P2.2 checkbox. Application loading is still
disabled; no loaded world, first world frame or desktop acceptance. This joins
the existing [first-person view receivers](human-state-loading.md#containing-first-person-view-binding-actions-and-activation-source)
and [Human owners](human-world-composition.md) at their actual shared lifetimes.
[Verification receipt](checkpoints/p2.2-player-first-person-composition-20261110.json).

## Actual preference and player mode

Original006B4A60 allocates a separate one-byte application+0C owner initialized
zero (global pointer008AE908). It is **not** profile settings and **not** live
FPV58. ApplicationStartupData/StartupSession now expose this same persistent
byte. Existing ingame0053BF90 updates it from the actual view after a toggle;
that input path remains separate. There is no guessed default first-person flag.

HumanPlayerModeLoading implements005469E0 over SAME player52C/530, FPV, Items,
SessionActivationState28, process guard, application preference, configuration24C,
gameBC FOV and local player identity. Equal mode returns. Otherwise store52C
BEFORE local-player gate/callbacks. Offline mode0 after anything except1 enables
FPV from the application byte, then calls actual zoom-off with live FOV and clears
530. After1 it disables secondary view, issues actual inline action18hex, then
clears530. Outside7, a nonlocal Human retains the mode store with no callbacks;
configuration24C=0 forces ordinary enabletrue. Required command2 network queuing
on secondary transitions explicitly rejects after the real local callback prefix.

Mode1 searches actual holster stacks reverse, only each first entry, unsigned
clock28>=item14 and exact definition kind14. No match restores previous52C.
A match enables secondary view, evaluates actual item model007D3AA0 and sends
inline action17hex/model. Modes2/3 offline compare expected/observed process
stage41 (WORD8CAB82/8CB282); mismatch restores previous52C without random draw.
A real match enables FPV then uses B0A8's actual primary payload native10
(serialized0C) FOV for zoom. Uninitialized guard remains a required failure;
production does not equalize it. Other modes only store52C, as original.

Static assembly corrects the misleading decompiler command-local names:
005A45F0 adds Human98 and tail-jumps005ABD90. These commands have kind1 at+8
and action17/18 at+C, so they call005ABB50; they are **not default no-ops**.
HumanInlineLoading forwards optional model then action to SAME E0/Look.
0077F710 updates a typed borrowed frame04 under bit10000 gates.0077F050 updates
scalar48 under bit2 gates, independent of equipment54. Each checks D0/D4 or
forced B0, sets40 only when changed and requested, otherwise C8 on rejection.
A null command model retains old04. No host pointer is stored in a DWORD.

## One first-person lifetime and real player lookups

FirstPersonLoadingComposition owns actual FirstPersonModelLoading, HumanItems,
animation SOUND notification, playback and view receivers over the SAME session
OverlayOwner. All borrow the same frames/Items/cache/Players/camera/category3/
SOUND/cursor/Effects and live FOV. Close uses actual00490B50; model lifetime's
existing destructor also reclaims partial prefixes.00493360 reset gates mode2,
closes, activates through actual file/clock/table receiver and only THEN rereads
the same application preference for enable. This does not implement full FPV
simulation/render or construct the containing session.

FirstPersonPlayersLoading now defines the former player-world lookup boundary:
actual current2C, checked per-Human WorldActors composition, SAME inline/B0,
005A8170 player mode and actual sessionB8. It preserves a missing inline versus
an engaged null DD0/CD0 sector. Target retirement resolves the actual Scene2
model100/kind9/EC Human and invokes005A9760 amount0 against its own inline before
camera releases the retained target. The callback captures actual actor/world
owners, surviving facade retirement; camera must clear BEFORE those owners.

Known Car-driver visibility is type4/seat0. Attachment virtualC4 and supported
kind4/20/21/34 virtual8C/bit10 consumers explicitly reject when reached. Other
attachment kinds genuinely do not hide the Human. Cold loading/spawn uses no
attachment. The SAME view's0045DAC0 update_player_visibility is now public for
actual Players0045E5D0 and Human005A5D50 resume, instead of requiring a separate
visibility facade. Full occupied actor control remains outside this boundary.

## Evidence and verification

Original client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Durable ignored WORK=analysis/work/mission-world-20261103 retains
player-view-references.log/player-view-globals.log, player-view-composition-required/
005469e0,005a6730,00493360; human-initial-command-dispatch/005abb50;
player-mode-and-soldier-receivers/0077f710,0077f050;
player-mode-transition.asm, player-inline-command-forward.asm and
player-mode-equipment-forward.asm. Original constructor is retained under
analysis/work/launch-protection-20261030/006b4a60.c. No original binary executed.

Focused native ASan/UBSan/LSan passes actual Human/PlayerState/base/InputManager/
holders/stacks/Look, cold/repeat mode0, reverse cooldown/holster/model/action,
stage41/modes2/3 FOV, callback failure/store ordering and source player lookup/
sector/retirement callback lifetime/identity/rejection. FPV/Items/clock/frame and
world composition lookups are explicit doubles. Strict i386 composes actual
SessionOwner overlay/FPV model/item/animation/Effects/camera, real Players control
and WorldActors checked map/Scene2 lookup, actual state and inline/Look action.
Remaining DD0/equipment/containing composition/SOUND/cursor endpoints are
explicit declarations. FPV composition/reset itself is compile-only here;
existing model/view native verification is recorded in the linked contract.
Application compile/link and27 tracker checks pass. Four temporary artifacts
hashed/removed. Six new partials, no complete promotions. Source wrappers and
composition adapters receive no invented original-function credit.

Continue the full checkbox: actual player presentation/HUD soldier/compass
binding/removal, remaining containing Human/FPV/SOUND/session definitions,
ordered loader/activation/first frame, required guard observed publication and
failure/repeat/user acceptance. No successful missing service is installed.
