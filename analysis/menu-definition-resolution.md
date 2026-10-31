# Menu definition identity and placeholder resolution

Duplicate/placeholder resolution is complete **source-only**. The
[resolution catalog](menu-definition-resolution.json) classifies every declaration,
shared product, repeated root model and campaign table row from the
[owner census](menu-route-census.md) and [control contracts](menu-contracts.md).
[Original byte evidence](menu-definition-resolution-map.json) preserves the
construction and routing distinctions. No original declaration is deleted,
renumbered or redirected; no runtime behavior or function-body credit changes.

## What can be counted

There are **30 numbered declarations: 28 populated owner-scoped roles and two
reserved empty registrations**. The 28 include conditional definitions and
resource-dependent construction. This is a source inventory across owners, not
a claim that 28 screens coexist, have distinct artwork, or work in the host.
The delivered numbered flow remains Main/Exit, two roles.

| Category | Source count | Counting decision |
| --- | ---: | --- |
| Populated numbered roles | 28 | Retain distinct owner/slot identity, even with shared artwork/code |
| Hero reserved registrations | 2 | Preserve slots 2/3; exclude from populated-role count |
| Shared runtime confirmation | 8 owner instances, 1 protocol | Common auxiliary dialog, outside the numbered-role count |
| Description overlays | 3 owner instances | Owned auxiliary products, outside numbered registries |
| In-game information panel | 1 | Engine/model panel, outside numbered registries |
| Embedded kind-five construction | 23 sites, 19 literal frame keys | Pages/templates under existing parents, not numbered screens |
| Dynamic product categories | 13 | Rows, panels, multipage content or auxiliary hook, not registrations |
| Campaign table | 20 rows | Model/data alternatives and sentinels for one numbered role |

Owner plus slot is the identity. Five numeric-ID collision groups are deliberate:
shared ID zero and numbered slots 1–4 recur in different registries. They do not
redirect to each other. Abstract vtables `008157D8/00815850` remain excluded from
the concrete-owner count. Descriptor kind-six subtypes 1/2 select the same team
owner and 3/4 the same items owner; data variants do not introduce registries.
Controller video, briefing and game boundaries also do not create numbered
definitions merely because navigation reaches them.

## Every repeated root/auxiliary model

The catalog derives all six collision groups from the complete inputs. These
are resource/code reuse classifications, not aliases that change event targets.

| Model | Registrations/products | Resolution and original evidence |
| --- | --- | --- |
| `single mission 2` | `main:5`, `main:7` | Different roles despite identical declaration names. `006391E0` builds campaign/single-mission data; `00614140` builds cinematics/video content. Keep both. |
| `options` | `main:6`, `ingame:2` | Same `0061CF60` builder, different owner-family IDs, lifecycle/context and application versus retained-game returns through `0061BEA0`. Reuse construction, preserve both instances. |
| `password` | `main:10`, `main:14`, `connection:2` | Different edits/callbacks/stack entry. `006377C0` adds the ordinary edit prefix and callback `00637970`; `0068AD20` lacks that prefix and uses `0068B9F0`. Add Server `00636FF0` uses two visible address/port edits and `00637440`. |
| `selection screen` | `team:2`, `items:2` | Similar controls, different data guards/bindings. `00691840` installs root `00691AC0` and photo `006919D0` callbacks. `00675F30` filters photos by roster, supplies soldier data via `01000038`, and uses `00677590`. Keep both modal roles. |
| `confirm` | Six numbered declarations and eight shared instances | Named confirmations, connection consoles, reserved registrations and runtime request dialogs have different construction/protocols; detailed below. |
| `description` | Team, items, in-game overlays | All bind null definitions at owner `+FC`. Team uses vtable `00815918`; items/player use `008156B8`. Shared asset/class does not merge the three owned products. |

Ordinary password entry `00637990` pushes modal slot 10; standalone
`0068AED0` timer state 5 selects regular slot 2. Add Server initializes
`binput01/binput02` with `255.255.255.255/11001`, sets edit flags and hides
`binput`. None is an alias of the other password-model roles.

The two connection consoles share kind-18 `text` and three button frames but
bind different timer/root callbacks. Ordinary `00632080` uses `00632490/006321A0`
and button message `0600100A`. Standalone `0068A990` uses `0068AED0/0068AEC0`,
raw `Exit Game` text and `06001004`. Their session and return contracts remain
owner-specific. The resolution catalog includes inspectable control/callback
signatures; matching frame names alone never establish semantic equivalence.

## Hero slots 2/3 are reserved empty definitions

`006971B0` declares hero slot 1 followed by three `CONFIRM/confirm` definitions.
The declarations at slots 2/3 retain IDs `08000000/0C000000`; they are **not
aliases of slot 4**. Their empty construction is established by the full path:

1. Root constructor `00625E80` zeroes root initializer/data vectors and the
   child vector at `+28/+2C/+30`. Each declaration is fresh; no controls or
   initializers are copied from another confirmation.
