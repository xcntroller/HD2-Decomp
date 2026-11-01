# Multiplayer function audit — 2026-11-01

This continuation adds **33 partial client entries** present in the frozen inventory and extends existing006C20F0 without double counting. No complete-function, engine-delegate, live-service or desktop-delivery claim. The frozen denominator remains18,783.

| Original entry | Implemented subset | Source |
| --- | --- | --- |
| `0061A860` | Decode multiplayer host/profile options, nested versioned settings, filters and sort fields transactionally with bounded chunks. | [multiplayer_settings.h](../recovered/frontend/multiplayer_settings.h) |
| `0061AC10` | Encode MPSetingsOrig.sav host/profile settings with the original nested tags and field order. | [multiplayer_settings.h](../recovered/frontend/multiplayer_settings.h) |
| `00616C50` | Initialize original multiplayer filter defaults independently of browser submission success. | [multiplayer_settings.h](../recovered/frontend/multiplayer_settings.h) |
| `006172F0` | Serialize the selected-map names within the original mode/preset rotation records. | [multiplayer_settings.h](../recovered/frontend/multiplayer_settings.h) |
| `00617610` | Read saved rotation names into bounded native mode/preset vectors without borrowed archive storage. | [multiplayer_settings.h](../recovered/frontend/multiplayer_settings.h) |
| `00618A00` | Build the base multiplayer map catalogue and original rule defaults, skipping the cooperative subtree. | [multiplayer_maps.h](../recovered/frontend/multiplayer_maps.h) |
| `006189C0` | Apply the original base-game directory allowlist to available multiplayer map records. | [multiplayer_maps.h](../recovered/frontend/multiplayer_maps.h) |
| `006197B0` | Move a selected rotation entry by stable removal and insertion, preserving intervening order. | [multiplayer_maps.h](../recovered/frontend/multiplayer_maps.h) |
| `00619760` | Remove a selected rotation entry with native bounds checks. | [multiplayer_maps.h](../recovered/frontend/multiplayer_maps.h) |
| `00619960` | Append an owned map-name copy to the selected rotation with the UI row bound. | [multiplayer_maps.h](../recovered/frontend/multiplayer_maps.h) |
| `0064ED00` | Compose the numeric edit and optional caption planes with independent owned controls and forwarded edit state. | [multiplayer_controls.h](../recovered/host/multiplayer_controls.h) |
| `006663F0` | Construct and populate the eleven browser row products with linked scrolling and preserved selected server identity. | [multiplayer_controls.h](../recovered/host/multiplayer_controls.h) |
| `00665A50` | Bind original server row text and password/dedicated/voice icon pairs with common pick targets. | [multiplayer_controls.h](../recovered/host/multiplayer_controls.h) |
| `006610D0` | Bind a labeled toggle to its authored inner text plane with the original shared control identity. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `0062F960` | Bind reverse-order named player rows, latency/score columns, team scores, server metadata and transmitted rule values. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `0062F3E0` | Construct information-screen controls and bind the original dynamically authored player/header/rule column styles. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `006295C0` | Build the fifty selected-map row products and their linked scrolling for the active rotation. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `00629A40` | Build available map list products for the base multiplayer modes from the confirmed directory catalogue. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `00629EE0` | Build the base Create rule settings list with original row identities, labels, numeric limits and toggles. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `0062CF60` | Capture the configured server/map rotation, validate selection and create an owned launch request through an explicit unavailable session boundary. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `00634A60` | Populate the favorites browser from stored records and retain the ordinary/favorites persistence distinction. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `0062E7A0` | Clone the normal map-row visual and independent caption for a cursor-following drag while preserving the source row. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `006318D0` | Clear stale information and request a selected-server information query with owned callback identity and failure handling. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `0062E920` | Move the map drag visual at original depth and reorder selected rows by stable insertion without resetting scroll. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `0062EA90` | Classify the original available/selected drop rectangles and add, remove, reorder or cancel the dragged map. | [multiplayer_menu.h](../recovered/host/multiplayer_menu.h) |
| `0068AED0` | Model standalone connection timer phases, host/client submission boundaries, rejection labels, delayed regular password and game-entry request with actual unavailability distinct from success. | [multiplayer_connection.h](../recovered/frontend/multiplayer_connection.h) |
| `0068A990` | Construct the isolated regular connection console and Exit Game action in a distinct type8 scene and registry. | [multiplayer_connection_menu.h](../recovered/host/multiplayer_connection_menu.h) |
| `0068AD20` | Construct standalone regular password slot2 with edit Enter/Escape, nonempty Yes submission, No cancellation and hidden unused edits. | [multiplayer_connection_menu.h](../recovered/host/multiplayer_connection_menu.h) |
| `00725EC0` | Decode the signed player count and bounded owned player vector; negative counts consume no player records. | [multiplayer_discovery.h](../recovered/frontend/multiplayer_discovery.h) |
| `00725E60` | Decode each player statistics prefix, team word and bounded name without retaining packet pointers. | [multiplayer_discovery.h](../recovered/frontend/multiplayer_discovery.h) |
| `00749AE0` | Decode original 11-bit signed score and the 10/10/10/5-bit player statistic fields, preserving the positive1024 edge. | [multiplayer_discovery.h](../recovered/frontend/multiplayer_discovery.h) |
| `00725C40` | Decode transmitted multiplayer rules with signed16 fields and explicit absence of untransmitted field12. | [multiplayer_discovery.h](../recovered/frontend/multiplayer_discovery.h) |
| `00725FB0` | Decode information players, rules and two10-bit team score tails as an owned payload. | [multiplayer_discovery.h](../recovered/frontend/multiplayer_discovery.h) |

