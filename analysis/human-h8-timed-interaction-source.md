# H8 timed Human interaction completion — source checkpoint

00751480 now dispatches every original switch case on the existing DD0/E0,
movement, equipment and twelve-byte action-queue owners. This checkpoint adds
F/11/13/15/17/19/1B/1D/1F/21/25 completion to the previously verified starts,
Door/model and occupied seat requests. Original switch defaults (including
22/23) return AL1 without action effects; no invented Runtime service is needed.
The diagnostic OutputDebugString call and incidental upper EAX bytes are not
reconstructed. This is source coverage, not a whole-function ABI claim.

## Original contracts and ordering

Use `analysis/work/mission-world-20261103/human-actions-explicit-abi-v3/00751480.c`
and matching00755500.c, with the retained original instruction exports
`human-h8-timed-end-{leaves,footers,footer2,queue}.asm` and
`human-action-update.asm`. The ABI-v3 export supplies actual thiscall arguments;
older export stack artifacts are not additional behavior.

D4 locks are masks1/2/8/40/100/200/800 (combinedB4B). Ends15/17 first test
00755470 (actual E0+B8==5), restoring equipment visibility only when false,
then unlock. Other unlocked ends clear the same bits. Queue operations match
only the front record;00756DF0 tests count,00756E00 returns front and00756E30
pops that record. No queue search or wholesale clear substitutes for them.

F and25 optionally retire posture1, then command5/18, then posture2/0 in that
specific branch. If front was already5/18 only that entry retires. F forces
animation45;25 does so only for DD0+A34 zero/one. End11 has distinct mode2/3
handling for front command1: mode2 also retires a following posture2/0, mode3
only the first record. End11 never forces animation. End13 retires command6;
15/17 command8;19 command9;1D equipment event8 before retiring command11;
1F command16. These request animation45 under the actual E0+B0 override,
then clear it. End1B retires command10 and requests animation62 **without
unlocking D4**. End21 retires command17 and returns with no other effects.

All these ends except21 write movement+130=truncate(500 / actual animation
scale). Raw754B02 and755003 establish the missing x87 producer in C. There is
no successful zero timer substitute. Scalar conversion uses the existing native
FTOL low-DWORD adapter. Engine animation/equipment calls stay actual endpoints;
failed callbacks retain the preceding queue/lock/override writes.

00755500 contains explicit switch cases only1..13. Its default is the shared
A38 decrement, retaining A34 even after crossing zero. Zero A34 or A38=-10000
returns before dispatch. The reconstructed default now performs the original
32-bit wrapping subtraction rather than invoking a nonexistent motion service.
Door/model/seat bodies retain their existing policies. Removed the obsolete
00751480/00755500 Runtime requirements; other live requirements are unchanged.

## Verification and limits

Temporary native ASan/UBSan/leak fixture passed **102,824 checks**, exercising
all eleven completion IDs over populated three-record queues, command/mode
combinations, empty queues, B8=5, animation scaling, callback failure prefix,
and all1024 masked frame action IDs over signed timer boundaries. It uses the
actual scalar-word owner and explicit Control/Look/equipment callback endpoints;
this does not execute an actual containing World or engine.

Strict actual World→Human authority/presentation i386 O0 link and launcher/probe
build pass. Windows fixtures are compile/link only. Tracker checks and existing
27 tracker tests pass. Both successful temporary fixture pairs are fingerprinted
in the receipt and removed. No new frozen function count or complete promotion.

H8 remains open. This checkpoint closes the timed interaction source gap; it
does not close remaining specialized Active/Remote/Players/non7/actor receivers,
the full-module Dog/Sight integration gap, application loading or desktop.
