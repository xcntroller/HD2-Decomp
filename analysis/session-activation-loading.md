# Offline session activation and prepared spawning — 2026-11-09

This is source recovery within the FULL P2.2 loading checkbox. It does not enable
GameApplicationState loading or establish a rendered world/desktop acceptance.
The containing host still needs actual world/VM/actor/physics/presentation wiring.
[Saved checks and explicit body selections](checkpoints/p2.2-prepared-spawn-20261109.json).

## Containing activation

`activate_offline_session` recovers006989B0's ordinary offline7 policy. It consumes
the SAME owned mission descriptor/live squad008AE910, distinct from HumanStore
008AE90C. The SAME loader must publish loaded14; returning true without its stages
is rejected. Existing graphics flag scope restores on failure. Car definitions,
tyres, materials and particle calls retain the original ignored-result behavior.
World quantizers precede light filtering, optional actual face initialization,
authority modules, HUD, presentation modules, then first-person initialization.
Progress5 retains .05/.1/.35/.5/.8/.9; state5 and timer500 precede squad spawning.
Spawn lookup is repeated for each active live soldier. Dubbing/text/music/clock
and optional application context retain their final ordering. Wider nonoffline
transport, protection ABI and actual host service composition remain partial.

0069BFF0/0069C080 keep borrowed LIGHT pointers. Restore visits them in reverse,
sets visibility true without update and clears while retaining allocation.
Collect restores first, enumerates actual scene mask2, and hides only E0 bit1
frames whose signed type164 is1..4 or8 and flags160&9 iszero. Hide/update precede
append; failure preserves prior writes. Destruction does not invent callbacks.

0072D2B0 mode3/0072D5F0 derive bounds from actual scene1F8 VISUAL mask1 lazy world
boxes+58. Minimum expands by .05, range by .1, with the original Z intermediate
FLOAT store.0072D900 truncates range*100 through signed64/EAX then grows powers
from2/bits1. Source rejects malformed ranges that would overflow the native loop.
0072F500 mutates session20's SAME stable profile, binds primary then secondary to
all actual input records (not embedded30), writes process table00982DE8 index0,
then publishes managerD4. Steps are .01/.0013/.01, word70=100.505C/offline player0
and the three seven-DWORD00724EC0 buffers reset only afterward. A genuinely absent
manager follows the native no-op branch, never constructs a substitute manager.

## Prepared Humans and equipment

00475940 searches active Scene2 actors ascending for the first type10, matching
team90 and flag9C.004759A0 returns the first free reserved slot0..31 or-1.00611C90
clears spawn9C before allocation, reads actual frame matrices separately and
normalizes direction with extended unit comparison and stored-FLOAT square root.
The default clothing producer uses actual006A3080 mask2;006A2C90 splits carried
records in reverse original-record order, appending before decrement and retaining
the first record's position and all bookkeeping.

00612A60 creates actual kind9, loads category1 into it, links to scene1F8 with
flag0, writes position/direction/name `_PLAYER_%2d`, updates and registers the
actual Scene2 Human. Factory reference release precedes property setup. The
model key uses actual clothing resource34 and keeps the final dot while removing
its suffix. `NA01` is the original008397A4 fallback; failed loads do not succeed.
0059EDC0 writes kind/personality/profile, releases old speech and acquires the
live speech48 variant1..4 or rand%4+1. NULL speech is retained. Explicit stats copy
all12 words; profile conversion changes only nine, retaining the final three.
The twelve-word reset copy precedes inventory; profile eye texture follows it.

0059EB60 resets storage007DDEF0 and actor weight before item creation. Physical
fixed order4C/52/58/5E/64 maps UI indices0/1/3/4/2 to storage0/4/5/2/3. Clothing
replacement follows fixed entry52, before later pack/weapons; holsters then
backpack are ascending records. Unsplit counts still create one instance per
record.007D7810 uses actual catalogue/RuntimeItem/identity registry and cold
007D8B40 storage transfer. Enabled module notifications set Human178 and hand
cooldown; all successful items expire at game28+920. No active displaced-item
or B0/A8/HUD transfer branch is invented. Storage reset detaches borrowed pack6
before owned slots0..5, clears holder timers and preserves stack flags.

