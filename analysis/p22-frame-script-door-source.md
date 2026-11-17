# Reached script DOOR_Lock — 2026-11-17

Source-only correction after8d0d1f2. User contact candidate a27a24a5 completes
activation loaded1/status5, Scene78 render, fixed-update, input and full update.
A later script command154 causes handled result1; no access violation appears in
that run. Stable game/scene/control/presentation acceptance remains separate.
Exact executable and logs: WORK/first-frame-script154-return.
[Verification receipt](checkpoints/p22-frame-script-door-20261117.json).

The command descriptor calls154/9A DOOR_Lock with frame then integer operands.
Original PE translation byte at0041CE55 is41; jump table0041CBE0 maps that dispatch
to00417992 inside00416E10. Raw instructions confirm pool90 operand0, nonnull frame,
frameEC, actorEC[0], actor type6, THEN integerA0 operand1. Missing frame/data/actor
or wrong type skips mutation and integer access, then guarded-advances and returns
true at0041C8C6 (corrected after the initial checkpoint; see below). Nonzero integer calls005FF650 with this SAME
Door's230 key; zero calls005FF5F0 with its230 key. Both ignore the bool result,
then invoke guarded00416CF0 and returntrue. This is not a script-supplied key or a
one-bit/truncated boolean. Unknown commands still retain their existing required boundaries.

The new base command receiver uses actual DoorSceneActor::key_lock, reconstructed
previously from005FF650/005FF5F0. Thus existing locks, open-Door refusal, actual
linked234 mutation and00601F40 module13/identity/3-bit command packets retain their
original order. There is no direct presentation update or separate packet stream.
The module borrows its constructor's SAME SessionBroadcastPackets before contexts;
the existing containing process outlives module/contexts. A malformed typed actor
or required linked-Door failure throws before PC advance. No fallback success.

48 focused native ASan/UBSan/leak checks instantiate the actual context, Door,
linked Door and broadcast packet owners, with explicit frame/reference leaves.
They cover null/unknown/non-authority paths, wrong actor, lock/unlock/no-op,
open-Door refusal with ignoredfalse and advance, nonzero full-DWORD lock value,
guarded PC, exact packet bits, partial linked failure and frame-reference cleanup.
The real containing application/probe i386 O2/Werror build and stage-only candidate
are recorded in the receipt. Native fixtures are fingerprinted then removed.
The agent does not execute original game/DLLs/Proton/Windows fixtures. OWN source
and prior staged artifact stay unchanged; interactive review stays uncommitted.

Research index was consulted before reading existing typed00416E10 and
human-pack-common-resume-leaves/{005FF650,005FF5F0} exports. No new Ghidra export
was needed. Raw dispatch/mapping and lookup records are retained in
WORK=analysis/work/p22-frame-closure-20261117. Original PE SHA256:
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
00416E10 remains partial; no new inventory count or promotion. Broader commands
still fail explicitly. Next is actual user continuation and its first concrete
failure; no full FRAME/HUM/OWN or game-delivery claim follows compile-only checks.

Correction after d3f1fe4: raw004179A2/B0/BA/C4 targets0041C8C6, proving the
shared advance/true tail for missing/wrong targets. The initial source and its
native fixture expectedfalse on these paths; that expectation was incorrect.
[Fade/shared-tail correction](p22-frame-script-fade-source.md) records revised
source, actual native checks and evidence. Initial receipt hashes describe the
old revision. The actual b4708c75 user run passed reached Door dispatch and now
stops at CSC_FadeIn160; exact logs WORK/first-frame-script160-return.
