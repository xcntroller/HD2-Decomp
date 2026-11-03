# Credits entrance at larger resolutions — 2026-11-02

The reported initial wait is explained by the **original width-dependent scroll
formula**, which the reconstruction preserves. No timing change is made.
This is a static/native comparison; the original game/DLLs were not executed.

## Original evidence

Original client SHA256:
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
[Fingerprint-checked instruction/constant map](credits-resolution-check-map.json)
supplements the already confirmed [Credits contract](credits-flow.md).

- `006887C0`, instructions `00688936…00688972`: read width and height, form
  `width × float(1/1024)`, initialize speed to that factor×50×float(.001),
  and offset to `−factor × (height+100)`. This is not a fixed100-pixel gap.
- `00689340`, instructions `00689926…0068993A`: a valid SPEED record replaces
  speed with `width × float(1/1024) × parsed_integer × float(.001)`.
  The installed English script starts with `<SPEED>70`.
- `00688C10`, instructions `00688CA2…00688CAC`: elapsed milliseconds×speed plus
  the previous float offset; store the new float and convert the accumulator
  through the signed-word path. `0064B950` subtracts that scroll from row baselines.

Consequently, the first glyph's top initially sits at approximately
`width/1024 × (height+100)`. Its ideal time to reach the viewport is:

`max(0, (width/1024 × (height+100) − height) / (width/1024 × .07))` milliseconds.

The bottom black fade then makes the entering text gradually visible; the table
below measures the first geometric viewport intersection, not perceived legibility.

| Resolution | Initial scroll offset | First glyph/viewport intersection |
| --- | ---: | ---: |
| 640×480 | −362.5 | Immediate |
| 800×600 | −546.875 | Immediate |
| 1024×768 | −868 | 1.43s |
| 1280×720 | −1025 | 3.49s |
| 1280×1024 | −1405 | 4.36s |
| 1920×1080 | −2212.5 | 8.63s |
| 2560×1440 | −3850 | 13.78s |

These are native measurements at10ms updates using the actual English script,
1,054 parsed rows and installed ANSI/CE Set1/2/3 metrics. The first record is
`DEVELOPMENT DIRECTOR`, style2. They are predictions of the original formula at
those dimensions, not a claim the original game supported widescreen modes.

## Verification and precision boundary

A temporary ASan/UBSan fixture composes the production localization/Credits parser,
CreditsScroll, CreditsLayout and font decoder. Independent hardware x87 arithmetic
replays the original initialization, speed and per-frame float calculation; the
float state and rate match. All seven viewport-entry measurements agree with the
formula within one10ms tick plus integer glyph conversion.

The audit also separates a pre-existing native precision difference: original
FSTS leaves the extended accumulator available to `__ftol`, while `CreditsScroll`
converts its stored float when `word()` is queried. Some sampled pre-entry frames
therefore differ by one pixel after float rounding. This is not the seconds-long
delay. No claim of exact original x87-to-word equivalence or full high-resolution
Credits completion is made. Signed16 row/scroll wrap remains the original narrow
coordinate policy. The [host audit](host-boundary-audit.md) records these limits.

The final fixture strictly compiles/links for Windows i386 without execution.
Existing native regressions,158 Python tests, three current application targets,
original evidence and ledger checks pass. Temporary sources/binaries removed;
no permanent tests or function credit. Logs/hashes:
`analysis/work/host-boundary-audit-20261102/{credits-sanitizers,evidence,windows-audit}.log`
and `verification.json`. Accepted executable remains unchanged:
`e11fb644abd669a04deae5b56bb095763ceec59401ad391ca76718cbcb9384a8`.
