# H8.4 Photo-place actor source

2026-11-16, `feature/p22-hum-h8-4`, after `a238bc8`.
Actual Scene2 type15 now supplies Human action3's target91 completion receiver.
H8.4/HUM/application/desktop remain open; this is a source checkpoint.

## Original contract and evidence

Registration0060B620 writes descriptor008AC91C into type15's slot; factory0060B640
allocates140 bytes and calls0060B6D0. Constructor sets type15, zero descriptorA0,
flags90/91=false and caption94=-1, and resolves the initial localized caption.
Vtable00813400 pins initialization0060B7D0→00610200, activation0060B820,
deactivation0060BB50, authority/presentation0060B800/0060B810 (genuine RET4),
packet0060BE20/0060BE30 (genuine RET4), material0060BE40 and reset0060BE80.
0060BC80 reads property chunk100's DWORD94, skipping unknown/short chunks.
Snapshot0060BE90/0060BF90 writes/reads chunk3000's raw byte91; short payloads
preserve prior state. No packet state change is fabricated for the empty methods.

0060B820 calls common activation on each pass; only the first clears91 and sets90
BEFORE caption/bounds/collision work. DriverAC produces aggregate bounds/centre/
radius. Native descriptor flags40, layer800000, frame/actor, live world matrix,
owned inverse, maximum-scale radius and transformed centre precede insertion.
Invalid aggregate bounds leave native centre unwritten; the source retains an
explicit guard.0060BB50 clears90, removes collision, then common deactivation.
Source error cleanup also retracts a partially published descriptor before its
owner/frame disappears. Other native object/container/SEH ABI is not reproduced.

00584E30's actual assembly keeps the type15 target inECX, pushes the Human, then
calls0060C130. That11-byte body writes target91=1 and RET4, ignoring the Human
argument. The old default decompiler signature had confused the pushed argument
with the receiver. Production now checks the actual typed PhotoPlace owner and
runs that write before the existing successful action cleanup.0060BE40 tests
picked-frame matid, then the actor frame when different, then fallback11;
00611090 accepts NULL as a miss. Existing actor_material_id supplies text parsing.

WORK `h8-4-type15-*`, `h8-4-photo-boundary-index.txt` and
`h8-4-photo-material-index.txt` locate existing contracts/exports before new
exports. Original ASM pins vtable/constructor/activation/call ABI. Read-only,
noanalysis `ExportPhotoPlaceH8.java` temporarily defines missing leaves and
corrects ABI, including004066C0's RET8; its transaction rolls back. Final typed-v2
exports supersede earlier bad-stack decompilation. No original code executes.

## Composition and verification

`PhotoPlaceSceneActor` binds to the existing Scene2 factory, activation/deactivation
and frame dispatch; packet endpoints consume nothing as native. Human services
bind completion and melee material to this actual owner. Snapshots retain raw
byte semantics. Clone/common full persistence and non7 entry are not implied by
these source helpers; there is no new successful generic actor fallback.

Temporary native ASan/UBSan/leak audit passes3168 checks across32 normal and96
failure lifetimes. It executes actual SceneActorRegistry/ActorBase/storage,
DynamicCollisionBuckets, PhotoPlace lifetime/properties/snapshot/material and
00584E30 action completion. It verifies same identity/frame binding, layer and
transformed descriptor, successful/cancelled completion, effect-before-action
cleanup, completion exception prefixes, both activation passes, re-entry,
unknown/short chunks, raw snapshot byte, material lookup and invalid-bounds/
caption/insert/remove failures with acquired-owner cleanup. Frame/math/bounds/
localization, Human activation fields/action-state view and unselected terminal
services are explicit endpoints; no full Human AI/Player/World execution claim.

Strict actual nested World/Human/PhotoPlace i386 O0 full link instantiates the
factory, both scene passes, real Human completion/material bridge, empty packet
receivers and retirement. Launcher/probe and tracker checks pass. No Windows
artifact or desktop execution. The [receipt](checkpoints/p2.2-human-h8-photo-place-20261116.json)
records fingerprints and audit cleanup. Four existing frozen bodies gain partial
entries:0060B6D0/0060B820/0060BB50/0060C130. Absent leaves, factory allocator and
delegated engine bodies earn no additional credit; no complete promotion.

Continue H8.4's remaining material/Dog/Aircraft/type34 and occupied Human/Player/
mounted/selected entry interfaces. D11 is already resolved under the explicitly
approved [compatibility policy](human-h8-d11-compatibility.md).
