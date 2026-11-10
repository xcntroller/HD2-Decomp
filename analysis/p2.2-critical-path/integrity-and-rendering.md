# Focused integrity and rendering evidence

Source status `841d89a35c02de2c12c73bb3df8792e758450aed`; original client fingerprint and export digests are in [manifest](evidence/manifest.json). The initial audit used a read-only Ghidra reference query and five targeted exports. This refresh checks the retained evidence and current source. The new Human AI checkpoint leaves the integrity and rendering paths below open.

## Integrity: narrower research gap

006E9EA0's128-entry process initializer and896 shared CRT draws are recovered in MissionIntegrityTables. WORD permutations address DWORD expected/observed fields and separate API payloads. Startup stores the table but has no actual initializer invocation. Original initialization ordering/seed belongs to startup, not each World; inserting it later changes shared randomness. API query implementation is not private observed-state publication.

New P22-R01 direct instruction references identify real writers:

| Private state | Producer evidence | What this resolves / what remains |
| --- | --- | --- |
|00B3BD8C|0099093E in009905B0 writes EAX;0098FAA0 clears on private cleanup|A real producer exists; allocation/index and full startup flow still unresolved |
|00B52F6C|009F02AD in009F02A0; export allocates four bytes then supplies pointer to private external functions|Actual payload producer, not GetTickCount copied into observed |
|00B4C2CC|009F0458 in009F02A0 writes value sourced from00B54BF8|Startup dependency on upstream source; later conditional +96 publication exists only after original checks |
|00B555B0|009F0D01/009F0D39 in009F0A60 allocate four bytes|Branches use loaded84-byte block (00B4FFA0 including WORD00B4FFEC) or external return; no constant healthy constructor |
|00B4EE80..8C|00999952 indexed store in009995F0;009EAC25 passes base address onward|Reference API labels indexed write DATA, so filtering only WRITE references misses producers |
|00B3BD7C|009EEA7C/009EEE71 in009EEA20 set1|Private environment selection exists; zero initial data is not proof it stays zero |

The two payload producer exports009F02A0 and009F0A60 list0099AEB0 as caller. This establishes common startup ownership, **not their relative order or successful branch**. Validation11 at00994DF0 alternates private pointer/index checks versus00B4C2CC==*00B52F6C-96hex. Validation21 cycles four branches; two call009F8C60/009F8D40, another tests private flags/WORD state, one branch succeeds directly. New exports of the two helpers show external function-pointer calls with root literal80000002 and four-byte output buffers, testing masks2/82; a registry-like ABI is an inference pending import/key confirmation. These outcomes are environmental, not mission Scene2 fields.

Existing00996330 action62 validates before publishing matched expected word into matched destination. Action02 uses per-thread return override; raw API return is not observed. This research identifies some producers but does not close validated publication, indirect import semantics, original startup ordering or selected check sequence. Do not force equality, choose a passing counter branch, seed convenient state, disable checks or infer success from private raw zeros. GUARD remains research-blocked for successful activation until the original producer→validation→action chain and initialization ordering are established. A user attempt may diagnose an honest validation failure without weakening it.

Highest-value follow-up is targeted caller order in0099AEB0, allocation/index producer009905B0/009995F0, and upstream00B54BF8/84-byte configuration source—not another full Human export or random API stub. Needed evidence is actual instructions/indirect target resolution plus startup entry sequence. Where unavailable, retain explicit unknown; a desktop black screen cannot establish private-state correctness.

## Rendering: required positive operations

0069B870 is absent as a frozen standalone function, but original static53-byte entry establishes offline7 unconditional actual Scene+78(scene,0), config170 guard otherwise. Host render_session is only its source policy; application render still rejects. 006C9260 is a separate engine callback and cannot substitute for Scene rendering or actual graphics present. Application render precedes fixed/variable updates; presentation follows them.

00486BF0 is a short GEffects wrapper delegating volume224→00482260. Reused00482260 export establishes procedural00482E50 is invoked before visiting retained light records. Its real process0089B63C starts with texture pointer, not an enable flag; loaded textures/buffers require actual draw work. Per-light loop deletes detached-owner records, requires scene visibility relation kind5 and visibility bit, updates camera angular history with wrapped atan2 difference007885B0, performs hemisphere/distance gates and swaps visible records far-to-near before00480D80/00480390 leaves. Empty light count therefore does not prove procedural rendering is inactive.

Decompiler confusion is material: stack aliases in00482260 obscure saved driver/graphics-state variables and apparent call arguments. Keep original instruction verification before implementing state restoration/ECX. The export accesses scene+15C camera-related state, while Sight contract references active camera+128. Do not silently identify these two native fields; verify actual engine scene interface/visibility camera roles at binding. Existing source scalar tests for draw leaves do not establish original visibility or first phase history.

Known evidence supports real saved driver3/graphics13/14 restoration, same camera world-transform dirty updates and positive texture/geometry ownership. Missing evidence remains initial per-volume3C/40 phase/history; do not invent a spin angle. Captured BASE Tutorial zero2C values do not prove active override or process absence. Original00483E50 clouder/clouder2 flags502/two3CC00 buffers and00483DE0 release remain upstream owner contracts.

0069AE10 warming fade and00486C10 startup fade are different. Startup opacity stays black through3000ms and fades during the fourth second. Execute real fixed/update/render while observing this; forced pause/fade suppression is not a valid frame. HUD/text/overlay gates do not remove their required positive consumers;006E8490 defaults enabled with its own editfont/page owner. Preserve null texture and exact raw versus boolean state restoration contracts.

Boundary A: correct real callbacks/services and honest failure propagation, with no success substitute. Boundary B: Scene render completion, callback stages and actual desktop present plus original world/camera visibility after fade, then failure/repeat. The latter requires user desktop execution; this research does not claim it.
