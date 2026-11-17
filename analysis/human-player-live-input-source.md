# Actual Player application-input sender — 2026-11-17

OWN branch after3c8fce0. This reconstructs the actual005A3DE0→0053BF90 sender,
not the previously completed0053F9B0 per-frame/local-buffer consumer. It also
binds the ordinary HUD order queries and Script's genuine inactive-cutscene
return. HUM/FRAME/OWN remain open; application installation follows separately.
No original/Windows execution or new desktop claim.

## Same mutable input, state and callback order

005A3DE0 loads current B0 and calls0053BF90 DIRECTLY; there is no native nonPlayer
no-op. The source safety guard requires the actual Player class at that call.
The3492-byte sender publishes BOTH mutable74/78 loans before either NULL gate.
Owned input packets, AI/shared and local snapshots retain value semantics;
HumanFrameInputBuffer can also borrow actual application/parent neutral arrays.
Copies of a loan capture its current256 bytes into an owned value. No polling,
const-cast, alternate input manager or replacement neutral arrays are introduced.

The sender preserves camera suppression and interrupted-action filtering; FPV
toggle and fresh mode/preference reads; actual mouse52/53 and inversion; native
zoom/delayed-mode scales; attached Actor8C/90/C0 policies; normalized direction,
body-relative yaw limits, pitch limits and full target20 publication BEFORE camera
virtual48. Its frame actions and breath callback consume the original latches.
Posture steps are10000→4000→2000→1000 for input4; input5 moves the other way and
admits sprint only through actual flags/conditioning/stamina. Focused raw review
caught the distinction from an unconditional prone store.

Original arguments and fresh state74/78 reloads are deliberately distinct:
view toggle, blocked clears, mouse and attached C0 use the original arguments;
post-callback item/latch/breath consumers reread current state pointers, while
posture still reads original second argument. Aliased callback tests cover this.
The blocked input jump-table bias is21. The actual preserved indices are21–24,
26–31,35–38,46–48,62; the independent original table mask is recorded with raw
addresses. Clearing consumes both actual buffers in reverse64 order.

Math follows explicit FLOAT spills and x87 extended comparisons/normalization,
including stored current yaw versus extended body yaw and native unordered pitch
selection. W lanes are preserved, not synthesized. Native NaNs propagate through
the corresponding mouse math; tests do not claim bit-identical libm/FPATAN ABI.

## Existing owners and remaining positive requirements

Actual World services bind camera, SAME profile inversion/config24C, FPV/mode,
Human DD0 animation/floor and actual Car/mounted input. The fresh slot3 drop gate
now reads actual item/cooldown flags; positive005A2C50/Player0053E9D0 randomized
request production remains explicitly required. It does not call damage-drop or
fabricate queued success. NonCar Actor90 reference-frame lookup remains required
only at that positive selected receiver. Existing Aircraft/type34/conditional
owners keep their prior honest requirements. Those callees receive no new credit.

005A5ED0 now uses actual kind3 recent-change (unsigned game2C<=state1BC), kind1
Player525, otherwise false.005A5F00 uses the same actual kind3 pending-order
owner; other kinds return0.005A5F50's reached Active record reads native0C/10/4A
for in-call HUD display; it creates/retains no task/record and supplies no fake
empty list. Missing B0/invalid positive record retains source safety failures.
Raw Active slots130/134/138 are0052F110/130/150 and agree with the existing owner.

00423EB0 genuinely returns when BYTE824 is zero, before any camera/context/engine
consumer. That actual module method is now available to OWN. Its positive saved
camera828/82C/830, cutscene834 and868/87C/88C owners remain required, with no
invented state producer or successful end flag. No positive cutscene claim.

## Runtime lifetime and verification

ApplicationFrameRuntime owns only the Runtime borrowed by the lifecycle's SAME
coordinator. Candidate construction and coordinator binding precede nonthrowing
publication. It retires coordinator before Runtime destruction, before callbacks/
World retirement, including bind failure, failed close, retry and destruction.
No second scheduler/World/control owner or readiness flag exists.

Native3248 strict ASan/UBSan/leak checks PASS for actual borrowed/value input
buffers and sender policy over16 lifetimes, all pair gates, independent native
switch mask, view/mouse/NaN/Car/attachment, current-vs-original pointer replacement,
latches/posture and failure prefixes. Player/World/Frames endpoints are explicit
doubles. Native2112 PASS uses actual ApplicationMission plus actual Runtime
lifetime policy, with labelled lifecycle/Runtime/World endpoints, constructor/
bind/close failure and retirement/retry. These are not loaded-engine executions.
Strict actual World i386 explicitly instantiates the complete sender, display
queries and Script end entry; compile/link PASS. Final launcher/probe and removed
audit fingerprints are in the [receipt](checkpoints/p22-player-live-input-20261117.json).
Existing unchanged Human/FRAME kernels retain their previous verification scope.

Five frozen client partial bodies only:0053BF90,005A3DE0,005A5ED0,005A5F50 and
00423EB0;005A5F00 gains source binding evidence. No complete promotion, helper/
absent/delegated-engine credit or resolved_source/matrix/application claim.
