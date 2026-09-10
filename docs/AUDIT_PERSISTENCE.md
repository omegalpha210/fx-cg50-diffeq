Current beta.8 changes are authoritative in [VISIBILITY_PROMPT_AUDIT](VISIBILITY_PROMPT_AUDIT.md).
R1/UIR1 are resolved by owner decision, implemented and validated. Drawing now
keeps Graph with a bottom bar; Output has per-IC ON/OFF and color; SAVE is v11;
numeric EXIT cancels immediately to G-Solve page2. The beta.7 report below is
historical evidence, including its former review choices and original controls.

---

# Persistence and resource audit — beta.7

## Persistent preferences and consumers

The current version10 record streams both complete Document values, with checked
header/length/checksum and two alternating generations. Adding per-IC color UI
does not add a field or change a byte offset. The current/recall load buffer is an
existing union with CompiledModel; UI calculation recompiles after storage use.

| Persistent state | Owner/consumer | Audit evidence |
|---|---|---|
| kind,dimension,all equation text,power | Equation/conversion/model_compile | All4 scalar subtypes,Second,N-th1..9,SYS1..9 cold roundtrip |
| all10 IC slots and all9 state values | IC/model trajectories | Active and inactive slots compared byte-for-byte; independent current/recall |
| h/h0,method,RelTol,AbsTol,Maxsteps,Step | RK4/RK45,Parameters/Table | Nondefault values retained with both methods |
| SF | Parameters/scalar renderer |0/1/12/50 roundtrip; old51/100 become50 in everyv3..10 reader |
| AUTO/MAN and solver bounds | Parameters/window synchronization | MAN and exact stored bounds retained; independent of view geometry |
| TIME window,both scales,grid/labels,projection/axes | V-WIN/Graph/TRACE | Nondefault complete ViewWindow retained |
| PHASE window,field/nullclines/ready | SYS2 view/analysis | Distinct nondefault phase state retained |
| dependent visibility and color[10][9] | Output/Graph/TRACE/G-Solve;visibility alsoTable/CSV | Complete matrix including inactive preferences retained |
| field style/color | Graph Settings/slope field | Nondefault segment/style and palette retained |
| Event enabled/expression/direction/action | Event/compiler/solver/Graph | Full EventConfig retained; runtime markers excluded |

`full_persistence_matrix` performs46 mode/dimension/method cases. Every case saves
different current and recall documents, initializes a cold App (without automatic
RCL), loads explicitly, compares every Document byte and recompiles the current
model. Runtime diagnostics remain invalid after cold start/load. Two truncated
slots fail without replacing either live document. Existing storage tests retain
interrupted-write fallback, malformed/current/future headers, checksums, native
short-I/O/close cleanup, generation and bounded CSV/STAT cancellation coverage.

Frozen v3..v9 migration fixtures are independent layouts with offset assertions.
v3 constant/mask adaptation, v6 nine-IC storage, v7 expandedIC, v8 separatePhase,
v9 adaptive solver and v10 Event compatibility remain. SF clamping does not reject
otherwise valid old sessions. v4..v6 now initialize their absent tenth color row
with the established palette; earlier nine rows are unchanged. No format bump.

No additional persistent preference omission was reproduced in this matrix.
TRACE cursor, temporary G-Solve results, compiled expressions and diagnostics
remain runtime state by design; incomplete IC drafts are not saved numeric values.

## Fixed capacity and ownership

| Resource | Bound and ownership |
|---|---|
| ODE state |9 variables,10 ICs; model validation checks active and inactive finite slots |
| Expression/editor |192 bytes per text including terminator; bounded token/program stacks |
| Per-IC colors |Existing90 bytes, each index0..5; exact selected family setter validates bounds |
| TRACE/display |258 points shared across families/branches; connectivity bits prevent joining recorded gaps |
| Overlay/staging union |Existing24096 bytes; masks/footer,BOX or staging are mutually exclusive |
| Busy pixels |Unused staging tail2852 target bytes;396x3 strip uses2376 bytes; no new framebuffer |
| G-Solve/Event/Phase results |32/32/16 entries with truncation/limit behavior covered by consumer tests |
| Table |7 rows,at most10 dependent columns; same-page cache is the existing local TablePage |
| CSV/STAT |Existing2048-byte formatting buffer; bounded998 data rows for STAT |
| Input queue |8 events with control/cancel priority; held EXIT filtered at native input boundary |

Only IC draft editing directly allocates application heap: changed fields retain
at most10 strings of192 bytes (1920-byte payload plus allocator overhead). Allocation
failure preserves the current editor; replacement frees the old field only after
successful allocation. NEXT success,INIT and document replacement free drafts;
back navigation intentionally retains them. No new heap allocation was introduced.
Existing iterative screen dispatch avoids accumulating C frames under navigation.

Busy painting saves, clips, synchronously uploads and restores at most3 full-width
rows using installed gint dwindow_set and r61524_display_rect. The initial delayed
screen transfers75 strips; later frames transfer7 header strips. Native call tests
verify those bounds. Host LCD shadow is test-only and absent from firmware. The
newly linked16-byte dwindow is initialized data; application BSS has no new buffer.

Memory totals and actual maximum single-frame measurements are recorded in
MEMORY_AUDIT. They are not cumulative call-stack or free-heap measurements.

## Local reference review

Original DIFF EQ manual PDF pages4,6,8,12,14,18 (printed3-2-1,3-2-3,3-2-5,
3-3-1,3-4-1,3-5-1) were rendered and read. They define the scalar subtypes,
linear second-order form, n-state derivative mapping and system IC requirements.
Current extensions RK45/Event/Phase and the owner's SF50/UI controls take priority
over historical controls. No manual artwork, PDF or private path is published.

## Limitations and hardware

Storage failure tests simulate filesystem behavior; they cannot establish Fugue
world-switch timing, an OS native-handle lock, power-loss guarantees beyond the
two-slot protocol or physical storage wear. Host scratch/LCD tests cannot measure
native transfer latency, RTC timing, key repeat, allocator fragmentation or stack
high-water. These remain HARDWARE TEST REQUIRED.
