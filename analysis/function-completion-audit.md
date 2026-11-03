# Partial-function audit and first completion group — 2026-11-03

**Complete, source-only milestone.** All564 existing partial entries were reviewed
as ledger/evidence records, grouped by their127 distinct limitation contracts.
The audit checks frozen membership, implemented-source associations, evidence
links and current delivery boundaries. It is not a claim that all564 original
bodies have undergone new instruction-by-instruction equivalence review.
[Per-entry dispositions and source hashes](function-completion-audit.json)
account for every entry without adding any new function to the ledger.

## Full-ledger findings and closure order

285 records contained stale integration wording: Main/Exit-only scope, disabled
live double-clicks, unfinished native Options, missing slot-zero binding, or
pending Cinematics acceptance, or missing native startup/data integration.
Current source and accepted flow/input/fidelity
contracts contradict those statements. Their limitations and evidence are updated;
original object/container/refcount/SEH differences, guarded malformed input,
unavailable game/network receivers and unimplemented branches remain partial.
Desktop acceptance is evidence of a delivered flow, not whole-body equivalence.
Historical notes retain their original checkpoint claims.

The five button queries below are selected because every dependency is inside
the group, their full frozen bodies are bounded, and they allocate nothing,
own no resources, call no engine service and have no failure/unwind branches.
The other559 entries retain partial status. In particular:

- Screen/widget/event/dialog functions still need original object and ownership
  ABI closure; working native factories or accepted screens do not close them.
- Input producer/installation/controller functions retain native arrays/owners,
  guarded malformed paths and numerical/environment differences. Closing the
  five button queries does not complete their callers or the input singleton.
- Startup/data/profile/archive/cache/media functions retain original ownership,
  SEH/failure, protection, generic-format or specialized-state exclusions.
  Original engine/COM/file implementations remain delegated.
- Game/session/load/save/result dependencies close with the Phase2 receivers;
  ready-session/server-dependent bodies follow Phase3. Loadouts remain completely
  untested until accessible. No original server may be executed.
- All five engine entries remain partial; model/resource/renderer/lifetime and
  format-fallback exclusions cannot be closed by calling the original DLL.

Next closure groups must follow these dependencies and the
[approved completion gates](function-completion-plan.md). The next plan checkbox
is detailed Phase2 dependency research and delivery planning, including such
closure work in each subsystem; Phase2 implementation has not started.

## Completed group: original input button queries

| Original entry | Complete contract | Remaining group dependencies |
| --- | --- | --- |
| `0066A050` | Selector0 means any nonzero state; otherwise any overlapping bit. Unsigned32-bit arguments, stdcall, AL Boolean result, RET8. | None |
| `0066A080` | Read previous state at this+2C, call0066A050 with selector/state, forward AL, thiscall/RET4. | 0066A050 |
| `0066A0A0` | Read current state at this+30 with the same selector/result/ABI. | 0066A050 |
| `0066A0C0` | Query current first; only when selected query previous. True iff current selected and previous unselected. thiscall/RET4. | 0066A0A0,0066A080 |
| `0066A0F0` | Query current first; only when unselected query previous. True iff current unselected and previous selected. thiscall/RET4. | 0066A0A0,0066A080 |

Implementation: [pointer_buttons.h](../recovered/frontend/pointer_buttons.h).
Previously the helper used the native default call ABI and the two fields lived
at offsets0/4. The reconstructed i386 bodies now use stdcall/thiscall explicitly
and place the original DWORD fields at+2C/+30. Standard-layout/size/offset
assertions guard this prefix. The preceding44 bytes are opaque to all five
bodies; no constructor or remaining input-owner reconstruction is credited.
The native PointerUpdate and menu/preview callers use these same implementations.
There is no separate unintegrated matching-only implementation.

Every original branch, read, helper dependency and return is accounted for by
the full145 bytes and six direct calls in the
[fingerprint-checked map](button-function-completion-map.json). The original
functions make no writes, callbacks, allocations, engine calls or exception
setup. C++ callers preserve the original short-circuit query order. The native
update/clear convenience methods are not credited as original functions.

## Selective matching workflow and result

