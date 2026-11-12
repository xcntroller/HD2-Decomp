# Integrity and rendering after overnight recovery

Source review `799a79305c70bc8de1860ac7e438bba25a71a8ac`. Original client SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`. This reassessment reused prior exports and ran two sequential read-only/noanalysis Ghidra queries for callsites, short instruction windows and known static control flow. Raw material remains ignored. [Evidence manifest](evidence/manifest.json)

## Targeted Steam GUARD closure at122ef38

The supplied Steam image contains an unconditional00997602→00997658 jump over00994DF0. Independent byte checks confirm that jump, all32 descriptors, initial blocked-state storage, the GetTickCount seed resolver and original direct observation stores83/60/100. Current source preserves descriptor matching, blocked checks, pre-API action62, per-thread action02 return restoration and actual API execution. Startup invokes the896-draw initializer before graphics and the later time(NULL) CRT reseed at the original device-tail boundary.

The earlier conclusion that private environment producers must run before every observation is superseded for this Steam target. Their original writer and CFG evidence below remains valid historical research. Dormant retail producer equivalence is outside the supported Steam closure. GUARD is resolved_source for the reconstructed offline graph. Mission execution and desktop outcomes remain under OWN/VIS/REPEAT.

Six of seven final receipt source hashes match. The committed startup_session.h digest differs from its receipt entry. Its actual code and call order were inspected directly. Upstream native/i386 results are reported with that exact-file attribution limit. [Targeted verification](evidence/guard-verification.json)

## Historical integrity investigation at799a793

The direct call at `006DB104` invokes `006E9EA0` near the beginning of `006DB0F0`, before the first graphics initialization operations. Its retained export shows no mission or offline-mode gate at that call. This establishes a device-startup invocation boundary, with process-owned storage. A device initialization retry can invoke it again according to that original caller. Preserve its position relative to later shared randomness. Per-mission reseeding would change the original sequence.

`006E9EA0` initializes both 128-entry WORD permutations, calls through `00A59000` at `006E9EBC` for the seed, seeds the shared CRT, and consumes exactly 896 draws. The original private wrapper's actual seed API and validation relation remain unresolved in this bounded investigation. At799a793 StartupSession exposed MissionIntegrityTables without initialization or observation publication. This is superseded by the Steam closure above. Existing expected, observed and API-payload storage remain distinct. [Original findings R07](evidence/original-findings.json)

## Private producer chain

| Original owner or field | Newly verified evidence | Remaining contract |
| --- | --- | --- |
| `0099AEB0` | Callsite009C5335 invokes009F0A60,009CAD19 invokes009F02A0,009CAE28 invokes009905B0. The earlier call block dominates the later call block in the known CFG. | Actual successful environment branch and global startup entry relation |
| `009905B0` | Push40hex and allocation at00990934, publication to00B3BD8C at0099093E | Allocation is16 DWORDs. Payload initialization/population cannot be inferred from allocation |
| `009995F0` | Conditionally called at00992A9A inside009905B0.00999952 selects indices for00B4EE80 according to table/status gates | Four indices later used by validation11 require genuine population, not arbitrary zero records |
| `009F02A0` | Prior export allocates00B52F6C, obtains private payloads, writes00B4C2CC from00B54BF8 | Original external-function semantics and upstream00B54BF8 still unresolved |
| `009F0A60` | Prior export allocates00B555B0 from a loaded84-byte block/WORD00B4FFEC or an external result | Configuration/environment source and correct branch remain unresolved |

The targeted CFG contains495 known blocks and one outgoing edge outside the owner. Entry reaches all three call blocks.009F0A60 precedes009F02A0 and009905B0 on the known paths that reach those later calls.009F02A0 precedes009905B0 similarly. Reverse reachability between these call blocks is absent. This is evidence-backed static ordering with a stated CFG limit. It does not establish successful dynamic startup or indirect target behavior. [Findings R06](evidence/original-findings.json)

Private entry0099AEB0 has a known caller at00A4D84B in00A4D782. Relating that protected entry to the normal application startup and its environment setup remains part of GUARD. This earlier investigation did not establish skip reachability. The later Steam-specific jump evidence above resolves that question for the selected distribution.

Validation11 at00994DF0 alternates indexed00B3BD8C checks against the00B52F6C/00B4C2CC relation. Validation21 has environment-sensitive branches including009F8C60/009F8D40. Their root80000002 and four-byte query ABI suggests registry-style operations, but exact imports/keys remain unconfirmed.00996330 validates before action62 publishes the matched expected word to the matched destination. Action02 uses per-thread return override. This producer/validation/action requirement describes the dormant retail route. The selected Steam path follows the independently verified distribution-specific jump.

Offline7 does reach meaningful guard consumers. The loader/activation uses the original wrapped APIs. The retained0069BA60 objective path compares stage13 and draws only on mismatch, conditional on a real objective call. The initializer itself precedes mission selection. The exact subset of descriptor validations reached on the selected first-frame path remains unresolved. Preserve real validation and shared CRT ordering. Do not synthesize healthy observations or convert raw API returns into observed values.

## Targeted RENDER closure at345a390

The historical missing containing-render/history observations below are superseded
for RENDER-owned source by feature/p22-render `345a390`, not merged into reviewed
master `2c96a7d`. Original allocation48/constructor00481320, loader00481810, refresh
00480FA0 and allocator proof leave3C/40 unwritten.00482843/4E consumes their old
bytes;00482862/64 stores current direction afterward. We independently compared
13 retained static ranges (9,413 instruction bytes) with the fingerprinted client.

Current Volume owns48 raw bytes allocated without value initialization before
metadata/random work. At the original visible/in-range gate, the actual platform
endpoint copies live3C/40 through ReadProcessMemory, then existing angular44 and
current-direction stores execute. There is no initialized first angle, zero delta,
extra draw or historical-heap producer requirement. OS-copy failure remains an
explicit source safety failure. Different CRT/metadata allocations and raw-sidecar
ABI do not reproduce a historical heap trajectory or guarantee pixel equivalence.

Positive00482260 traversal, procedural00482E50/resources00483E50/00483DE0, actual
Scene15C and graphics calls, debug0079B760/0079AC40 and GEffects special-light
004870A0/004871A0/update/restore source are present. Actual compiler/context
opcodeAD CSC_RunCLouder binds the SAME process configuration; opcodeBB binds the
special light. Procedural rendering precedes visibility traversal; driver3 and
graphics13/14 restore through primary-error-preserving cleanup. Special flicker
uses the SAME engine random before time224 update, with deferred expiry ordering.

`GameRenderStages` binds the existing World. The lifecycle owns stages and Scene
callbacks, exposes original render policy→actual Scene78, and retires callbacks
before stages before World. Its native Scene bridge handles failed registration,
borrower renewal and primary callback errors. Current master already has full UI
7a3f7b7; that source and FRAME must be composed with these lifecycle changes.
Actual GameApplicationState still declines loading. Conditional water/weather/
game64/colour updates retain required guards in their owning frame/update path.

All36 receipt source/document observations and70 retained evidence entries match.
Upstream native100-family sanitizer/full World-lifecycle i386/application-link
results were reviewed, not rerun; engine/UI endpoints are doubles, and no Windows
or original output ran. Full tracker reports legacy0062D880 missing ignored WORK
evidence, with focused ledger checks passing. Actual selected populations,
Scene15C state, Scene completion, present/fade, repeat and second mission remain
runtime/user acceptance requirements. [RENDER verification](evidence/render-verification.json)

## Historical render observations at799a793

## Corrected render owner and clock

Original instructions00486BF0..00486C02 read GEffects+224 into EAX, addC4 to ECX, push EAX, then call00482260. **The volume owner is GEffects+C4 and the argument is clock224.** Our previous volume224 wording incorrectly described the receiver. Current EffectsModuleLoading already owns volumes and EffectsLoadingProcess.field224 separately, and the latter advances with loading updates. No current source mismatch is asserted for those owners.

At004822C1 the caller sets ECX=0089B63C and pushes the time argument before00482E50. This process render call occurs before per-light visibility traversal.00482E50 checks its first texture pointer and can genuinely return when absent. A populated texture requires its actual procedural geometry/draw operations. Empty light vectors alone cannot determine this branch. Preserve the existing clouder/clouder2 textures, flags502 and two3CC00 buffers. [Original findings R08](evidence/original-findings.json)

00482260 then reads the real scene+15C camera-related frame, updates dirty transforms, computes camera angular change, checks each light's scene visibility relation, hemisphere and distance, and swaps visible records far-to-near before the recovered draw leaves. Sight uses scene+128. The relation between those native camera roles and first-frame camera setup still needs original evidence. Original initial per-volume3C/40 history remains unknown. Baseline Tutorial field2C zero does not determine active Patch or process texture state.

Driver3 and graphics13/14 restoration must follow the native save/restore semantics. Stack aliasing in the decompiler makes typed scalar fixture arguments insufficient evidence for the containing visibility renderer. Existing leaf tests validate their own numerical/state boundaries only. The application still lacks real pre/post scene render consumers.

## First presentation boundary

Original0069B870 calls actual Scene+78(scene,0) unconditionally in offline7.006C9260 callbacks stage1/2 perform their original control/HUD/text/fade/overlay operations with status and visibility gates. Application rendering occurs before fixed/variable updates, and graphics present follows them. Startup opacity is black for about three seconds and fades during the fourth while real updates must continue.

Boundary A may stop at an actual validation/stage/receiver failure with coherent cleanup. A successful first frame requires original resources and actors, real active camera, completed scene rendering and actual presentation. Full P2.2 acceptance requires user observation after fade, failure return, repeat and second mission coverage. These historical questions are superseded within the later supported Steam/RENDER source scopes above. Actual selected engine and desktop outcomes remain unobserved. [Implementation order](implementation-order.md)