Not newly credited: absent entries0064EFE0,0065B2C0,00634880,00636570,00635600,006361B0,00638840,00637970,0062D3C0,00619B60,0068A460,0068A710 and0068B9F0. No credit for automatic C++ destruction standing in for00617B80. Original00722790 Host,00723D20 Connect, auxiliary UDP/GameSpy and00698510 game owner are unimplemented; delegated OS/engine calls do not add credit.

Verification and scope: [pipeline](multiplayer-pipeline.md). Static exports include original function addresses and retain the client fingerprint; temporary audit code is removed after execution.


## Accepted follow-up audit — 2026-11-01

The reviewed follow-ups add five more frozen client bodies; the original33-entry
batch above is unchanged. These five are partial, never complete-function credit:

| Entry | Implemented subset | Evidence/source |
| --- | --- | --- |
|0062D880|Base-mode rule enable masks and toggle dependencies, retaining disabled values.|`multiplayer-rule-caret-20261101/rule-refresh.asm`; `frontend/multiplayer_settings.h`; `host/multiplayer_menu.h`|
|006363E0|Filtered/total/player footer format and localized online-player suffix.|`multiplayer-discovery-20261101/006363e0.c`; `host/multiplayer_menu.h`|
|00637440|Read and validate manual address/port, unknown display initialization, owned browser merge/persistence and close.|`multiplayer-wire-20261101/00637440.c`; `host/multiplayer_menu.h`|
|00654C60|Bind and align the four sort-arrow planes with original normaldown position and direction/hover state.|`multiplayer-review-widgets-20261101/00654c60.c`; `host/multiplayer_controls.h`|
|00648C80|Separate underscore placement, advance/blinking and original0.1f baseline lift with truncation.|`options-caret-20261031/00648c80.c`; `host/menu_text_geometry.h`|

Evidence directories are under ignored `analysis/work/`; sources are under
`recovered/`. Original native object/vtable/renderer ABI remains unreconstructed;
engine/OS/CRT services stay delegated. The regular owner, legacy style modulo,
protocol string extraction and library calls referenced by source comments do
not acquire credit merely from those references. Only these five demonstrable
bodies were added. Existing credited control/text/drag bodies are extended
without double counting. Total564 partial (559 client/five engine), zero complete
of18,783. Ready-server UI remains untested despite reachable frontend acceptance.
