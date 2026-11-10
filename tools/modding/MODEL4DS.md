# Read-only 4DS model inspector / OpenGL viewer — 2026-11-10

This checkpoint adds an actual model viewer to the existing application. Open
**Model inspector…**, then **Open installation…**, search for a resource, and open
it. The same viewer opens from Mission Explorer's **View model** button. No game,
DLL, Proton, engine or original code is executed. Original assets stay read-only.

The decoder is `model4ds.py`, frame math is `model_math.py`, model/texture resource
handling is `model_resources.py`, and the GUI/renderer are `model_gui.py` /
`gl_view.py`. Parsing and deterministic metadata export have no Tk/OpenGL dependency.
The new implementation changes only `tools/modding`.

## Evidence and format boundary

Existing contracts were reused first: [engine model map](../../analysis/engine-model-map.json),
[header decoder](../../recovered/scene/model_file_header.cpp),
[first table](../../recovered/scene/model_first_table.cpp),
[model load policy](../../analysis/model-load-policy.json),
[resource directories](../../analysis/resource-directories.md), and the existing
installation resolver. Neither the reconstruction's loader nor cache was changed.

Existing local decompilation exports cover 1005D970/1002BBE0/1002BE30. Subsequent
frame/mesh layout was checked by static `objdump` reads of the **same H&D2**
LS3DF.dll, SHA256
`12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`.
No Ghidra was launched. [model4ds-evidence.json](model4ds-evidence.json) records
function/slot addresses and bounded code hashes rather than raw captures. Normal
operation does not require the DLL. No other game's 4DS conventions are adopted
as evidence.

| Original address | Established boundary |
| --- | --- |
| 100340B0 →1005D260 →1005D970 | Model open wrapper/worker/file-reader chain; not a reconstructed runtime cache |
| 1005D970 | Exact `4DS\0`, LE version0x29, opaque8 bytes, material/first-table count<=6000, uint16 frame count, frame dispatch and one-based parent IDs, final byte |
| 1002BBE0/1002BE30 | Existing first-table layout; actual calls pass primary/secondary/extra names to the graphics texture service; sequence metadata |
| 10019750 + vtables1009C7C8/1009C6D8/1009C5F8 | Visual subtypes0/1/2 and reader slots at+70; no subtype names inferred from another game |
| 1001EC40 | Position12 bytes, quaternion/scale/unknown/flags33 bytes, two byte-length strings |
| 1001C060 | Dummy common frame plus32 bytes, retained opaque |
| 10020BA0 | Joint common frame plusDWORD, retained opaque |
| 1009B960+70 →1001EC40 | Kind11 common-only serialized body |
| 10038410 →1005DDD0 | Common visual frame followed by shared geometry reference or new LOD geometry |
| 1005DDD0 | One-based mesh alias, <=10 LODs, metric/flags/vertex count,32-byte base vertices, additional/special vertex branches, triangle groups/material IDs |
| 10030DB0/100311E0/10030F30 | Native32-byte base allocation and position/bounds consumption |
| 10031CE0 | Additional stream stride masks; deliberately unsupported by this decoder |
| 10053900 | Visual2 common mesh, counted opaque binding records and per-LOD word arrays; no deformation reconstruction |
| 1002D450/1001E390/1002D830 | Serialized XYZW rotation, row scaling and local-times-parent row-vector matrices |

### Confirmed structural decoding

All fields below are little-endian. Counts and offsets are checked before reads
or allocations. Byte-length names preserve Latin-1 byte values, including embedded
NUL; they are not silently repaired into valid filesystem paths.

- Prefix16 bytes: signature4, version2, opaque8, first-table count2.
- First-table record: flags4, opaque72/76 depending on bit0x80000; optional extra
  byte-length name, primary byte-length name, and secondary name only under the
  established nonempty-primary /0x40000000 /!0xA1000000 conditions. Bit0x04000000
  adds two sequence-count bytes and16 opaque bytes. Raw parameter bits and a
  float view are retained, without inventing diffuse/ambient/specular semantics.
