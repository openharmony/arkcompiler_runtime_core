# AGENTS.md — `runtime/mem/gc`

Garbage collectors for the VM. Read `../../AGENTS.md` (runtime) first for the runtime↔GC boundary;
this file covers only what is specific to editing collector code.

## Collectors (`GCType` in `gc_types.h`)

| Type | Dir | Use |
|---|---|---|
| `G1_GC` | `g1/` | **Production** region-based generational collector (concurrent mark, evacuation, TLABs) |
| `STW_GC` | `stw-gc/` | Simple stop-the-world mark-sweep; reference baseline |
| `EPSILON_GC` | `epsilon/` | No-op collector (allocate only) for debug/perf isolation |
| `EPSILON_G1_GC` | `epsilon-g1/` | G1-shaped epsilon variant that skips real collection |
| `CMC_GC` | `cmc/` | Concurrent-mark-copy collector (`ark_use_common_runtime` option) |

Selected at runtime via `--gc-type` (see `runtime/options.yaml`). Don't hardcode a collector — `CreateGC(gcType, ...)`.

## Architecture

- Class chain: `GC` (`gc.h`, abstract) → `GCLang<LanguageConfig>` (`lang/`, adds `RootManager`, depends on `LanguageConfig`) →
  `GenerationalGC<LanguageConfig>` (`generational-gc-base.h`) → `G1GC`. `StwGC`/`EpsilonGC` derive from `GCLang`.
- Cross-cutting pieces: `gc_barrier_set.*` (pre/post write barriers), `card_table*` + `heap-space-misc/crossing_map*`
  (remembered sets), `gc_root*` (root scanning), `gc_phase.h` (phase state machine), `gc_trigger.*` (when to collect),
  `reference-processor/` (weak/soft/final refs), `workers/` (parallel GC task pool), `bitmap.*` (mark/live bitmaps).

## Invariants — get these wrong and you corrupt the heap

- **Barriers are mandatory on reference writes.** Any store of an object reference in mutator must go through the active
  `GCBarrierSet` (G1 uses SATB pre-barrier + card post-barrier). Don't add a raw reference-store path that bypasses it.
- **GC visibility / safepoints.** Object pointers held across a safepoint, allocation, or runtime call can move or die;
  keep them reachable via roots/handles, not raw `ObjectHeader*` locals. Moving collectors (G1/CMC) relocate objects.
- **Phase order matters.** Respect the `GCPhase` state machine; marking, remark, and sweep/evacuate have ordering and
  STW-vs-concurrent constraints. Read `../../../docs/gc-knowledge.md` before changing phase flow.
- **`workers/` code runs concurrently.** Guard shared state; marking stacks (`gc_adaptive_*stack*`) are the intended
  hand-off structure.
- Keep `.h`/`-inl.h` split as-is: hot inline paths live in `-inl.h`.

## Boundaries — ask before

Changing barrier contracts, `GCType` set or default, phase semantics, trigger policy/heuristics, the GC↔compiler
interface (barriers are emitted by codegen and irtoc fastpaths — a change here ripples into `compiler/` and `irtoc/`),
or root-scanning contracts. These are correctness-critical and cross-subsystem.

## Verify

- Main GC gtests: `ninja arkruntime_memory_management_test_2` (bundles `g1gc_test`, `g1gc_fullgc_test`,
  `epsilon_gcs_test`, `gc_root_test`, `card_table_test`, …) and `ninja arkruntime_gc_trigger_test`. Test sources are in
  `runtime/tests/`; target groupings are defined by `add_gtests(...)` in `runtime/CMakeLists.txt`.
- GC type arguments: `--gc-type=` `g1-gc|stw|epsilon|...`.