2. Content guards at `006973D2` and `006973F4` only augment masked IDs
   `04000000` with `00697600` and `10000000` with `00697510`. Slots 2/3 receive
   neither augmentation.
3. Binder `00668470` creates only children present in that vector, then stores
   the model key and ID. Loading confirmation artwork does not turn its geometry
   into interactive controls or supply a default confirmation protocol.
4. Initial selection is hero slot 1. Hero Back `00696DC0` opens modal slot 4;
   the completed route inventory establishes no incoming UI route for 2/3.

Thus both are reserved empty registrations and contribute no populated UI role.
Preserve their original load/bind behavior and IDs if the original object ABI is
implemented later. Removing them would move slot 4 and break original routing.
This does not assert that an arbitrarily injected generic stack message could
never select one of the empty objects.

Populated hero slot 4 has text/Yes/No with original actions `14001100/14001200`
then modal pop. Ordinary slot 4 has an additional hidden `bok` child; its
owner/Exit behavior remains distinct. Shared slot zero builds four controls
through `00668790`, binds callback `006693A0`, and uses request modes/replies/
force-close through the existing runtime-confirmation contract. Neither its
protocol nor its eight owner instances should be counted as named confirmation
registrations.

## Campaign sentinels and variants

The full twenty-row table at `00841668`, stride 24, has these classifications:

| Table rows | Classification | Original outcome |
| --- | --- | --- |
| 0–6, 10–13 | Eleven nonempty model/data variants | Same `campaign:1` ID and `0068C0F0` builder, subject to actual model success |
| 7 | Non-null empty-key sentinel; labels unset | Passes the pointer check and attempts loading an empty model key; no default model is substituted |
| 14–17 | Selectable null sentinels | Getter returns zero; `0068BDE0` returns false before declaring/selecting a screen |
| 8/9/18/19 | Zero-filled padding outside selector range | Never selected by this getter; no model variant or additional screen |

`0068BD10` admits campaign 0/1 and a signed progress result 0–7, choosing
`progress + campaign*10`; other values fall back to record zero. The new
read-only `006B8F90` export confirms that missing/out-of-range campaign-vector
entries return -1. Progress otherwise comes from `006B8660` for session mode 1
or `006B8570`; the guard alone does not prove all sixteen rows can actually be
reached with installed campaign data.

`0068BDD0` returns word zero without conflating empty strings and null pointers.
An empty key follows the original resource loader `0066BFA0 -> 004290A0` and
requires its exact-zero success result. The delegated engine/layered-resource
outcome was not executed or guessed. The sentinel has no populated model key
to count as a new design. The eleven nonempty keys also remain alternatives
of the single registered role, not eleven registrations.

## Embedded reuse and availability

Repeated `scroll13` sites `0066F4BA/0066F502` are alternative child-ID encoding
branches: test `0066F47E`, branch `0066F485`, then ordinal shift 4 or 12 before
the common append at `0066F525`. They are not two pages. Repeated `Panel02` and
`P_visualstore` sites belong to their actual items/player/team parents. Options
groups and binding pages, diary `multi_page`, soldier/medal/store rows and the
ordinary `0067C860` auxiliary hook likewise add no registered screen identities.
All 23 kind-five sites and all thirteen dynamic categories remain cataloged;
classification does not remove source sites or flatten conditional IDs.

Fourteen model keys unlocated in the base archive and 21 loose-text misses retain
the [documented layered-resource dependency](menu-contracts.md). Uninspected
Patch/language members, equal texture hashes or shared captions are not evidence
of duplicate definitions. They also do not justify placeholder success services.

## Verification and next work

The exact client fingerprint is
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
The new byte map checks 44 complete instruction/data ranges and 1,638 direct
calls. Forty relevant functions contain 15,229 verified original instructions,
including new instruction exports for the root constructor, binder and progress
selector. Original assets/binaries remain untouched; Ghidra research was read-only.

The temporary audit independently derives the populated-role set from actual
control placements, verifies all registrations/IDs, collision groups, callback
signatures, shared/embedded exclusions and twenty raw table records, and rejects
damaged catalogs/maps. Existing native checks, all 158 tooling tests, related
evidence maps, frozen ledger and whitespace checks pass. Three strict Windows
targets remain up to date; application and staged hashes are unchanged. No C++
changed, so no new sanitizer fixture was needed. No original game, DLL, Windows
fixture, Proton or desktop test was executed by the agent.

Detailed logs/manifest are ignored under
`analysis/work/menu-definition-resolution-verification-20261031`; temporary audits
are removed. Function ledger remains 369 partial (364 client/five engine), zero
complete, frozen 18,783. Continuation work remains uncommitted at the user's
instruction. The [following planning checkbox](menu-family-delivery.md) is now
complete: concrete family delivery checklists, running-game dependencies and
independent UI validation. Actual flow delivery remains open.
