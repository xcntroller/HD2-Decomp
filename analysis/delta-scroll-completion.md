# Cursor delta queries and Credits scroll store — 2026-11-03

**Verified source-only milestone in the expanded completion pass.** Four existing
partial entries are complete; the broader [queue](function-completion-queue.md)
remains open. No new original entry or delegated engine body receives credit.

## Complete boundaries and original evidence

| Entry | Original complete behavior | Integrated implementation |
| --- | --- | --- |
| 00669FF0 | Read floats+14/+18; ordered x87 compare with0, X first and Y only if X compares equal; AL true for nonzero or unordered. No arguments/RET0. | PointerCursor::moved |
| 0066A020 | FLD float at input+1C, return in ST0, RET0. | PointerCursor::wheel_delta |
| 0066A030 | Ordered x87 compare of float+1C with0; AL nonzero/unordered, RET0. | PointerCursor::wheel_moved |
| 0064B940 | Store the full input DWORD at text+78; AL=true, thiscall/RET4. | CreditsScrollState::store |

[Original map](delta-scroll-completion-map.json) checks every byte of all four
frozen bodies, the actual zero constant0080F51C, and renderer instruction
0064B9B3 (`SUB AX,[ESI+78]`). The renderer consumes a WORD even though the setter
stores a DWORD. Earlier coverage incorrectly described the setter as low-word
conversion; that fragment actually belongs to caller/render adaptation.

The delta prefix has asserted float offsets+14/+18/+1C and no virtual/native
pointer fields. i386 query methods explicitly use thiscall. The normal menu and
pointer-dispatch diagnostic now call those methods directly. Existing free
helpers use the same x87 comparison primitive. Input singleton construction,
global cursor coordinates, clipping/accumulation and full0066A220 remain partial.

The comparison primitive retains `FLD; FCOMP; FNSTSW; TEST AH,44` and parity
interpretation. Ordinary C++ comparisons could change ordered-NaN exception flags
or use SSE. The x86 implementation uses the caller's x87 control word, balances
the x87 stack and preserves sticky flags; it does not mask or clear exceptions.
The configured target is i386 llvm-mingw. Other-architecture convenience fallback
is not a claim of original x87 ABI/environment equivalence.

The Credits setter owns only the DWORD write/true result, not surrounding native
text storage. Its prefix asserts offset78 and size7C. Credits now passes the full
checked integer offset into the stored text state, then explicitly reads its low
word for layout. Rate, initial offset, row arithmetic, completion and the prior
stored-float precision limitation are unchanged. The complete original caller,
text constructor, renderer and generic text ownership remain partial.

## Verification and matching

Temporary native ASan/UBSan fixture passes132,928 delta triples, including both
zeros, signs, normals, subnormals, infinities and quiet/signaling NaNs. It tests
all12 valid x87 precision/rounding combinations, ordered-invalid/denormal flags,
sticky flags, short-circuit suppression of the Y comparison, balanced stack and
poisoned original-layout read-only storage. Another262,144 cases verify every
Credits low word across four high-word ranges, true returns and exact write
boundaries. Full-value/low-word conversion boundaries and1000 composed clicks pass.

The same fixture compiles/links for Windows i386 without execution. Current strict
application/preview/startup builds, existing native regressions,158 Python tests,
original evidence and ledger checks pass. No new desktop test is claimed.

`compare_function_bodies.py` reports **two exact matches** for the wheel getter
and Credits setter; these bodies contain no relocations. Original sizes are4
and12 bytes respectively. The motion/wheel predicates retain the ordered x87
sequence and field accesses but differ in integer Boolean/branch code. Their
absolute FCOMP data relocations are manually reviewed against the emitted zero
constant; the current generic comparison tool does not normalize those operands.
Do not report them as exact matches or silently mask their data references.
Compiled instructions and the two-function report are retained in
`work/function-completion-deltas-20261103/` with source/artifact/audit hashes.

Totals after this milestone: **9 complete client entries,555 partial (550 client/
five engine)**, frozen18,783 unchanged. Temporary fixtures are removed. Installed
assets, configs/profiles and existing packages are preserved; the agent does not
restage the client. The staged client independently advanced to the prior
source-only build3c2e76f6…40bee, whose log ends result0; no new desktop acceptance
is inferred.
No staging, packaging, original game/DLL/Proton/Windows-fixture execution, commit
or push. The next milestone combines scalar owner fields and input-foundation
helpers with one consolidated verification sweep, as the user requests. The expanded
pass cannot close while the remaining555 rows still require body/dependency review.
