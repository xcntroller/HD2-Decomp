# Graphics input setup and startup localization

This source-only slice follows manager checkpoint `46359bd`. The services after
input-manager attachment in `006DB620` are **language/text initialization**, not
profile/control loading. Their original bodies are now recovered alongside the
input setup sequence. The live host still uses its established staged menu text.

## Input setup order

[`input_setup.h`](../recovered/frontend/input_setup.h) reconstructs `006DB620`:
query graphics; initialize keyboard through +90 with flags zero; clear manager
bindings; snapshot the actual joystick list/count; query graphics again; attach
manager; resolve the application language; preload localized text. Initialization
and attachment results are ignored. The joystick view precedes the second
graphics query, which may return a different object after a callback.

The recovered manager, defaults and producer remain independently composable.
This setup does not install control defaults: the original tail handles text.
Concrete device setup/list providers and remaining application/state startup
services still precede live controller integration. No empty-string or successful
no-op services are used to force that integration.

## Language cache and wide-character input

[`startup_language.h`](../recovered/frontend/startup_language.h) reconstructs
`006B67B0`, the normal self-query comparison/directory helpers `006B6750` and
`0048FBC0`, and executable-directory cache `006E8ED0`.

The resolver opens HKLM `SOFTWARE\Illusion Softworks\Hidden & Dangerous 2` with
access 1, queries `language` into a zeroed 128-byte buffer without requesting its
registry type, and falls back to literal `english` on open/query failure. It
preserves the returned spelling. Cache installation precedes closing the key.
A nonempty cache returns immediately without recomputing the wide-input flag.

After first installation, case-insensitive Japan/Russian matches force the flag
on. Other languages probe `Text\<language>\\FontConfig.hxml` using original
`fopen("r")`/close behavior (`006ED860`). The doubled separator is intentional:
the directory helper already returns a trailing backslash. The resulting flag
is original application +30 / global 008AE92C, consumed by the recovered keyboard
preprocessor's ANSI-versus-wide character route. This is an existence probe,
not font-configuration parsing or renderer integration.

Self queries see the installed cache before flag computation finishes. Native
source supports that normal application path; arbitrary derived virtual getters
and original refcount/string/class ABI are outside scope. Comparison matches the
original CRT's default C-locale byte folding, not locale-specific tables/locking.
An empty successful registry value would recurse indefinitely in the original;
native code detects recursive empty resolution. Unterminated registry/module
buffers throw instead of overreading. Native registry exception cleanup closes
the key when string construction/validation fails.

Executable-directory caching calls GetModuleFileNameA with capacity 260, ignores
its return, and cuts at the last backslash only. Forward slashes are not treated
as separators. An empty cached result is queried again next time.

## Text loading and parsing

[`localized_text.h`](../recovered/frontend/localized_text.h) reconstructs:

| Entry | Source behavior |
| --- | --- |
| 006B1090 | Enumerate `<executable-root>\text\<language>\*.txt`, filter names and load relative DTA paths |
| 006E4CF0 | File open/size/seek/read gates, BOM handling, diagnostics, parsing and normal close rule |
| 006E5300 / 006E5320 | Split at CR/LF with bounded scanning; skip CR/LF runs and parse every line |
| 006E5240 | Signed decimal prefix and first/last quote extraction on a single C-string line |
| 006E5060 | Signed-ID sorted cache, reject -1 and duplicate IDs; first text wins |

Enumeration preserves filesystem order, skips `.`, `..` and `dialogs.txt` with
case-insensitive comparison, and ignores each file-load result. It loads through
relative `text\<language>\<filename>` paths, so the original DTA search/archive
context remains required. A failed first enumeration has no close; a successful
search closes after its final next call. Native exceptions additionally close
the search rather than leaking it.

File loading requires a non-invalid open handle, positive signed size, seek to
beginning returning zero and a full-length read. UTF16LE BOM FF FE is rejected,
logging the original Unicode diagnostic followed by FAILED. UTF8 BOM EF BB BF is
skipped without transcoding the remaining bytes. Parsing an empty/comment-only
file still succeeds if it passed file gates; a zero-byte file fails. Normal
cleanup preserves the original **close-only-handle-zero** condition, including
on failures. This is recorded without assuming the delegated library's handle
semantics. Tiny BOM probes are bounded in native source instead of reproducing
original out-of-range reads.

Records use a signed `%d`-style prefix and the first and **last** quote on the
same line. There is no multiline concatenation, escape decoding or quote-aware
comment suffix. Embedded NUL terminates record parsing but not raw line advance.
Other suffixes after an integer are accepted, empty quoted strings are accepted,
ID -1 is rejected, and earlier duplicate IDs remain. Native signed numeric
overflow throws rather than relying on CRT overflow behavior. Raw UTF8/legacy
bytes are retained. Native `std::map`/strings replace original sorted-vector and
reference-count allocation; unused original padding is not reconstructed.

## Concrete platform boundaries

[`host/startup_localization.h`](../recovered/host/startup_localization.h) supplies
actual RegOpenKeyExA/RegQueryValueExA/RegCloseKey, CRT file probing/enumeration,
GetModuleFileNameA and original `rw_data.dll` file exports. The DTA adapter borrows
a loaded module and delegates `_dtaOpen@8`, `_dtaSeek@12`, `_dtaRead@12` and
`_dtaClose@4`; names match both client imports and installed DLL exports. Archive
registration is not invented. This library stays outside the frozen client/
LS3DF census. Original logger `006ECED0` remains separate; native diagnostics use
OutputDebugStringA with original messages.

The concrete Windows adapters strictly compile and link for i386, including
advapi32. They have not been desktop-executed in this checkpoint. The existing
host has no new switch, renderer or input behavior.

## Validation

Temporary ASan/UBSan and strict Windows i386 audits cover registry open/query
fallbacks, installed spelling, cache-before-close, cached flag preservation,
Japan/Russian and font probe rules, recursive/unterminated inputs, module-cache
separator/retry rules, all 65,536 byte pairs for default case equality, 10,000
record variants, signed ID boundaries, duplicates and quote/comment behavior.

An independent C-string/sscanf reference matches all **2,162 records from installed
TEXTY.txt**, **428 from TEXTY_DD.txt**, and zero from license.txt. File audits cover
invalid/zero/negative sizes, seek/short-read failures, both BOM paths, tiny files,
handle-zero/nonzero cleanup and diagnostics. Enumeration checks filters, order,
failed-file continuation, duplicate precedence and close behavior. Actual manager
setup uses a changed second graphics pointer and preloads text despite keyboard
initialization returning failure, matching the ignored original result.

Temporary audits are removed after verification; no permanent tests or agent-side
Proton launch are added. Existing native checks and all 158 tooling tests pass;
the live host build is up to date. Evidence/accounting/diff checks also pass. The source-only input-setup/localization checklist entry is complete;
live integration, archive/device setup, rendering/localization consumers and full
application startup remain open.

Twelve new partial client entries: 146 total (141 client/five engine), zero
complete, frozen denominator 18,783. The [map](startup-localization-map.json)
checks both original fingerprints, 15 byte anchors, 16 path/message resource
references, 80 direct calls and the keyboard initialization slot. Read-only exports remain ignored
under input-setup-application-services and startup-localization-* directories.
