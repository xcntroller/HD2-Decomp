# Integrated explorer verification — 2026-11-09

Historical checkpoint37dcd58; its ISD1 blocking boundary is superseded by
[the verified raw/LZSS/checksum milestone](ISD1.md). Counts below describe that
prior checkpoint, not the current decoder.

Scope: the new resolver, actor subsets, dependency model and integrated GUI.
[Earlier validation](VALIDATION.md) remains historical evidence for the original
293-script / loose-folder milestone. No original game/DLL/Proton or Ghidra was
run. This work introduces no client/server changes or function-ledger credit.

## Original inputs and actual outcomes

Used the existing local installation/research copies read-only. Registration
metadata matched seven ISD0 indexes (53,612 members), plus all 4,560 LangEnglish
and 1,059 Patch ISD1 names with structural/index-name checks. Fifty-eight mission
names were discovered. ISD1 CRC/payload support was not inferred from this.

The chosen profile was English, flags low byte 0, override 0 (loose first):

| Session | Selected mission files / dependencies | Entities | Graph edges | Accepted script analyses |
| --- | ---: | ---: | ---: | ---: |
| Tutorial | 275 | 2,139 | 5,315 | 129 |
| ALPS1 | 174 | 178 | 1,464 | 130 |

Counts are dataset/profile observations, not universal expected values or proof
of world readiness. Indexed unbound sources, opaque resources and explicitly
blocked payloads remain in the report. Empty-reference classifications and
additional diagnostic codes can change validation-category counts without
changing serialized input coverage.

Tutorial Scene2 selected **Missions.dta** and its 1,890 object boundaries agreed
with the pre-existing `inspect_scene2.py`; there are 80 serialized actor entries
across scene/actor documents. Its two patched instructor scripts and one model
were selected from ISD1 and correctly blocked: their base copies were not used.

ALPS1 Scene2 and actors select **Patch.dta**, with base Missions copies retained
only as shadowed provenance. Ten selected resources are payload-blocked in this
input set. That is not a fully parsed ALPS1 mission. ALPS1's existing original
**base research documents** were separately parsed as explicitly labelled base
bytes: 134 actor entries, with no malformed supported property payloads. This
separate check does not bypass installed precedence or claim patched coverage.

## Actor decoder contract comparison

205 original supported property payloads from Tutorial/ALPS1 base Scene2 and
actors.bin were compared against the actual thirteen recovered C++ reader
families in `recovered/game/actor_properties.h`. Every consumed-byte boundary
agreed. The temporary native comparison passed ASan/UBSan/LSan (outside sandbox
because LeakSanitizer cannot operate under ptrace). This checks serialized reader
boundaries, **not every interpreted value, callback or world side effect**.
Door portal and Switch-state callbacks were fixture no-ops. Constructors/defaults,
normalization and gameplay effects are not claimed to have been reconstructed
by the Python view. Car properties remain unsupported; known type and unknown
property decoder are distinguished. Empty AE24 payloads are labelled as skipped
by dispatch, not successfully reconstructed default state.

## Focused negative and GUI checks

Temporary independent synthetic archives verify loose/Patch/base selection,
archives-first order, winning ISD1 refusal, higher-priority index barriers,
archive mutation checks and path traversal rejection. Actor fixtures cover
supported versions, truncated payloads, unknown chunk retention and malformed
property reporting while keeping other records available. Whole document
truncations publish no partial decoded graph. Session-retention bounds,
streaming deterministic JSON, new-file/outside-root export rules and the old
loose-folder/candidate workflow pass. Engine identifiers are matched verbatim;
they are not subjected to filesystem path syntax rules.

Native GUI checks exercise actual installation discovery, Door actor search and
property detail, graph search/canvas/navigation, validation filtering, archive
source display in the existing inspector, stale-search clearing, installation
export protection and clean close. Tutorial graph and ALPS1 validation windows
were inspected as rendered screenshots. This is agent native desktop testing,
not new user acceptance or original-game compatibility verification. Existing
branch delegation allows verified local commits for review at merge.

One focused real-input audit, retaining both sessions and materializing JSON for
comparison, completed in about 11 seconds and peaked near 200 MiB RSS on this
host. Ordinary exports stream JSON; that audit includes extra comparison strings
and is not a UI performance guarantee. No heavy game build ran. Temporary fixture
sources, proprietary slices, reports and screenshots were confined to /tmp and
removed; no permanent tests or original assets are committed. Final audit hashes
are recorded in the workstream handoff.

Known remaining boundaries: ISD1 CRC/payload decoding; expansion profiles;
engine resource-cache ordering, imported joints and frame-path queries; complete
actor/asset formats; source spans/structured compiler diagnostics; faithful
include resolution; execution/reachability and binary editing. Reports explicitly
set `coverage.complete_format_support` to false.

Temporary audit source identities, removed after verification:

| Audit | SHA256 |
| --- | --- |
| Real installation / explicit base comparison | b610244c4dcdecf906f498df7ebefbd1bbad75679590b25efba8a1a2554ac6a1 |
| Synthetic lookup/bounds/export/identity checks | 3ac9faa7ced4b7024cc1d94e71315cc360420d51249b07bd52d648948cb101b6 |
| Native GUI flows | 2f5507e4bd87a263dd6c81cc3535a6c566310a7f7a69c0b25ba264199f5163fb |
| Property comparison driver | 351ace1a4d6631c2657d16acf670c3307e35b0603e5a3bd734fdf7c13052b49a |
| Temporary C++ sanitizer fixture | b15b1d947c9f158b8ad127ebc8a5ad423a8f5b6def97912685a8ac338b908ca2 |

The real headless entry point additionally exported Tutorial schema2 and checked
selected Missions provenance, 2,139 entity identities, unique validation IDs
and explicit incomplete-format coverage. Language ordering/English fallback,
loose case-collisions and duplicate diagnostic identities also passed synthetic
checks. No new full audit cycle was started after these focused checks passed.
