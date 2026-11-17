# Actual loading return: partial Human footprint textures — 2026-11-17

Source base9289d4f, with the sound-scene correction pending on FRAME. The user
reported no crash on the revised cb657c0c candidate and requested verification.
The actual logs establish a clean process exit and a HANDLED mission failure:

- Tutorial SessionOwner constructed with four live soldiers and real scene.
- Sound initialization progressed; there is no prior006E954F access violation.
- Human activation threw `Required Human presentation texture failed: d_l.tga`.
- Game entry reported failure, returned to Main, then shut down with result0.

**No loaded mission or gameplay frame is established.** The user's report and
clean teardown confirm the earlier sound crash correction, not mission delivery.
The exact executable and three fresh logs are archived under WORK/loading-return
before any revised staging. [Receipt](checkpoints/p22-frame-loading-footprints-20261117.json).

## Original status return and caller

Research index was consulted before further static research; reused original
0078A4F0 C/JSON/raw assembly and0084F43C texture table in H8 WORK. Both calls
at0078A56A..A57F and0078A5B9..A5CE pass GetIGraph virtual3C(name,NULL,0x402,out),
clear the actual destination before calling, and branch on ANY nonzero status
to the common RET at0078A5E2. They KEEP already acquired texture references,
including a nonnull output supplied with a failed status. They do not load later
sides/records and do not throw or roll back earlier references on this status.

00785BE0 calls0078A4F0 and continues directly into mission/driver settings and
its shared Human reference increment. Later Humans skip this shared initialization
until last-user0078A5F0 releases the retained references. The caller does not
require all26 texture loads to succeed before ordinary Human activation.

A read-only ResourceStore metadata lookup of the actual installation finds
s_/a_/b_/w_ left/right textures, with d_l/d_r missing and no archive-index errors.
This is an explicit static resource profile, not a claim to capture engine lookup
flags. It corroborates the actual engine's d_l load failure. No resource was
renamed, substituted, generated, extracted over the installation, or bypassed.

## Correction and verification

HumanPresentationTextures now distinguishes an attempted initialization from
all-success completion. It stops on the first nonzero status, retains the exact
prefix, leaves later references untouched/null and keeps completion false.
The actual GameActorFrames adapter returns engine status separately from its
ResourceOwner, preserving an actual failed-call output rather than discarding it
through the generic Car texture helper. The native flags and NULL secondary
argument remain exact. Source C++ exceptions from unsupported required services
still propagate and retire their acquisitions. This change follows this exact
native status RET; it is not a general missing-resource success policy.

Native ASan/UBSan/leaks PASS **4,803** checks: all26 failure positions with null
AND nonnull failed outputs, exact original table/name order, no calls after a
failure, ascending last-user release, successful loads/null-success output,
nonpresentation gate, propagated endpoint exceptions and actual shared first/
last Human policies. Real HumanPresentationShared/ProjectionProcess/storage are
used, with explicit engine/mission/driver endpoints. Tests confirm another Human
cannot retry a partial shared acquisition; a new first Human after last-user
cleanup can. Temporary native audit is fingerprinted then removed.

Full actual i386 O2 application/probe build and stage-only evidence are recorded
in the receipt. The isolated review also adds one-time host diagnostics AFTER
successful activation binding and completed input/fixed/update/Scene78 calls.
These report actual returns, never synthesize readiness or establish present.
The production adapter remains uncommitted; OWN source/older artifact remain
preserved. Revised staged candidate still needs an actual user mission test.
No FRAME/HUM/OWN source or desktop promotion, new function count, original ABI/
allocator/SEH equivalence or original/DLL/Proton/Windows execution by the agent.

## Next actual run: SupplyBox literal correction

The user reran the f5cbd39f candidate and again returned to the menu. Fresh logs
show `Human footprint texture loading stopped: name=d_l.tga status=80000001`,
then the next real actor dependency: `Required SupplyBox animation is unavailable`.
This establishes that original nonfatal footprint handling now continues Human
initialization; it does not establish mission activation or a gameplay frame.
The exact new executable/logs are archived under WORK/loading-supply-return.

The actual original005FAC80 references literals at0083E9D4/C8/B8/AC. Their bytes
are **#cratefly, #crateopen, #crateclosed, #cratedrop**. Earlier reconstruction
mistook Ghidra symbol names `s__crate...` for actual leading-underscore strings.
The installed Models archive contains all four `models/#crate*.5ds` payloads.
Original bytes, source table and decoded installed payloads were cross-checked
and fingerprinted after research-index lookup. The actual cache3 loader and
ownership stay intact; the four keys are corrected, and any genuine missing
clip still fails with its exact name included. No placeholder clip, renamed
asset, alias or blanket missing-animation suppression is introduced.

A focused literal/payload check and full strict actual application/probe build
verify this small correction; unchanged ownership/policy sanitizer suites were
not repeated. [SupplyBox correction receipt](checkpoints/p22-frame-loading-supply-20261117.json)
records the new source/build/stage fingerprints. Actual next mission loading
and successful frame execution still need user observation; the interactive
adapter remains uncommitted.
