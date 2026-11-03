# Scalar owners and input foundation — 2026-11-03

**One completed source-only batch:** all41 candidates reviewed,11 complete bodies,
30 with specific remaining recovery work. The expanded completion pass remains
open; these30 are not declared externally blocked or deferred merely because of
an ABI difference. [Per-entry boundary/dependency review](scalar-input-batch.json).
No new function entries or engine-body credit.

## Closed bodies and integrated behavior

| Entry | Complete original contract |
| --- | --- |
| 0067EA10 | Store DWORD status at this+04, thiscall/RET4. Actual ApplicationState now orders status/pending before its native type metadata. |
| 0066A860 | Store the raw byte at input+48, including noncanonical true bytes, thiscall/RET4. Production session uses PointerInhibition. |
| 006691A0 | Clear only reply DWORDs+88/+8C in order, thiscall/RET0; preserve mode, stack and queue. Actual RuntimeConfirmation places its prefix accordingly. |
| 006E5300 | Fastcall ECX/EDX begin/end scanner; stop before CR/LF/end, count NUL as data, return DWORD length. Localized-text parsing calls this body. |
| 006C6C90 | Signed32 → float stores at+A8/+AC/+B0 in order using FILD/FSTP and caller x87 environment; AL=true/RET12. Actual manager limit service calls this body. |
| 006A8230 | Stdcall/RET4 reserved-scan test on the argument's low byte; exact14-byte list at008424F0, AL result. |
| 006A8290 | Write output−1 first; reject negative/out-of-range/null-vector indices, otherwise return one-based contiguous group ordinal through the output pointer; thiscall/RET8. |
| 006A83C0 | Validate index and full DWORD kind/code arguments, then truncate stored WORD fields, set direction/dirty, call conflict clearing; AL Boolean/RET16. |
| 006A8480 | Clear conflicting bindings only for overlapping modes, preserving hidden commands and the2848/2856 caption exception; preserve direction/padding; AL=0/RET4. |
| 006A85B0 | First record with both kind/code zero, including hidden entries, or FFFFFFFF; ECX receiver/RET0. |
| 006A8740 | Reset vector end to begin, preserving header, begin/capacity, mode and installed manager state; ECX receiver/RET0. |

All selected bodies now have explicit i386 call ABI and directly usable original
field/record layouts. Native convenience wrappers adapt optional/size_t/bool UI
interfaces but do not earn separate original-function credit. The configured
Win32 vector is asserted to be a twelve-byte pointer triple at configuration+04,
with mode+10 and record size1C. These bodies do not allocate/free vectors or own
their construction, copying or teardown. That distinction permits closing them
without claiming the original allocator/SEH/manager bodies are complete.

The binding edit corrects a real domain mismatch: kind10001 with code3B must pass
the original full-width kind check, then store kind1/code3B. The old uint16_t
parameters truncated kind before checking and rejected it. Mouse code10100 likewise
passes the original full-width axis gate before storing0100. Ordinary UI values
remain covered by the same reserved-key/conflict rules.

The ordinal method restores the original invalid-index/output contract and
write-before-read alias behavior. Native wrappers keep their separate exception
policy for invalid size_t requests. Conflict clearing avoids an eager selected
record access when the original loop would never access it, including empty lists.

Original006A8740 contains a statically unreachable record-copy loop:006A874A
copies EBX to EAX,006A874C compares EAX with EBX,006A874E always jumps to006A8769.
The end=begin implementation accounts for the full frozen47-byte body without
recreating unreachable work or changing the inventory boundary.

## Evidence and comparison

[Fingerprint-checked map](scalar-input-batch-map.json) preserves all bytes of the
11 closed bodies, two original edit-helper calls, the reserved-scan literal and
the constructor finding below. The census, ABI/field/return contracts and all
reachable branches were reviewed individually before promotion. Full-body review
is distinct from instruction equality.

