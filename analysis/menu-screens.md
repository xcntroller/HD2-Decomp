# Registered menu screens and dependencies

This inventories the fourteen definitions registered by `00623DD0` and the
identifier-specific builders called by `00627060`. It is **not** the complete
reachable-menu census: nested pages, in-game variants and application states
remain open. [Machine-readable evidence](menu-screens.json) contains exact IDs,
model strings, call sites, 53 static children and 100 initializer call sites.
These are metadata; no original pseudocode or localized caption text is included.

| Slot | Definition | Model key | Static children | Augmentation | Runtime |
| --- | --- | --- | --- | --- | --- |
| 1 | MAIN MENU | `main_menu` | 17 | — | Bounded functional |
| 2 | CHANGE PROFIL | `change_profile` | 17 | 006211b0 | Not enabled |
| 3 | SINGLEPLAYER | `singleplayer` | 15 | — | Not enabled |
| 4 | CONFIRM | `confirm` | 4 | — | Bounded functional |
| 5 | SINGLE MISSION 2 | `single mission 2` | 0 | 006391e0 | Not enabled |
| 6 | OPTIONS | `options` | 0 | 0061cf60 | Not enabled |
| 7 | SINGLE MISSION 2 | `single mission 2` | 0 | 00614140 | Not enabled |
| 8 | multiplayer menu | `multiplayer menu` | 0 | 00633f80 | Not enabled |
| 9 | create server | `create server` | 0 | 00628490 | Not enabled |
| 10 | PASSWORD | `password` | 0 | 006377c0 | Not enabled |
| 11 | Server Filters | `server filters` | 0 | 00638590 | Not enabled |
| 12 | Game info | `gameinfo` | 0 | 0062f3e0 | Not enabled |
| 13 | MP Connect console | `confirm` | 0 | 00632080 | Not enabled |
| 14 | ADD SERVER | `password` | 0 | 00636ff0 | Not enabled |

Slot zero is a separately constructed runtime confirmation object from
`0066C1D0`; it shares the `confirm` model with slot four, but its identity and
definitions differ. Slot-zero modal replies are not enabled in the host.

Slots five and seven are distinct despite identical names/models: the original
construction loop sends them to `006391E0` and `00614140`, respectively.
The Cinematics button selects slot seven. Keep their identifiers and builders
separate. An empty static child list above means construction is delegated to
the augmentation, not that the screen has no controls.

Credits changes application state to twelve through `00626B20`; it requires
tracing that state’s lifecycle instead of inventing a credits screen. Options
passes through `0061BEA0` and settings-backed messages before selecting slot six.
The options builder references Video, Sound, Controls, Game and Multiplayer
groups; their nested controls/settings semantics are unfinished. Profiles need
editable text, lists, difficulty state and profile persistence. Single Player
needs selector/radio controls and callbacks `0063B590`/`0063B740` before activation.

The reconstructed shared stack supports regular selection (hide current, push,
activate with focus zero), modal push (retain current visibility, push, activate
with focus `10200000`), back/pop and resume. Registry ownership outlives stack
entries. The host binds only slots one/four. Unsupported bindings are rejected
before changing a live screen. Targeted delivery for the two live screens is integrated and desktop-confirmed;
original observer/data lists,
full factory, application state transitions and resource/cache lifetimes remain
unfinished. Native vector storage is not the original segmented-deque ABI.

Next implementation dependency: recover original observer/data callbacks and
nested group construction, then choose a flow whose data/controller boundary
can be completed. Do not enable another button solely because its model loads.