[compare_function_bodies.py](../tools/compare_function_bodies.py) compares full
frozen contiguous original PE32 bodies with named i386 COFF functions. It verifies
the original fingerprint/anchors/calls and census boundaries, then normalizes
only direct-call/tail-jump relocation operands whose target identities are
accounted for. It preserves branch displacements, register choices, stack code
and all other bytes; unknown relocation targets reject the comparison. Reports
separate exact byte equality from same-order helper targets and behavioral proof.
It currently supports this bounded client/contiguous-body/REL32 case; it is not
a general engine, split-function or absolute-relocation matching tool.

Original PE metadata reports linker6.0. That is a compiler-family clue, not proof
of an exact compiler or settings; the original toolchain/options are unknown.
Current compiler is llvm-mingw clang23.1.4 targeting i686-w64-windows-gnu,
C++17, O2 or Os, no LTO. Header attributes retain separate i386 bodies.
**Both configurations produce0 exact and5 different bodies.** All five retain
the same ordered helper targets. Detailed instruction/byte reports are in
`work/function-completion-20261103/matching-O2.json` and `matching-Os.json`.

Reviewed differences are code generation: the selector becomes a branchless
zero-selector mask; field loads are pushed directly; transitions save/reload
the this pointer because ECX is volatile, use a different saved-register set,
invert a canonical Boolean, or tail-call the previous-state query. Original
callers exploit their callees' private ECX preservation; the compiled group
correctly reloads it. AL, stack cleanup, nonvolatile registers, DWORD field reads,
query order and lack of writes remain correct under the declared call ABI.
Instruction equality is not claimed and the existing `complete` definition is
unchanged: verified full-body reconstruction, not original compiler identity.

Reproduce from the repository root with this temporary body-emission fixture:

```sh
cat > /tmp/hd2-button-match.cpp <<'CPP'
#include "frontend/pointer_buttons.h"
extern "C" bool button_select(unsigned selector,unsigned state) {
    return hd2::frontend::pointer_buttons_selected(selector,state);
}
extern "C" unsigned button_queries(const hd2::frontend::PointerButtons* input,unsigned selector) {
    return unsigned(input->previously_selected(selector)) |
        (unsigned(input->currently_selected(selector))<<1) |
        (unsigned(input->pressed(selector))<<2) | (unsigned(input->released(selector))<<3);
}
CPP
.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++ \
  -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror -I recovered \
  -c /tmp/hd2-button-match.cpp -o /tmp/hd2-button-match.obj
python3 -B tools/compare_function_bodies.py \
  --map analysis/button-function-completion-map.json \
  --object /tmp/hd2-button-match.obj \
  --objdump .tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/llvm-objdump \
  --output analysis/work/function-completion-20261103/matching-O2.json
rm /tmp/hd2-button-match.cpp /tmp/hd2-button-match.obj
```

The fixture is temporary; its exact compiler invocation/source digest and
comparison results are recorded with verification. No matching result alone
promotes another entry or validates its callers/resources.

## Verification and accounting

Temporary ASan/UBSan audit tests16,777,216 exhaustive low-byte triples plus252,024
full-width/boundary/single-bit/random cases against an independent bit-scan
reference. Poisoned raw prefixes verify exact+2C/+30 reads and no writes. It also
checks native update/clear and1000 clicks through the actual PointerUpdate/timing
composition. The Windows fixture compiles/links without execution.

The comparison tool recognizes five exact positive controls with relocated
helper calls and rejects eight negative controls: bad original bytes, missing
or duplicate symbol, wrong fingerprint, non-entry body, malformed object and
unknown helper relocation, and a truncated inventory body. Positive controls are static fixtures, not additional
matching or reconstruction credit. Existing native regressions,158 Python tests,
strict i386 application/preview/startup builds, original map and frozen ledger
checks pass. Temporary fixtures and binaries are removed after recording results.

Manual whole-body review promotes exactly five existing client entries; no new
entry/engine/delegate credit. Totals: **5 complete,559 partial** (554 client/five
engine), denominator unchanged at18,783. Recovery is source-only; the prior
selection fixes are separately user desktop-accepted. Original files, staged
profiles/config and accepted staged client are preserved. No packaging, original
game/DLL/Proton/Windows-fixture execution, commit or push.
