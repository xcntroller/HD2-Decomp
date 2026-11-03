# HD2 reconstructed front end — portable Windows build

Extract the **whole ZIP** into a writable folder, then double-click
**HD2FrontEnd.exe** (or Launch.cmd). The extracted folder is self-contained for
this menu checkpoint. It needs no Python, compiler, source checkout, original
game executable or absolute installation path at runtime.

On first launch, select **Create Profile** in Main, create/select a profile,
then enter the available menus. This package deliberately starts with empty
PlayersProfiles and no game.cfg. It includes no installed profiles, keys,
configuration, logs, Wine prefix or previously staged user state. Each extracted
copy writes its own profiles/settings, game.cfg, window.cfg and host-runtime.log.
Keep the folder writable; avoid Program Files. The original engine uses legacy
byte filesystem paths, so use a path representable in the Windows system code page.

## Options

Default: interactive 800×600 window, original registry-selected language (English
fallback). Run from Command Prompt in the extracted folder, for example:

```bat
HD2FrontEnd.exe --width 1920 --height 1080 --language spanish
HD2FrontEnd.exe --fullscreen --width 1024 --height 768
HD2FrontEnd.exe --windowed --width 800 --height 600
Launch.cmd --width 1280 --height 720 --language Czech
HD2FrontEnd.exe --frames 10
HD2FrontEnd.exe --help
```

Supported installed translations: Czech, english, EnglishUS, french, german,
italian and spanish. An explicit language affects this launch and does not write
the registry. Japanese translation was unfinished; wide/GDI fonts are unavailable.
`--fullscreen` uses the original engine's exclusive fullscreen device at the
requested resolution,32-bit color and default refresh rate. Use a display mode
supported by your graphics driver; an unsupported mode fails with a diagnostic
in host-runtime.log. `--windowed` selects the default windowed mode. These are
per-launch options; restart to change mode. Fullscreen pauses drawing while
inactive or the device is lost and delegates recovery to the original engine.
Fullscreen also enables the original Options gamma gate. Live resizing and an
in-menu display-mode selector are not included. Credits retains the original
width-scaled entrance offset: around8.6 seconds before text enters at1920×1080.
Double-click **LaunchFullscreen.cmd** to start fullscreen at the default800×600.

## Required files and platform services

Keep all extracted files together. package-manifest.json records every packaged
file's size/SHA256, the client identity, copied DLLs and their static imports.
These original DLLs are included as local delegated dependencies:

- LS3DF.dll: original graphics/3D/input/sound/movie services.
- rw_data.dll: archive/file/codec services.
- IJoy.dll: original joystick interface dependency.
- binkw32.dll: movie decoding.
- vorbisfile.dll, vorbis.dll and ogg.dll: audio decoding.

The folder also includes the installed base-game archive set and required loose
Text/GameData/Tables and available movies/previews. Models, sounds and menu
images use the original DTA archives; fonts use original Text/Fonts paths.
No development Models/Sounds/MenuText directories or Maps font aliases/extracts
are supplied. Keep the original Text directory as well as the archives.
It excludes the original game/server executable and expansion-only archives.
Removing files can prevent startup or menu entry; an EXE alone is insufficient.

This is a32-bit Windows PE client using the Universal CRT/API-set runtime and
legacy Direct3D8, DirectInput/DirectSound, WinMM, COM and Win32 services. Those
platform components are not replaced by this package. LAN discovery also uses
DirectPlay8; missing/unavailable networking is reported through the existing UI.
Native Windows compatibility is not newly desktop-verified by packaging.

For an existing Proton setup on Linux, use a separate compatibility-data folder
and the extracted EXE. Example, from the extracted folder:

```sh
mkdir -p "$HOME/.local/share/hd2-rebuilt-prefix"
STEAM_COMPAT_CLIENT_INSTALL_PATH="$HOME/.local/share/Steam" \
STEAM_COMPAT_DATA_PATH="$HOME/.local/share/hd2-rebuilt-prefix" \
STEAM_COMPAT_INSTALL_PATH="$PWD" \
"$HOME/.local/share/Steam/steamapps/common/Proton - Experimental/proton" \
run "$PWD/HD2FrontEnd.exe" --width 1280 --height 720 --language english
```

Adjust the Steam/Proton paths for your installation. The executable resolves its
own directory and loads the included LS3DF.dll/IJoy.dll there. Keep the same prefix
for subsequent runs. This command is a user launch instruction; packaging does
not execute Proton, the original game or its DLLs.

## Included behavior and current limits

Opening movies/subtitles → Main, Profiles, front-end Options/bindings/settings,
Credits, available Cinematics with retained return, reachable multiplayer UI and
the observed team-selection checkpoint are implemented. Input, localization,
resolution layout, audio and bounded cleanup have prior user acceptance.

Full single-player completion, game/level loading, gameplay-save receivers and
in-game/result screens remain Phase2. **Loadout/equipment screens are completely
untested until accessible.** Ready-server information, Join/password/consoles,
Host/Connect and session services remain untested until Phase3. Resume stays
unavailable; Main's stored profile caption does not establish save compatibility.
Broader save metadata/preview recovery is still open. Full original setup,
protection VM, object ABI and engine replacement are outside this package.

The revised package's original-path resource loading and fullscreen support are
[user-accepted2026-11-02](../analysis/resource-fullscreen.md#desktop-acceptance--2026-11-02).
This records overall acceptance, without claiming a separate result for every
suggested desktop case. The bundled README predates this acceptance record.
If startup fails, keep host-runtime.log from the extracted folder; it records
the current run.

## Rebuilding the package from the repository

Build application-launch with the documented i386 toolchain, then run:

```sh
python3 -B tools/package_application.py
```

Default outputs: dist/HD2-Phase1/ and dist/HD2-Phase1.zip. Existing outputs are
refused so a rebuilt package cannot overwrite profiles/settings in an older one.
Use --output with a new folder name, or --no-zip for the folder alone. The builder
uses fresh private staging and never imports writable user state.