- Frames: kind byte; for kind1 a subtype byte and flags word; parent word. Parent0
  is root, otherwise an earlier frame's one-based identity. Common payload is
  position3 floats, quaternion4 floats, scale3 floats, unknown float, flags byte,
  name and properties byte-length strings. Parent/geometry IDs are not list-name
  guesses. Stable IDs combine the resource identity and original member offset.
- Visual0/1 geometry: word reference0 means a new mesh; nonzero aliases an earlier
  frame's actual geometry. New geometry starts with a byte LOD count. Each LOD
  has float metric, DWORD stream flags, word vertex count, base vertices, byte
  group count; each group has word triangle count,3 uint16 indices per triangle
  and word material reference. Material0 is absent, other references are one-based.
- Only base stream_flags0 with ordinary vertex counts is supported. `0xFFFF`
  vertex reuse and all additional stream layouts stop parsing explicitly. LOD
  metrics are retained as uninterpreted values; the UI selects LOD indices manually.
- Dummy6 adds32 opaque bytes, joint10 adds4, kind11 adds none. These exact sizes
  are established, but their payload semantics remain unknown. Small opaque
  payloads retain raw hex; large suffixes retain offset/size/SHA256.
- Visual2 binding tails are length-established: byte count; if nonzero,32 opaque
  bytes, count bytes, count96-byte records, then one DWORD used count and that
  many words per mesh LOD. Counts exceeding LOD vertex capacity reject. The
  geometry is shown **undeformed**, with explicit partial status. No posed or
  skinned-character correctness is claimed.
- Unrecognized frame kinds/visual subtypes stop at the record's original start.
  No scanning for plausible names or next records is used. Safely decoded earlier
  geometry can remain visible on unsupported input; malformed supported input
  withholds **all** drawable geometry transactionally.

### Attribute interpretation and transforms

Position/32-byte stride are established by the native bounds/allocation readers.
The preview uses float3 at0 for position, float3 at12 for normals, float2 at24 for
UV. The normal/UV interpretation is corroborated from actual H&D2 geometry and
texture mapping: all2071 sampled normals in six models are unit length, with face
correlation except expected smoothed/degenerate cases, and actual BMP textures
produce coherent original surface details. Original graphics vertex declarations
are **not** independently reconstructed. This confidence distinction is retained
in each LOD's `attribute_evidence`; raw field values are preserved unchanged.

The confirmed transform is a row-vector matrix with translation in the last row,
per-row XYZ scale, local*parent composition and serialized XYZW. SetRot3's
abs(w)>=1 identity branch is retained; the loader does not insert a normalization
call. Derived matrices/bounds use Python floating arithmetic and are not claimed
bit-identical to x87/FLOAT spills. OpenGL receives the transposed representation
by interpreting the same row-major bytes as column-major. Normals use OpenGL's
normal transform/normalization for display. Original coordinates are not flipped.
Y-up is the display/camera convention; no unsupported handedness or unit claim is
made. Bounds cover actual decoded LOD0 vertices only. Unknown frames are excluded
from partial-model bounds. Selection-origin crosses/grid/axes/bounds are viewer
annotations, never substitute meshes or inferred actor placement.

## Statuses and resource resolution

- `decoded`: supported structural records consumed. This does **not** mean every
  material property, flag or renderer behavior has known semantics.
- `partial`: length-established opaque fields, binding/sequence behavior or a
  safely stopped unsupported suffix remain; earlier verified geometry is usable.
- `unsupported`: version/layout/tool limits prevent supported decoding without
  a prior usable frame boundary.
- `invalid`: malformed/truncated supported fields, illegal indices/references or
  nonfinite/excessive numeric attributes. No geometry is rendered. Numerical
  guards are tool safety restrictions, not a statement that the original engine
  rejects every such asset;12 original corpus members hit this attribute guard.
- Resource `unavailable`/`invalid` statuses remain separate from model parsing,
  including unavailable codecs/CRC failures. Previous model geometry is cleared
  after a failed selection rather than left looking like the new resource.

`ModelResources` uses the existing ResourceStore's exact selected virtual path,
profile, Patch/loose/base precedence and checksum/codec barriers. It never opens
a shadowed copy when selection fails. File/archive provenance is visible and
exported. The independent browser lists `Models/*.4ds`; mission-local .4ds files
can also be opened through their recognized Mission Explorer entry or the CLI.

