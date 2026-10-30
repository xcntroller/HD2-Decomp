# Shared control state foundation

2026-10-30: the plan entry **Reconstruct shared control state,
initialization/event messages and eligibility** is complete for the portable
native menu implementation. This closes the common contract used by text,
buttons, containers and the recovered options-widget runtimes. Widget visual
construction, real preference services and general screen factories remain
separate open entries in [the plan](../plan.md).

## Recovered contract

The fingerprinted [instruction map](menu-foundation-state-map.json) supplements
the earlier [UI map](menu-ui-map.json) and
[factory map](menu-control-factory-map.json). Addresses refer to the unpacked
base client with SHA-256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.

| Original code | Native contract |
| --- | --- |
| 0064DCC0 | Disabled/visible/focusable defaults 0/1/1; words +58/+5C default 0/FFFFFFFF |
| 0064D9F0 | Eligibility is `focusable & visible & (disabled == 0 ? 1 : 0)`, preserving byte semantics |
| 0065CD40 | Tagged registration precedes masked ordinary dispatch; all ordinary messages, including unknown codes, return true |
| 01000001..01000006 | Guarded derived disable/enable; unconditional virtual show/hide; direct focusability stores |
| 01000007..0100000C | Typed observer registration/removal and mutable +5C/+58 property setters/queries |
| 010000A1 | Emit the argument as a tag only when bit 04000000 is set, passing notification argument zero |
| 0200002C | Fresh layout event per child; reread child storage/count after callbacks; ignore child result |
| 0065C760 / 0065C790 | Store visibility, change frame visibility, update frame, then notify; repeated calls repeat effects |
| 00661CC0 | Store disabled byte before resolving/mirroring optional attached text |
| 0065CF00 | Dispatch the actual mutable initialization record through +6C; reread storage/count after callbacks and ignore handler result |
| 0065D150 tail | Visual gate, data initialization, event initialization, insertion; failed gate destroys/skips but returns handled |
| 0065CAC0 / 0065C9A0 | First matching ordinary observer; special override; reverse removal with context taking precedence over tag, then callback |
| 0065CF50 | Notify before tagged delivery; retain payloads; clear tag bits; local target FFFFFFFF uses the emitting derived control; other targets enqueue |

The common runtime composes these operations rather than introducing separate
widget copies. Derived setters remain authoritative: for example, a slider can
reject an enable request because of its range or disabled parent. Native text
and button integration already has desktop confirmation; options-widget state
composition has native validation with supplied backends.

For layout propagation, the original initializes target, code, argument and
payload. Its two unused event words are unspecified stack contents. Native
records zero those spare words; this does not claim to reproduce uninitialized
original storage. Direct queries mutate the caller's record, while copied +74
delivery retains query changes only in its copy.

## Closure audit

A temporary C++ audit used the actual state/runtime, initialization, event and
observer implementation, plus actual selector/toggle/slider and container
handlers with fake visual/input backends. It passed strict native compilation
with AddressSanitizer and UndefinedBehaviorSanitizer, and strict Windows i386
compilation. No permanent unit tests were added.

Coverage included every base selector and unknown/masked/tagged cases,
noncanonical disabled bytes, repeated transition guards, frame/text/observer
ordering, sender identity, override/removal precedence, false observer results,
local versus queued routing and payload release after all owners disappear.
Initialization callbacks appended records and forced vector reallocation;
layout callbacks grew the child list. Both traversals visited the new entries
without forwarding prior mutations. Factory success/failure ordering and
derived widget enable/disable policies also passed.

For an independent eligibility comparison, the audit extracted the exact
23-byte predicate at 0064D9F0 from the fingerprint-checked, section-mapped PE.
It executed only these self-contained instructions against an owned 0x68-byte
buffer on native Linux x86-64, using RCX for the object pointer and the low-byte
return. These instructions have identical relevant semantics in this mode and
contain no calls or external addresses. All **16,777,216** combinations of the
three input bytes matched both native eligibility helpers. No game, engine or
Proton process was launched.

This audit establishes the shared native contract. It adds no visible menu
action, implements no widget visuals or settings services, and does not verify
the unfinished widgets against a desktop engine run. Original class layout,
intrusive ownership and SEH remain portable replacements; original-function
census entries therefore remain partial. Closing this delivery checklist entry
adds no function credit: **96 partial, zero complete, frozen total 18,783**.

Persistent evidence check:

```sh
python3 -B tools/validate_frontend.py --map analysis/menu-foundation-state-map.json
python3 -B tools/function_progress.py --check
```
