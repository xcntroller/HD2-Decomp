# H8.4 actor material receivers

2026-11-17, `feature/p22-hum-h8-4`, after `85dc76f`.
The remaining HUM melee D8-material guard is replaced with actual typed virtual
receivers on the existing actor owners. H8.4/HUM/application/desktop stay open.

## Original dispatch

00610070 constructs common vtable008137F0. Its D8 slot points to005B88B0,
a genuine constant11/RET4; this is the source base receiver, not a placeholder.
Concrete native vtables select the following overrides:

| Family | Original receiver | Source policy |
| --- | --- | --- |
| Human/Dog |0059C2A0/005D4CB0| Constant66 |
| SupplyBox |005F9850| Constant25 |
| DocumentPlace |0060E550| Constant85 |
| Ladder |005EA560| Actual initialized geometry's material184 |
| Car/Animation/placed runtime35 |004DBEE0/005B5F20/005E2910| Picked frame matid; any negative value becomes11 |
| Door/PhotoPlace/ExplosivePlace |006023B0/0060BE40/0060CEE0| Picked frame, then own model when different, then11; only exactly-1 means missing |
| Runtime Mine |005FEEB0| Existing bound definition material100 |
| Other current native common families |005B88B0| Constant11 |
| Loose runtime11 / detached12 |005B74C0/005FDDC0| Constant11, same source base policy |

00611090 first rejects NULL, then case-sensitive first matid/atoi parsing.
Negative-2 therefore survives the model-fallback family but becomes11 in the
Car/Animation/placed family. The source helpers preserve that distinction, NULL
and same-frame behavior, read order and live metadata. Ladder and Mine retain
required actual geometry/definition producers; no new default fills their state.
HUM's existing kind3/21 melee damage branch still supplies material66 itself;
StaticWeapon's separate native D8 is the common11 receiver.

`SceneActor::material` is a typed virtual source adaptation. Derived classes
supply their recovered native policy; HUM now calls that same owner directly.
No new actor factory, generic damage success or native vtable ABI is inferred.
Dog D4 damage and its activation/control/simulation remain separate requirements.

WORK `h8-4-actor-material-map.json` records constructors and actual D8 targets.
The initial guessed DocumentPlace vtable was discarded:0060DB70 proves008135E0.
Car constructor is004D0B60; common/Zone and runtime11/35 slots were also checked.
Research-index receipts precede selected exports. Read-only/noanalysis
`ExportH8ActorMaterials.java` temporarily defines absent leaves and pins00611090's
RET4 ABI, then rolls its transaction back. Original common vtable and runtime
leaf ASM supplement the C exports. Existing0060E550 export was reused. No
original executable/DLL or Windows fixture is run; project is not saved.

## Verification and accounting

Temporary strict native ASan/UBSan/leak audit passes26976 checks over32 repeated
policy/concrete-family lifetimes.100 picked/model text pairs per family exercise
negative-vs-sentinel comparisons, both signed extremes, NULL, same-frame/no
second read, fallback and metadata-failure prefixes. Actual typed Human, Dog,
Ladder/geometry, PhotoPlace, ExplosivePlace, DocumentPlace, Animation, SupplyBox,
StaticWeapon, Null and placed-item owners exercise virtual dispatch. Actual
00451230 HUM melee source runs against seven concrete material families, proving
query exclusions restore BEFORE actor/pick/material/effect and failed metadata
emits no effect. Human/common storage and navigation owners are actual source;
engine frames/resources/math, item payload/query/lookup and terminal effects are
explicit endpoints. No loaded World, positive Dog damage or engine parity claim.

Strict actual nested-host i386 O0 full link instantiates every currently
supported Scene2/runtime factory and HUM's virtual material bridge. Launcher/
probe and tracker checks pass; Windows artifacts are compile/link only.
[Receipt](checkpoints/p2.2-human-h8-actor-material-20261117.json) records evidence
and removed temporary audits. These D8 leaves are absent from the frozen
inventory: no new function credit, denominator change or complete promotion.
Existing00611090/00451230 partials gain composition evidence only.

Continue H8.4's Dog D4/actual owner, Aircraft/type34, occupied Human/Player/
mounted and selected entry dependencies. [D11](human-h8-d11-compatibility.md)
remains resolved only under the four explicitly authorized compatibility choices.
