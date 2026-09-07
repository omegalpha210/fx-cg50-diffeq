# Initial-condition behavior audit

Primary evidence is `docs/reference/diff_eq_manual.pdf`. Page numbers below are PDF pages, followed by the printed section where useful. **ORIGINAL** means the manual states or demonstrates it; **INFERRED** means the implementation follows the mathematics and the manual's common workflow without claiming an undocumented calculator detail.

## Common decision

Every supported mode exposes F4 ADD/F5 DROP in the independent Initial Conditions screen. An IC block always represents one complete IVP: one `x0` plus every state value required by that mode. The equation editor contains no IC rows and LEFT/RIGHT starts equation-text editing at the beginning/end, then moves its cursor. IC set numbers appear only on block headings, so system state names `y1`…`y9` cannot be confused with set numbers.

The original manual repeatedly separates “specify the initial value(s)” from equation entry and Output selection (PDF pages 4, 6, 8, 10, 12, 14 and 18). Its separable example explicitly enters `y0={0,1}` and says to enter a list of initial conditions to graph a family of solutions (PDF page 5, printed 3-2-2). This proves multiple families for separable equations. For the other modes the manual establishes complete IC vectors but does not explicitly document ADD/DROP keys or list broadcasting, so those multiple-family decisions are marked **INFERRED**.

| Mode | A. Original multiple IC evidence | B. Mathematical meaning | C. User value | D. UI decision |
|---|---|---|---|---|
| First-order Separable | **ORIGINAL:** `y0={0,1}` and “family of solutions” list (PDF p.5) | Each `(x0,y0)` defines an IVP, including equilibria and neighboring solution curves | High: the reference example itself compares a family | Show ADD/DROP |
| First-order Linear | **INFERRED:** the manual requires `x0,y0` (PDF p.6); no explicit multi-value example | Each pair defines a distinct solution of the same linear ODE | High for comparing transients and parameter-independent families | Show ADD/DROP |
| Bernoulli | **INFERRED:** `x0,y0` is required (PDF p.8); no explicit multi-value example | Each pair defines a distinct solution, including special zero/equilibrium cases | High for comparing nonlinear solution branches | Show ADD/DROP |
| General First-order | **INFERRED:** same first-order procedure and `x0,y0` (PDF p.10) | Each pair defines an independent numerical IVP | High; this is the broadest first-order mode | Show ADD/DROP |
| Linear Second-order | **INFERRED:** one complete set is `x0,y0,y'0` (PDF p.12; OCR in some extracts prints the derivative mark incorrectly) | Two state values at one x define one solution; another complete triple defines another solution | High for comparing displacement/velocity starts | Show ADD/DROP; never treat `y0` and `y'0` as separate solutions |
| Other Second-order | The app represents this as **N-th, order 2**; the manual's N-th workflow requires the complete derivative vector (PDF p.14) | `(x0,y0,y'0)` defines one nonlinear second-order IVP | High when comparing nonlinear trajectories | Show ADD/DROP through N-th order 2 |
| N-th order | **INFERRED:** the manual requires `x0,y0,y'0,…,y^(n-1)0` (PDF p.14) and preserves initial values during →SYS conversion (PDF pp.15–16) | One full N-state vector defines one solution | High for comparing high-order responses with identical equations | Show ADD/DROP; duplicate and delete whole vectors only |
| Systems | **INFERRED:** the manual requires `x0,(y1)0,(y2)0,…` (PDF p.18) | One x and N state values define one trajectory; multiple vectors define multiple trajectories/phase curves | High for trajectory and phase comparisons | Show ADD/DROP; state labels stay `y1`…`yN` inside numbered IC blocks |

## Bounds and exceptional state

The app supports at most nine IC blocks. ADD copies the selected block to preserve a complete vector and appends it; DROP removes the selected block and shifts its graph/list masks and colors with it; ADD assigns new default curve colors. Zero IC blocks remain valid only in first-order modes while SF is nonzero, which supports a slope-field-only graph. Higher-order and system calculations require at least one complete block; their last-IC DROP is blocked, as is first-order DROP when SF=0. These are bounded implementation decisions rather than claims about undocumented Algebra FX list semantics.

All screen layout, key repeat, ADD/DROP behavior and physical modifier behavior remain **HARDWARE RETEST REQUIRED**.
