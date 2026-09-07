# Input workflow and lifecycle

This hardware-feedback update preserves rollback point `6addf4d`, the RK4/parser,
Fugue world switch, iterative dispatcher and explicit SAVE/RCL policy.

| Screen | F1 | F2 | F3 | F4 | F5 | F6 | SELECT EXIT |
|---|---|---|---|---|---|---|---|
| Equation | VAR | FUNC | V-WIN | | | NEXT | Type/order/Main |
| IC | PREV | | V-WIN | ADD | DROP | NEXT | Equation |
| Parameters | PREV | INIT | V-WIN | OUTPUT | SET | GRAPH | IC |
| Output | GRAPH | LIST | COLOR | INIT | | DONE | Caller |
| Graph Settings | ON | OFF | | INIT | | DONE | Caller |
| Main | 1st | 2nd | N-th | SYS | RCL | SAVE | Stay |
| Zoom | IN | OUT | AUTO | ORIG | | | Graph bar |
| TRACE | x= | | | | | BACK | Graph bar |
| X/Y-CAL scalar | | | | | | RUN | Cancel |

## Fields and one-event policy

The common `ui_field_complete` translates EXE once. EDIT EXE validates/commits,
ends editing, then selects the next field; the last edited field stays selected.
SELECT EXE advances, or on the last field invokes the screen's F6 NEXT/GRAPH/DONE/
OPEN. EDIT EXIT validates/commits and stays on the same field; SELECT EXIT returns
to its parent. Invalid drafts/cursors stay. LEFT/RIGHT enter editing at either
end; direct physical input replaces. A single EXE never enters the next editor.
Available stage navigation softkeys validate before acting.

OUTPUT EXE follows variable rows. A modified last row first confirms/stays and
then completes; an unmodified last row completes immediately. G/L and accepted
color changes are applied immediately and survive arrow navigation. EXE neither
toggles G nor opens a palette. RIGHT activates COLOR; F3 opens the current state's
chooser. Palette arrows/EXE/EXIT retain select/accept/cancel behavior. Destructive
F5 NO/F6 YES confirmations retain their explicit consent keys.

X-CAL/Y-CAL is the explicit exception: starts with a blank draft, blinking cursor,
and only F6 RUN. EXE and F6 share validation and immediate execution. Empty/invalid
input stays with an inline error, without reusing an old value. Valid EDIT EXIT
commits/stays and the next EXIT cancels. Physical scalar keys remain; no function
softkeys are shown. TRACE x= retains its separate exact-query policy and cancellation
routes pending EXIT/MENU through TRACE's priority handler.

Equation FUNC/VAR changes only the softkey bar, preserving body/draft/cursor.
FUNC pages contain ABS/SINH/COSH/TANH/ASINH and ACOSH/ATANH. Physical SHIFT trig
remains; parser support is unchanged. VAR uses actual mode scope: higher derivatives
y1..y(N-1), system y1..yN, existing general/separable-g aliases; x-only fields have
no state tokens. F6 pages/back, EXIT closes, menu EXE is a no-op. Selecting a token
inserts at the current cursor and restores the normal bar. Constants retain OPTN.

Main digits 1–4 match F1–F4; 5/6 trigger no SAVE/RCL. The short description
`Recall / load session` reflects its RAM and saved-session choices.

## Settings and persistence

PREV/Magenta, NEXT/Cyan, V-WIN/Orange, SET/Bright Green and actual F6 GRAPH/Red
remain. OUTPUT F1 GRAPH is blue; COLOR alone has a white background and five
red/orange/bright-green/cyan/magenta letters in the existing renderer.

`model_output_defaults` restores all IC G/L masks and delegates colors to the
existing six-color default helper (Magenta/Cyan/Green/Red/Blue/Black). INIT on
V-WIN, Parameters, OUTPUT and Graph Settings selects the first item without editing.
Graph Settings INIT resets only Grid/Axis Label. IC ADD/DROP and last-IC protection,
per-state colors, phase color, v4/v3 persistence and size-change consent are preserved.

V-WIN syncs Solver X only in AUTO; manual Solver edits do not alter V-WIN. ORIG
uses V-WIN defaults, preserves equation/IC/h/manual range, and syncs AUTO. IN/OUT/
AUTO/ORIG keep ZOOM active until EXIT. Graph pan still extends AUTO integration.
Menu-only open/page/exit has no integration or file I/O; actual zoom/search can
integrate. Scalar cursor blink redraws only the footer, without another solve.

The dispatcher owns caller ancestry and retains selection on auxiliary return.
No Document copy or growing screen call stack is added. Cold startup uses defaults;
only explicit SAVE writes, and RCL distinguishes RAM recall from loading slots.
Native storage remains in synchronous `gint_world_switch` workers with closed
handles before UI or numerical work resumes. Host session ABI is not SH-portable.

## Validity and graph resources

Graph/TRACE/G-Solve consume trusted points and NULL continuity delimiters. Magnitude,
nonfinite and domain failures retain usable prefixes; user cancellation is separate.
No gap-spanning line/bracket or invented RK4 restart is allowed. Each IC is a separate
IVP; independently trusted post-gap providers are supported by the consumers and
exercised in a synthetic fixture. See [validity audit](NUMERICAL_VALIDITY_AUDIT.md).

TRACE keeps its existing 44,756-byte cache/mask/footer. Movement/blink uses retained
samples, with fixed 400 ms/125 ms repeat timing and EXIT/MENU priority. Black↔Blue
is shared with G-Solve; other TRACE XOR colors remain. EXIT reverses overlays.
The installed gint keydev/getkey/timer rules audited in [TRACE_EVENT_AUDIT.md](TRACE_EVENT_AUDIT.md)
remain in effect. Physical keyscan, LCD, MENU, Fugue and runtime memory margins are
**HARDWARE TEST REQUIRED**, not established by host tests.

References: DIFF EQ PDF p.8 Bernoulli, p.19 Graph/List; fx-CG50 PDF p.9 (printed 1-1)
physical ALPHA X/Y, separate NEG, SHIFT sqrt/pi. Existing [reference audit](reference/README.md)
is retained; user PDFs are not staged as part of this update.
