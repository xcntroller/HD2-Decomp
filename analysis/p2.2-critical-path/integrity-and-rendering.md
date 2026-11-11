# Integrity and rendering after overnight recovery

Source review `799a79305c70bc8de1860ac7e438bba25a71a8ac`. Original client SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`. This reassessment reused prior exports and ran two sequential read-only/noanalysis Ghidra queries for callsites, short instruction windows and known static control flow. Raw material remains ignored. [Evidence manifest](evidence/manifest.json)

## Integrity initializer lifetime

The direct call at `006DB104` invokes `006E9EA0` near the beginning of `006DB0F0`, before the first graphics initialization operations. Its retained export shows no mission or offline-mode gate at that call. This establishes a device-startup invocation boundary, with process-owned storage. A device initialization retry can invoke it again according to that original caller. Preserve its position relative to later shared randomness. Per-mission reseeding would change the original sequence.

`006E9EA0` initializes both 128-entry WORD permutations, calls through `00A59000` at `006E9EBC` for the seed, seeds the shared CRT, and consumes exactly 896 draws. The original private wrapper's actual seed API and validation relation remain unresolved in this bounded investigation. Current StartupSession exposes MissionIntegrityTables but supplies no actual initialization or validated observation publication. Existing expected, observed and API-payload storage remain distinct. [Original findings R07](evidence/original-findings.json)

## Private producer chain

| Original owner or field | Newly verified evidence | Remaining contract |
| --- | --- | --- |
| `0099AEB0` | Callsite009C5335 invokes009F0A60,009CAD19 invokes009F02A0,009CAE28 invokes009905B0. The earlier call block dominates the later call block in the known CFG. | Actual successful environment branch and global startup entry relation |
| `009905B0` | Push40hex and allocation at00990934, publication to00B3BD8C at0099093E | Allocation is16 DWORDs. Payload initialization/population cannot be inferred from allocation |
| `009995F0` | Conditionally called at00992A9A inside009905B0.00999952 selects indices for00B4EE80 according to table/status gates | Four indices later used by validation11 require genuine population, not arbitrary zero records |
| `009F02A0` | Prior export allocates00B52F6C, obtains private payloads, writes00B4C2CC from00B54BF8 | Original external-function semantics and upstream00B54BF8 still unresolved |
| `009F0A60` | Prior export allocates00B555B0 from a loaded84-byte block/WORD00B4FFEC or an external result | Configuration/environment source and correct branch remain unresolved |

The targeted CFG contains495 known blocks and one outgoing edge outside the owner. Entry reaches all three call blocks.009F0A60 precedes009F02A0 and009905B0 on the known paths that reach those later calls.009F02A0 precedes009905B0 similarly. Reverse reachability between these call blocks is absent. This is evidence-backed static ordering with a stated CFG limit. It does not establish successful dynamic startup or indirect target behavior. [Findings R06](evidence/original-findings.json)

Private entry0099AEB0 has a known caller at00A4D84B in00A4D782. Relating that protected entry to the normal application startup and its environment setup remains part of GUARD. No claim that the private producer chain can be skipped for offline7 is supported.

Validation11 at00994DF0 alternates indexed00B3BD8C checks against the00B52F6C/00B4C2CC relation. Validation21 has environment-sensitive branches including009F8C60/009F8D40. Their root80000002 and four-byte query ABI suggests registry-style operations, but exact imports/keys remain unconfirmed.00996330 validates before action62 publishes the matched expected word to the matched destination. Action02 uses per-thread return override. Correct observed-state publication therefore requires the actual producer/validation/action chain.

Offline7 does reach meaningful guard consumers. The loader/activation uses the original wrapped APIs. The retained0069BA60 objective path compares stage13 and draws only on mismatch, conditional on a real objective call. The initializer itself precedes mission selection. The exact subset of descriptor validations reached on the selected first-frame path remains unresolved. Preserve real validation and shared CRT ordering. Do not synthesize healthy observations or convert raw API returns into observed values.

## Corrected render owner and clock

Original instructions00486BF0..00486C02 read GEffects+224 into EAX, addC4 to ECX, push EAX, then call00482260. **The volume owner is GEffects+C4 and the argument is clock224.** Our previous volume224 wording incorrectly described the receiver. Current EffectsModuleLoading already owns volumes and EffectsLoadingProcess.field224 separately, and the latter advances with loading updates. No current source mismatch is asserted for those owners.

At004822C1 the caller sets ECX=0089B63C and pushes the time argument before00482E50. This process render call occurs before per-light visibility traversal.00482E50 checks its first texture pointer and can genuinely return when absent. A populated texture requires its actual procedural geometry/draw operations. Empty light vectors alone cannot determine this branch. Preserve the existing clouder/clouder2 textures, flags502 and two3CC00 buffers. [Original findings R08](evidence/original-findings.json)

00482260 then reads the real scene+15C camera-related frame, updates dirty transforms, computes camera angular change, checks each light's scene visibility relation, hemisphere and distance, and swaps visible records far-to-near before the recovered draw leaves. Sight uses scene+128. The relation between those native camera roles and first-frame camera setup still needs original evidence. Original initial per-volume3C/40 history remains unknown. Baseline Tutorial field2C zero does not determine active Patch or process texture state.

Driver3 and graphics13/14 restoration must follow the native save/restore semantics. Stack aliasing in the decompiler makes typed scalar fixture arguments insufficient evidence for the containing visibility renderer. Existing leaf tests validate their own numerical/state boundaries only. The application still lacks real pre/post scene render consumers.

## First presentation boundary

Original0069B870 calls actual Scene+78(scene,0) unconditionally in offline7.006C9260 callbacks stage1/2 perform their original control/HUD/text/fade/overlay operations with status and visibility gates. Application rendering occurs before fixed/variable updates, and graphics present follows them. Startup opacity is black for about three seconds and fades during the fourth while real updates must continue.

Boundary A may stop at an actual validation/stage/receiver failure with coherent cleanup. A successful first frame requires original resources and actors, real active camera, completed scene rendering and actual presentation. Full P2.2 acceptance requires user observation after fade, failure return, repeat and second mission coverage. The remaining private environment and camera/history questions are explicit original-code unknowns. [Implementation order](implementation-order.md)
