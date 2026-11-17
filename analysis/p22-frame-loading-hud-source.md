# Loading HUD: exact explosive timer artwork names — 2026-11-17

Base0e69b80, feature/p22-frame. The user's actual 0f5fd6a8 candidate passed
SupplyBox clips and spatial sound, loaded humanzones/gameinterface/ingame_info,
then returned to Main on `Required HUD explosive timer artwork is missing`.
The exact executable and three logs are archived in
WORK/loading-hud-timer-return. No mission activation or first gameplay frame
was observed. [Receipt](checkpoints/p22-frame-loading-hud-20261117.json).

Original004AF630 references format `timer-explosive%02d` at0083B280 and root
`explosivetimer` at0083B294. Existing indexed C/JSON/raw exports and original PE
literal bytes establish the hyphen; the decompiler's sanitized symbol spelling
was incorrectly copied as an underscore. Source now uses the exact native
format for slots0..9. A genuine missing child still fails, now naming its key.
The failure point retains the prior prefix: actual text allocation, geometry
with default inset10, then inset0 and empty/style48 append before visibility.
Actor borrowers, reverse order, text ownership, reserve and cleanup are unchanged.
No alternate key, substituted artwork or successful fallback is introduced.

Read-only decoding of the actual installed models/gameinterface.4ds found602
frames with zero model diagnostics. Root591 explosivetimer has all ten exact
children timer-explosive00..09 at indexes592..601 (native child order preserved).
To avoid the next identical punctuation failure, a focused check compared75
remaining HUD startup keys against that model:53 static names also match bytes
referenced by the existing original exports; info/timer domains follow native
formats. down_panel has model-presence corroboration only. Original Border,
TMCamera and ~TMCamera spellings differ in case from the model; they are preserved
because the original bytes confirm them. Case-insensitive static model presence
is corroboration, not execution of engine lookup semantics.

Full actual review application and startup-probe i386 O2/Werror compile/link
passed; revised stage-only build and hashes are in the receipt. No new permanent
or mirror test was added for this string correction. Existing lifetime/geometry
verification is unchanged; this check does not claim engine execution or desktop
success. The OWN interactive adapter remains uncommitted pending actual user
acceptance. FRAME/HUM/OWN remain partial; no inventory or complete credit changes.

WORK=analysis/work/p22-frame-closure-20261117. Research index queries preceded
static reads; the receipt fingerprints original exports, literal/model reports,
focused startup lookup report, archived actual run and the recoverable review
patch. Windows/original/DLL/Proton execution remains solely the user's action.