Texture primary/secondary/extra references preserve their original spelling and
roles. For an explicit Maps/Maps_C/Maps_U virtual path, use that path. For bare
names, enumerate exact name candidates under those three established registered
roots. A **unique** candidate can be previewed, labelled
`registered_root_name_candidate`; multiple candidates are ambiguous and render a
fallback. Registration order is not invented as cache lookup order. Selected
invalid resources stop at that candidate, without trying another root/archive.
The actual graphics cache/search policy remains unreconstructed. Mission object
model candidates keep their pre-existing `name_candidate` classification; opening
one is an explicit user selection, not a newly asserted actor-to-model mapping.

## Materials and textures

Only verified local BMP inputs are decoded: BITMAPINFOHEADER40, BI_RGB,
8-bit indexed palettes and24/32-bit BGR/BGRx, row padding, top-down/bottom-up images.
RGB output was compared byte-for-byte to Pillow for actual selected textures;
Pillow is an audit dependency only. Alpha/palette reserved bytes are not guessed.
DDS, TGA, compressed/extended BMPs and unsupported image formats remain explicit
fallbacks. No filename-extension substitution, generated textures or alpha-map
composition is performed. Secondary/extra maps and sequences remain resolved but
uninterpreted, never reported as successfully displayed original effects.

Materials show primary RGB texture previews when available, or a clearly labelled
neutral gray material. Lighting is a conventional OpenGL preview; original
colors/blending/alpha/specular/effects/culling and renderer globals are excluded.
All supported frames are shown; serialized visibility flags and automatic LOD
policy are not evaluated. This approximation remains separate from file-decoding
status. Untextured source
materials also use a neutral preview, without claiming their original color.

## GUI and export

This is an additional child window of the existing toolkit, sharing its owner and
Mission Explorer, rather than a second application. It works independently of a
mission through a local file or installation browser. Mission Explorer recognizes
.4ds files, browses model frame/material records, records texture dependencies,
and opens the same viewer. Following a model candidate from an actor record
retains a return link to that original record. No mission placement or approximate
actor filename link is fabricated.

Left drag orbits; wheel zooms; middle/right or Shift+left drag pans. Controls select
wireframe/solid, textures, grid, axes, bounds and LOD. Hierarchy selection highlights
meshes or displays decoded frame origins. Properties retain identifiers, offsets,
transforms, material references, sample vertex/triangle values, diagnostics and
provenance. Typed packed arrays expose all decoded geometry to future tools;
metadata reports intentionally embed only bounded samples rather than duplicating
entire asset buffers. The JSON schema is model-specific version1; mission exports
remain version2 with additive recognized records/references.

```
make -C tools/modding gui
python3 tools/modding/model_inspect.py models/ammocrate.4ds \
  --installation /path/to/game --output /path/outside-game/model.json
python3 tools/modding/model_inspect.py /path/to/local.4ds
```

Add `--archives-first` / `--language English` for an explicit installation profile.
`model4ds.py FILE` is also a decoder-only CLI. New exports must be outside protected
input roots and never overwrite a source. Reports are deterministic for unchanged
inputs/profile. Reports can contain proprietary names/samples and must not be
committed as fixtures. CLI exit0 means decoded/partial report produced;1 means
invalid/unsupported model;2 means invocation/resource/export failure. Check status.

The viewport uses standard-library ctypes with system libGL/libX11, Tk and a GLX
compatibility context. No Python rendering package, game engine, C/C++ headers or
new pip installation is needed. Linux X11/XWayland is supported; unavailable GLX
is reported without fabricated rendering. Geometry/textures decode on one worker;
all GL/Tk calls stay on the GUI thread. Idle rendering is event-driven. Close and
reload free context/texture/native-window ownership. Standalone local files have
no implicit installation/texture search context.

Bounds:64MiB input/member,6000 frames (tool restriction), native6000 materials and
10 LODs,1million vertices,2million triangles,4096 per texture dimension/8million
pixels,64 texture dependencies considered for decoding and32MiB total decoded RGB.
Identical images share one GL texture allocation. Model loads replace the previous
asset; archives are never retained as entire byte buffers. Scene discovery indexes
models without automatically loading all mesh data. Resource and hierarchy views
page or bound previews; JSON streams metadata. These are explicit safety limits,
not original engine capacity claims or hostile concurrent-filesystem isolation.