005A6930 looks up the SAME Human's actual item identity, retires real model
speech and calls0048A780. At nonzero game28 its shortcut is **strstr(resource34,
model134)** (007E5A40), not strcmp. Missing inventory returns false; effects
failure is deliberately ignored before resolving marker `f` (008390EC) and178.
The source policy explicitly rejects an active DD0/B0/Sight/Players Human; those
live clothing refresh branches remain required when implementing active changes.

0048A780 preserves the SAME original model pointer. It captures the actual eye
material via0048A3F0 plus two further lazy resolves, loads a temporary kind9,
links to the original parent and copies position/rotation/name. Actual jitter
is rand*6/32768 minus **4** (0080F9F8), Z rounded first; it is not minus3. Eight
animation lanes transfer reverse, retaining weight/flags/time and disabled-lane
removal.00489FD0 copies kind7's native120/128 pointer links through matching short
names and virtual74;0048A170 moves foreign children by original model100 ownership
and parent short name, retaining LOCAL position/quaternion. Both run before and
after loading the new resource into the original model. Temporary unlink/release
precedes updates and eye restoration. Engine endpoints remain delegated, with
no new engine function credit. Native failure writes are not rolled back into a
fictional successful model; a cleanup guard retires temporary engine references.

## Local spawn packet production

0073CBD0/0073CCF0 own a separate ten-slot process quantizer table, seeded only0..3
with the original constants. It is not history's three angle quantizers.
0073CC40 grows once by two and copies a profile; source rejects unproduced slots
instead of manufacturing zero profiles.0073D3A0/0073D3F0 retain explicit-index,
override-pointer, then default-index precedence; defaults are position0/angle1.
0073D280/0073D2F0/0073D370 grow real owned buffers: first100 bytes, equality also
grows, then native bit-capacity doubling. Unwritten padding is deterministically
zero in source and has no semantic/native-byte equivalence claim.0073D440 writes
unmasked quantized X/Z, clamps only Y to its minimum and compresses near-zero Y
with one flag.0073D6C0 uses yaw atan2(z,x), pitch asin(y), then mask3.0073D070
retains signed CHAR promotion and the writer's unmasked spill.

00611C90's normal game1C=0 branch still publishes a local packet in offline7.
0072E940 updates the SAME ControlStartupState records under the real recursive
session lock, retaining unspecified bytes.00475530/0072BEB0/0072BEF0 begin module13,
priority0, target0 and prepend spawn actor identity10bits. Fields then are subtype0
2bits, caller player0 8bits, allocated Human identity8bits, quantized position,
direction, signed Scene2 generation004754F0 8bits, local record name/value, then
00727480. Equipment00727450 writes signed position16/item16/count8 unmasked;
00727480 retains fixed physical order, carried counts, eight raw bookkeeping
DWORDs and native -1→255/1023 mutations. Soldier98 now retains its actual pack
volume-limit producer separately from used volumeA0.006739F0 scalar defaults are
recorded without new factory-body credit.0072BF90 target0 clears active1C and
**leaves the cursor pending**; messages append. It does not enqueue by itself.

Actual flush0072C5D0 must append terminator19, enqueue via0072C230 and process the
presentation queue0072C3B0/0072C0B0 through module13's real decoder and006123A0.
The separate game1C=1 direct branch requires that SAME receiver. These dispatch,
camera commands and first simulation00469300 are the next dependencies; the
production source does not replace them with successful defaults.

## Focused verification

ASan/UBSan/LSan pass for actual Human/RuntimeItem/storage and captured resolved
item catalogue, stack split, profile/speech/eye order, cold transfer expiry and
clothing model/animation/link/attachment transfer, resource shortcut, active/
missing-eye guards and cleanup. A second native fixture verifies actual input
manager/history profile rebinding, global defaults/growth, complete offline packet
field decoding, native signed spill, pending appends and recursive direct lock.
Activation/light/bounds fixtures preserve29 injected failures and graphics restore.
Frame/cache/audio/presentation/activation boundaries use doubles; these are not
desktop verification. Strict i386 actual host/Items/Human/Effects/Dubbing/Control
composition and application compile/link pass. No original client/DLL, Proton or
Windows fixture was executed. Temporary fixtures are hashed and removed at this
subsystem checkpoint; unchanged subsystems were not broadly re-audited.