The O2 i386 comparison reports **one exact match**,0067EA10. The other ten differ
in register allocation, Boolean code, scan loops, equivalent field/store code,
division by record stride, or deletion of the proven unreachable clear branch.
The axis setter uses the same ordered x87 operations and stores but places its
AL=true write later. No exact instruction match is claimed for those ten.

The static comparison parser had a concrete defect: an unpadded final byte before
a tab on a long instruction was parsed as mnemonic text. Its fix is verified
against independent COFF section bytes for every emitted candidate, including
the21-byte reply-clear body. This avoids losing bytes in comparison; it does not
weaken the exact-match rule or normalize additional differences.

## Remaining candidate boundary

The30 open entries are individually indexed with original body hashes, existing
exports where available, source links and remaining work. Main dependencies:

- Six screen callbacks require the original focus+6C, child-pointer array+08,
  valid-index virtual+04 and child callback vtables+44/+48/+50/+54/+58/+60, rather
  than native tree/callback storage. Closing them requires a real screen/control
  ABI bridge that preserves fresh lookup and callback reentrancy.
- Coordinate/input/pick bodies0066A120/0066A220/0066A7C0 and dispatcher0066B990
  require the shared raw input owner/global cursor and frame/capture ABI. The
  current ray additionally folds in caller Z+.1 and guards invalid dimensions;
  that is not the complete original coordinate body.
- Queue0066C770/common0066C950 retain the native deque/shared payload/registry
  replacement. Original segmented queue globals, intrusive payload release,
  screen/capture/resource ownership and failure behavior remain to reconstruct.
- Character FIFO0066B380 retains a deliberate unterminated1023-byte guard where
  original MultiByteToWideChar could overread its stack buffer. Actual graphics
  +A8/raw call ABI and a reviewed policy for that path remain explicit work.
- Manager lifetime, binding installation/production, configuration application,
  defaults and startup must close their original vector growth/allocator/SEH,
  device ownership/global008AEA48 and callback contracts. The native producer's
  POV guards and incomplete x87 environment also remain explicit differences.

**Constructor correction:**006A81C0 first PUSHes ECX, then reads [ESP+3] at
006A81C3. That is the high byte of the saved this pointer;006A81C7 writes it to
the header. It is not an indeterminate stack byte. The original also returns
EAX=this and preserves the untouched header padding while zeroing its pointer
triple/mode setup. Native header0 initialization and GNU constructor return ABI
do not yet reproduce that complete contract. The ledger now records this concrete
remaining work; a compiler probe confirms the ABI distinction. No constructor
completion is claimed.

These dependencies define further batches, not a reason to abandon source
recovery or move to Phase2. In particular, actual server/UI availability is not
used to defer independently reconstructable helpers.

## One consolidated verification sweep

Temporary ASan/UBSan fixture exercises the actual11 bodies together:
40,000 randomized editor/config transactions with independent conflict-policy
reference;131,072 scan aliases;20,000 bounded binary line scans and parser use;
144,000 axis triples across all12 valid x87 rounding/precision modes, with
independent integer-to-float bit rounding, precision flags and stack/control-word
checks;10,000 status writes; all256 inhibition bytes; reply-write boundaries,
queue/mode preservation, ordinal aliasing and retained vector allocation.

The before/after native fixture reproduces and fixes full-width kind truncation.
Strict application/preview/startup i386 builds, Windows fixture compile/link,
existing native regressions,158 Python tests, original maps and ledger checks
pass. Only a fixture's explicit byte-inspection cast needed adjustment for
Clang's dynamic-class warning; no production fix required another full sweep.
Windows fixtures and original game/DLL/Proton code are never executed.

Logs/source/artifact hashes: `work/scalar-input-batch-20261103/`. Temporary
fixtures are removed; no permanent tests. Source-only totals: **20 complete
client,544 partial (539 client/five engine)**; denominator18,783 unchanged.
The current staged client/config/profiles are preserved, with no restaging,
packaging, commit, push or new desktop claim. The overall pass remains open.
