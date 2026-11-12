# PACK command12 HeavyFire support — source checkpoint 2026-11-13

Command12's valid source effect boundary now uses actual Human/Items, Players,
speech and **HeavyFire module16**, rather than Mine12 or a successful stub.
[Receipt](checkpoints/p2.2-human-pack-heavy-support-20261113.json) records source,
audit and build fingerprints. Common3, consolidated PACK integration and full
H8/application/desktop acceptance remain open. This is not full-body ABI credit.

## Original evidence and source owners

Original client SHA256 remains
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Research index preceded lookups/exports. WORK is
`analysis/work/mission-world-20261103`; the earlier H8 action/HeavyFire C exports
and retained native assembly were reused. New read-only/noanalysis exports in
`human-pack-heavy-receiver-resume` cover00472590/00471750. Native instructions
are retained in `human-pack-heavy-air-core.asm`,
`human-pack-heavy-resume-constructor.asm`, `human-pack-support-action.asm` and
`human-pack-heavy-resume-air-writer.asm`. Successful Ghidra runs report no ERROR;
no original binary execution or saved project mutation occurred.

`HeavyFireDefinitions` recovers00472210 defaults and004732B0 loading from the
actual files resolver: sixteen model/speed/effect/count/power/radius records.
Missing/empty file preserves the existing config; the first root resets defaults.
Name fields advance the record cursor, subsequent scalar fields update that
record, and unknown chunks skip. Source bounds reject native malformed reads.
The actual host module constructor loads this process config once.

`heavy_fire_player_support.h` recovers005467C0. Actual inventory definition243
or244 is required; a missing item or genuine ray miss returns false. Direction
XYZ scales by500 with FLOAT stores, then SAME loaded World ray uses masks1/0 and
no normal output. Side005A8820 is read after that query. Air type uses original
RandFloat*2+1 truncation, direction negates X/Z with Y0, height is hitY+60, and
ordinary offline count is bool(side). Extended mission50/54 supplies the other
count reads. Native caller returns true after module admission even when its
busy/result code rejects creation; the source preserves that distinction.

`HeavyFireModuleLoading` owns actual190/1A0 support lists with type-erased
**real** aircraft/mortar objects, not empty effect callbacks. Request gates,
construction-before-insertion, support flags, module16 publication and inverse
retirement follow00472E50/00472CE0/00472350/004723F0. Update traverses air then
mortar in reverse and deletes false-returning entries by swap-last. Finish
reverse-deletes lists without clearing support flags; activate clears flags.
The module registry/World/process are the same existing owners.

**ABI correction:** module16 lives at primary object+4. Thus activation's
module-relative1AC..1AF are the SAME bytes as primary request1B0..1B3, not a
second flag array. The primary constructor's1AC clear initializes list capacity.
Receiver configuration+0C similarly means primary+10. Native relative offsets
must not be mixed when integrating these bodies.

## Positive support objects and concrete endpoints

`HeavyFireAircraft` supplies00470530/00471170/00471090: native direction
normalization and15000ms signed64 flight, actual model-cache creation/hierarchy,
local offsets, attached group8 SOUND voices and independent support speech,
first-model motion, native interior-only oscillation (last member is skipped),
voice transition, crossing blast and scattered particles/group6 sound, expiry,
and reverse model/voice release before independent speech release. Important
x87 intermediates/FLOAT spills were checked against raw instructions; source
zero W lanes and finite guards are explicit portability adaptations.

`HeavyFireMortar` supplies004719F0/00471B50: actual Y300 input,7000ms signed64
timer, source voice/delay, ground-ray crossing blast with radius20/power1000,
20 scattered effects and expiry at-7000. Aircraft expires at-15000. Combined
mode authority wrappers genuinely return true without advancing: presentation
owns advancement. Presentation/noncombined authority execute the real body.
Timers retain native signed32 elapsed to signed64 subtraction/wrap semantics.

Actual host services bind model/cache/scene/frame and sound-frame methods, SAME
SOUND catalogue/voices, module17 particles and loaded static ground ray. A checked
extension of the **existing** explosion contact link reaches SAME
`GameBlastWaves::create`/Startup blast storage, preserving the initial advance;
no duplicate blast owner or fabricated activation exists. Human loans carry a
captured identity and validate the physical slot before later dereference.
Resource and callback failures retain applied prefixes and unwind owned resources.

00472590 now decodes both actual module16 presentation layouts and consumes
without construction in authority mode. Non-authority presentation constructs
real support objects with native side2 and actual physical Human lookup. The
real module dispatcher binds it. Authority module16 retains native defaultfalse.

## Native unsafe paths and exact source guards

These are native invalid-memory adaptations, not successful missing services:

- 00473018..29 (air) and the matching mortar writer have an inverted source test:
  **nonnull Human writesFFFFFFFF**, while NULL loads absolute address10. Source
  preserves the nonnull bits and throws on the invalid null read. Do not silently
  change this into a symmetric Human identity protocol. Append cursor/metadata
  rolls back on source exceptions; earlier object/flag effects remain applied.
- 006BC5A0 allocates an ordinary kind3 mission with **50 hexadecimal bytes**;
  006BD990's37 payload writes40..4C. Reading descriptor50/54 for action34 there
  is outside that object. Source requires the actual extended kind13 mission
  fields, rather than inventing ordinary offline configuration words. No claim
  of gameplay unreachability is made. Missing actual mission also rejects.
- A zero-plane aircraft frame unconditionally dereferences its first plane in
  retail; source reports that invalid access. It does not fabricate a plane.
- Received mortar uses native side2; its deletion computes flag index4, beyond
  the four support bytes. Source rejects that native overflow before mutation;
  finish can still retire the object. Received air side2 clears index2 as native.
  This remains a declared unsafe non7 retirement limit, not an idle success.
- Unproduced type/action bounds, absent received Human and nonfinite direction
  reject instead of reproducing native out-of-range/null/undefined memory reads.

The valid positive object/effect paths are supplied; these guards do not imply
native malformed-data equivalence or full H8/Phase3 delivery. Do not remove them
to make a fixture appear successful. Application loading stays disabled.

## Verification and next work

Five temporary native ASan/UBSan/leak suites pass100 lifetimes each: definitions,
request/list/wire policy, mortar body, aircraft body and combined command12
codec→support admission→real module16 request/receiver/support ownership chain.
They cover real production policies and models/timers/streams, with explicit
actor/mission/ray/model/SOUND/blast/particle endpoint doubles; no engine result is
inferred. Busy/missing item/ray miss, throw prefixes, reverse destruction,
mode wrappers, received construction, signed timers and queue rollback are checked.

Strict concrete i386 FULL LINK instantiates actual Human packet dispatch,
command12 support, module16 packet routing, both support objects and their
World/ray/engine/SOUND/blast consumers. Latest launcher/probe rebuild passes.
Executables were not run. Temporary audit sources/binaries are fingerprinted
and removed; logs remain ignored in WORK. Frozen-inventory checks and existing
tracker tests apply; only implemented frozen bodies gain partial claims.

Next: finish common3's remaining selected subtype bodies on the same actual
owners, including target/timer/interaction/default policies and required
producer/retirement contracts. Then run consolidated affected packet verification
and integrate committed HUM dependencies back into PACK. Ordinary H8 frame work,
Players delayed command execution, wider unsupported actors, application loading
and actual desktop acceptance remain separately owned and open.
