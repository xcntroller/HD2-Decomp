# Standalone Windows front-end package — 2026-11-02

Section5's packaging checkbox is **implemented, built, statically/native verified
and user desktop-accepted2026-11-02**. The prior
[bounded traversal](phase1-traversal.md) remains accepted with its explicit gaps.
No Phase2 research or commit/push is started by this packaging milestone.

The subsequent user request authorizes a separate
[original-resource/fullscreen follow-up](resource-fullscreen.md). Its revised
package has a separate desktop gate; this document's artifacts and acceptance
below describe the preserved earlier package.

## Concrete artifacts

- Portable folder: `dist/HD2-Phase1/`; entry `HD2FrontEnd.exe` or `Launch.cmd`.
- ZIP: `dist/HD2-Phase1.zip`,2,040,485,277 bytes (about1.90GiB),554 payload files.
- Client SHA256: `3547347cd6d54fb4f7a730d6a6653ac6cc3171b47e43a616cfae4cf03fe6070b`.
- ZIP SHA256: `c22aa63571af49828d0ac0f44f8c313cb98ff07ce06db30e2eb29d14ce9aaa2e`.
- [Launch/platform/dependency instructions](../docs/STANDALONE.md) are also
  copied into the package as README.md. package-manifest.json lists payload
  sizes/hashes, bundled DLLs/imports and supported language directories.

These local generated assets are ignored under `/dist/`. The ZIP/folder includes
the user's installed base data and original DLLs; no original game/server EXE,
source checkout, Python runtime or compiled-toolchain DLL is required in the
extracted runtime. Platform services listed below remain required.

## Completion boundary and dependencies

The package includes the entire current compiled client implementation, available
base archives/loose assets, four opening movies, installed Cinematics/previews,
menu models/textures/click audio and original font sets/translations. It preserves
the existing staged implementation's resource policy: baseline extracted menu
resources plus actual registered archives, no fabricated fallback artwork.
Expansion-only archives and unrelated installed executables/DLLs are excluded.

Static PE import closure proves the seven included non-system dependencies:

| Dependency | Import/lifetime boundary |
| --- | --- |
| LS3DF.dll | Original graphics/3D/input/sound/IShow engine. Frozen SHA256 `12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee`. |
| rw_data.dll | Shared archive/file/codec DLL; frozen SHA256 `95a65ee003e2d7ab6047ed47d9adad1db330bd5482e33cfb18127d15af5879be`. |
| IJoy.dll | Explicit native module owner; frozen SHA256 `f3e896afe35894609688e634a54929fd91f6a53ecfc8c43f30d83d8b96ec4ee7`. |
| binkw32.dll | LS3DF movie-decoder import. |
| vorbisfile.dll | LS3DF audio-decoder import; depends on vorbis/ogg. |
| vorbis.dll | Transitive audio-codec import; depends on ogg. |
| ogg.dll | Transitive audio-codec import. |

All seven original DLL copies have individual source/output hashes in the
package manifest. System DLLs/API sets are classified separately, including
Win32/COM/WinMM, UCRT, Direct3D8, DirectInput/DirectSound; DirectPlay8 is the
existing dynamic COM discovery dependency. Static closure does not certify OS
availability or actual native Windows/codec/device behavior. No DLL is executed
by the builder or agent. Original SDK operations remain delegated, without new
function-body credit. [Host-boundary audit](host-boundary-audit.md).

## Native entry and fresh user state

`application_launch_options.h` provides the native packaging policy. An argument-
free double-click resolves the executable's own directory, uses adjacent
LS3DF.dll/IJoy.dll, selects800×600/frame-limit0 and retains original language
selection. Optional width/height/language/frames need no wrapper. `--help` prints
usage without loading engine modules. The existing `EXE ENGINE --frames N ...`
developer/Proton CLI still parses identically for valid existing commands.

The entry still changes into its own staged directory and requires the existing
`.hd2-launch-workspace` marker before resource/log/module work. This is a native
launcher adaptation, not recovered original command-line or setup.exe behavior.
The menu/controller/input/music implementations are unchanged by packaging.

The builder adds `seed_user_state=False` to real startup staging; existing callers
keep defaultTrue and their previous preservation behavior. Every package uses a
new private temporary workspace. It imports **no PlayersProfiles data or game.cfg**
and preserves an empty PlayersProfiles directory in the ZIP. It also excludes
runtime logs, compatibility prefixes and original keys/configuration.

This uses confirmed original missing-file behavior: configuration006A60A0 clears
LASTPROFIL/PARENTLOC/NETCDKEY when no file is present; profile006AF1D0 keeps an
empty selected name; Main00627520 offers Create Profile and Exit while profile-
dependent actions stay unavailable. The user creates/selects a real profile to
enable the menus. No successful fabricated profile or unlocked movie set is
seeded. Subsequent writes belong to that extracted folder.

## Builder and verification

[tools/package_application.py](../tools/package_application.py) composes existing
resource stagers, validates the three frozen DLL identities, walks static PE32
imports for codec closure and copies the actual local dependencies. It rejects
unknown/missing imports, linked files, ambiguous Windows DLL/output paths,
installation/output overlap, existing outputs and accidental user/config or extra
EXE payloads. ZIP64 entries retain relative paths and empty directories. All
payloads are assembled privately before publication; existing package directories/
ZIPs are refused to protect any user state in an older extracted package.

Verification passes:

- Three strict i386 application targets. Temporary native ASan/UBSan audit of
  actual argument code: argument-free/default, root/UNC/spaced paths, legacy
  parity, limits/repeats/unknown/missing options and language validation. The
  fixture also compiles/links strictly for Windows i386, without execution.
- Temporary Python packaging audit: PE/import parsing, malformed import ranges,
  transitive/system closure, actual clean-state startup staging with synthetic
  PE/resource boundaries, ZIP hash/CRC/empty-directory handling, existing-output
  preservation, unsafe path/dependency/core-hash rejection, private staging
  cleanup on failure and rejection of accidentally seeded user state. Production
  dependency guards are patched only inside that isolated fixture context.
- Real package: every output matches its manifest size/SHA; every ZIP payload is
  streamed and hash/CRC checked against the folder; only one EXE and no user
  profile/config/log/source/Python files; all seven DLL imports resolve to the
  package or declared platform imports. Client matches the new built target.
- All572 files in the original installation retain their hashes. All eight
  existing staged writable files and accepted staged executable retain theirs.
  The older accepted client is also preserved under the audit work directory.
- Existing native regression suite,158 Python tests, frozen ledger and whitespace
  checks pass. Temporary audit scripts/source/native/Windows binaries are removed;
  no permanent tests added. Logs/hashes:
  `analysis/work/standalone-package-20261102`, including verification.json and
  artifact-verification.json. No original game/DLL, Windows fixture or Proton run.

The previous accepted staged client remains
`e11fb644abd669a04deae5b56bb095763ceec59401ad391ca76718cbcb9384a8`;
the newly built/packaged client has the new hash above. Do not claim those two
builds are identical or stage the new client over the accepted workspace merely
to manufacture agreement. Original counts remain **564 partial (559 client/five
engine), zero complete /18,783**. No new function credit, commit or push.

## Desktop review supplied with the package

Extract the complete ZIP to a writable folder separate from the repository/game
installation. On Windows, launch HD2FrontEnd.exe directly with no arguments. On
the user's existing Linux setup, use the relative portable Proton command in
the included README with an isolated prefix. The agent cannot perform this check
under AGENTS.md's original-DLL/Proton execution prohibition.

Confirm opening movies reach Main/Create Profile, create/select a profile, enter
one available menu and Exit. Reopen the extracted EXE with a width/language option
and confirm the package's profile/settings persist. This checks the new entry,
clean-state and relocated resources; no repeated exhaustive route tour is needed.
These were the supplied review steps; the acceptance below records the user's
overall result without claiming a separate report for every suggested case.

## Desktop acceptance — 2026-11-02

The user reports: “The build works fine, by the way.” This accepts the delivered
portable build and closes section5's packaging checkbox. The client/ZIP identities
above remain unchanged. No additional agent desktop execution or exhaustive
traversal is claimed. Creation-time manifests/logs and the bundled README's
pending-review wording are historical; this later acceptance supersedes them.
The accepted folder/ZIP have not been regenerated or edited for this record.

## Loose resource layout and archive fidelity

The user asks why Models/MenuText/Sounds and BMP/TGA files in Maps are loose.
The portable package preserves the development stagers' layout; it does not
reproduce the original installation's physical resource layout.

- Six Models/*.4ds files and Sounds/button.wav are unchanged members extracted
  from models.dta and Sounds.dta by tools/stage_menu_resources.py. Those original
  archives are also bundled. These copies originated in early menu staging;
  archive registration and client model/sound caches are already integrated.
- Maps contains32 unchanged members extracted from Maps.dta (29 BMP/three TGA),
  plus17 byte-identical copies of original Text/Fonts TGA atlases with HD2_ names.
  Font aliases come from stage_menu_text.py, stage_credits_resources.py and
  stage_menu_presentation.py. They are not conversions of .555/.565 textures.
- MenuText is a reconstruction-specific helper directory: copied original font
  metrics and a generated diagnostic labels.bin. The ordinary localized menu
  uses its actual localization cache; symbols still read MenuText/SYMBOLS.tbl.
  recovered/host/menu_text.h::read_menu_text_file uses std::ifstream, so simply
  putting these helper files in a DTA would not make that reader archive-aware.

Evidence: the accepted folder's menu-resources.json records39 archive copies
with original member names/hashes; menu-text-resources.json records the initial
font aliases/metrics. A read-only comparison confirms all17 HD2_ atlases match
original Text/Fonts files byte-for-byte. Static indexing of this installation's
Maps.dta finds31,571 members:168 .tga,7,995 .bmp,20,980 .dx1,1,750 .dx3 and678
.dx2; no .555/.565 members. This local installation has no loose Maps directory.
The role of .555/.565 files observed in the user's other layout has not been
traced; no claim about their exact encoding or cache lifecycle is made here.

Original archive setup006DB6E0 and directory setup006DBE60, called by006DBFC0,
are documented in [archive evidence](startup-archives.md) and
[directory evidence](resource-directories.md); current model/sound lookup and
delegated loaders are documented in [cache integration](menu-caches.md).
Matching the original physical layout would require auditing/removing redundant
extraction and adapting remaining helper/alias loads. That cleanup is not
implemented or desktop-verified by this explanatory follow-up. Keep the accepted
package intact; deleting its loose directories is not established as safe.

Full Single Player/game/save/results remain Phase2; loadouts remain **completely
untested until accessible**; ready-server UI/services remain **untested** until
Phase3. Broader saves.metadata/preview/compatibility remains open. Packaging
does not change those gates. Phase2 research/planning is the next later checkbox.
