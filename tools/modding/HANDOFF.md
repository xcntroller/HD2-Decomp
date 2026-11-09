# Modding-tools handoff — 2026-11-09

Latest: first unchecked ISD1 CRC/raw/LZSS milestone is complete and independently
verified for a local checkpoint. No push/merge without renewed approval. Earlier
branch delegation permits verified local commits for merge review. Main/client/
server source, shared handoffs, plan and ledger remain untouched; no credit tally.

Launch `make -C tools/modding gui` → Mission explorer → Open installation. The
original explicit language/loose/archive-first profile is preserved. Patch wins
where selected; no fallback on invalid/unsupported data. Content and Archive
columns distinguish decoded verified bytes, partial format parsing, unavailable
codecs and invalid inputs; file/actor details retain provenance/check results.

`isd1.py` reconstructs separate descriptor/tag/payload storage, all native
checksum stages and raw/LZSS. Actual Patch ALPS1 Scene2(6727085 bytes) and actors
(41052) match existing captures byte-for-byte and by SHA256. Details, addresses,
limits, checksums and audit hashes: [ISD1](ISD1.md), [evidence](isd1-evidence.json).
Other contracts: [formats](FORMATS.md), [resolution](RESOLUTION.md), [usage](README.md).

Consolidated verification:5619 LangEnglish/Patch metadata records passed; full
Patch sweep decoded1053 members/259360081 bytes, six sound members remained
unsupported after stored-checksum verification.32 captured resources match
byte-for-byte and five absences agree. Tutorial275 files/2139 entities/5345 edges;
actual selected ALPS1246/4452/8586. No invalid/blocked selected inputs in those
sessions; format coverage remains partial. Synthetic independent checks cover
all CRC layers, raw/LZSS, key resets, clear/encrypted/empty/multiblock payloads,
truncation, offsets, malformed streams, unavailable codecs and precedence. Native
GUI checked real patched records plus invalid/unavailable states; rendered window
inspected. Python compilation and small existing inspector build pass.

No original DLL/game/Proton/copied original code executed; static objdump only,
no Ghidra or heavy game builds. No new native code, hence no new sanitizer claim.
Originals read-only; no permanent tests/assets/raw exports committed. Temporary
/tmp/hd2-isd1-audit fixtures, source slices, reports, screenshots and disassembly
removed after verification. New user desktop acceptance is not claimed.

Next unchecked item: structured script diagnostics/spans and explicit events.
It was not started during this archive milestone. ISD1 sound tags>=8, expansion
profiles, full actor/asset formats, engine cache/joint resolution and include
semantics remain excluded. Do not confuse verified container bytes with complete
mission/runtime compatibility; no 3D/editor/binary serialization work was added.