## Verification actually performed

No original code execution/Ghidra/heavy game builds were used. All assets were read
from the authorized local installation/research copy. Temporary fixtures/captures
were outside Git and removed after verification; no original assets are committed.

| Actual model | Structural status | Stored vertices / triangles (all LODs) | LOD0 instance triangles |
| --- | --- | --- | --- |
| ammocrate | decoded |144 /72 |72 |
| k_barelmir | decoded |116 /98 |60 |
| b_church | decoded |48 /24 |96 (shared meshes) |
| b_us_helma | decoded |309 /329 |208 |
| dk_a_bedna01 | partial (opaque dummy payloads) |463 /204 |204 |
| fpv_hands | partial (joints/bindings, undeformed) |991 /1526 |1526 |
| bike_legs | partial |1107 /1734 |1734 |
| (pes | partial |427 /636 |636 |
| d_horse | partial |1119 /1076 |1076 |

- These nine models matched an independent temporary C++ stream reader using
  existing recovered header/first-table code and independently written frame/mesh
  traversal. Counts, aliases, parent/index/material bounds, byte offsets and file
  endpoints matched. Packed vertex/index arrays matched original source slices.
  First-table records also matched the established Python inspection tool.
- A3444-member selected Models corpus, excluding names beginning #/!/%/&/+,
  yielded1496 decoded,1916 partial,20 unsupported,12 invalid numeric guards.
  The pass checked established table endpoints, parent constraints and all decoded
  index bounds. Many partials have fixed opaque dummy/joint fields; others stop
  at unsupported visual3/4/5/6/8 or frame5/7/12 records. This is neither a complete
  installation census nor proof of all-format compatibility. Peak RSS80244KiB in
  this sequential decoder pass; no mesh archive was loaded wholesale.
- Temporary synthetic fixtures checked every truncation byte of a minimal valid
  model, every truncation byte of an indexed BMP, invalid parents/aliases/indices,
  unknown-type stops, palette/padding/orientation, deterministic exports, row-vector
  transform fixtures and200 malformed byte inputs. Resolution fixtures checked
  missing/ambiguous/invalid/unsupported textures and no shadowed/root fallback.
- The independent native nine-model reader passed ASan/UBSan/LeakSanitizer outside
  sandbox ptrace supervision. This checks the audit reader and existing contracts;
  the new decoder is Python, not a claim that system graphics drivers were sanitized.
- Actual Tk/OpenGL rendering ran on Mesa26.2.6 compatibility4.6, AMD Radeon RX9070XT,
  X11/XWayland. Native checks exercised real textured crate/barrel/building/helmet,
  complex prop, undeformed hands, hierarchy highlight, wireframe, LOD switching,
  search, repeated replacement and context teardown/reopen. A focused16-cycle
  create/load/render/destroy check measured RSS162616→165588KiB (about3MiB
  cumulative cache growth); the broader rendering/export/screen-capture audit
  peaked at432300KiB. This is bounded-input evidence, not a zero-growth or
  graphics-driver leak-proof claim. Selected Patch
  `models/b_drevo2.4ds` decoded with verified ISD1 checksums and rendered16
  vertices/18 triangles. Native X11 mouse events exercised orbit/pan/wheel paths.
- Rendered images of actual decoded geometry/textures were visually inspected,
  beyond automated assertions. The real crate surface detail, upright barrel
  lettering and helmet/strap geometry were visible. This is actual rendering
  verification, not new human desktop acceptance or original renderer equivalence.
- Actual Tutorial actor candidate →`models/en09.4ds` (partial), ALPS1 actor
  candidate →`models/e_elbx1.4ds` (decoded), viewer open and return to the original
  actor record passed. Candidate classification was retained throughout.

Excluded: unsupported record families/additional streams, full graphics cache
resolution, alpha/effects/animation/skin evaluation, 3D mission placement, editing,
binary serialization, runtime/gameplay/debugging and full-format compatibility.
