# Runtime JIT Profiling And AOT PGO Data

This directory handles runtime profiling data structures and `.ap` profile IO. It does **not** own JIT queue orchestration or OSR entry logic.

## Scope

Use this directory when the task is about:
- inline caches, branch profiles, throw profiles
- `.ap` profile format
- loading/saving runtime profile data
- AOT PGO inputs consumed by `ark_aot`

Do **not** start here for:
- JIT worker setup
- compiler queues
- OSR trigger logic
- OSR entry / stackmap frame materialization

Those live in:
- `runtime/compiler.cpp`
- `runtime/compiler_thread_pool_worker.cpp`
- `runtime/compiler_task_manager_worker.cpp`
- `runtime/compiler_queue_*.h`
- `runtime/interpreter/instruction_handler_base.h`
- `runtime/osr.cpp`

## Directory Structure

- `profiling_data.h` - runtime profile data structures
  - `CallSiteInlineCache` - receiver type cache for virtual call sites, monomorphic/polymorphic/megamorphic behavior
  - `BranchData` - taken / not-taken counters
  - `ThrowData` - exception frequency counters
  - `ProfilingData` - container indexed by bytecode PCs
- `libprofile/` - `.ap` on-disk layout and serializable AOT profile structures
  - `pgo_header.h` - on-disk header and record layout
  - `pgo_file_builder.{h,cpp}` - profile serialization/deserialization
  - `aot_profiling_data.h` - AOT-facing representation of saved profiling data
- `profiling_loader.{h,cpp}` - load `.ap` into runtime/PAOC-facing structures
- `profiling_saver.{h,cpp}` - serialize runtime profile data to `.ap`
- `profile_saver_worker.{h,cpp}` - background saver worker based on taskmanager queues

## Integration Points

- runtime profiling starts once methods cross `--compiler-profiling-threshold`
- collected data is saved on runtime shutdown or through explicit/incremental profile-saving flows
- saving the profile requires more than `--profile-output`; the current runtime-side switches are
  `--profilesaver-enabled=true` and, for background saves, `--incremental-profilesaver-enabled=true`
- `compiler/tools/paoc` consumes AOT profile data through `--paoc-use-profile:path=<profile-file>[,force]`; the `path` sub-option is required
- when `ark_aot` applies a profile, mirror the application inputs from `--paoc-panda-files` into `--panda-files` so
  the runtime has already loaded them before profile attachment
- `compiler/tools/aptool` is the main offline inspection tool for `.ap`
- current saved profile content includes inline caches, branch counters, and throw counters

Related files outside this directory:
- `runtime/options.yaml` - profiling and hotness thresholds
- `compiler/tools/paoc/paoc.cpp` - AOT PGO loading and use
- `compiler/tools/aptool/` - human-readable profile inspection
- `docs/bytecode_profiling.md` - profiling data model, `.ap` overview, and AOT use
- `docs/aptool.md` - `ark_aptool` usage and output format
- `compiler/docs/aot_pgo.md` - compiler-side profile consumption and AOT restrictions
- `compiler/docs/performance_workflows.md` - perf workflow that may consume these profiles

## Current Caveats

- AOT PGO is profile-guided, but it does not re-enable CHA-style guarded devirtualization for AOT graphs
- external inline-cache driven AOT inlining still follows `--compiler-inline-external-methods-aot` and the normal AOT
  external-inlining checks in `compiler/optimizer/optimizations/inlining.cpp`
- profile class context must match the currently loaded panda-file context or `ark_aot` will reject the profile

## Testing

Direct profiling and `.ap` coverage mainly lives in:

- `compiler/tests/profiling_*`
- `compiler/tests/aptool_dump_test.cpp`

For broader validation, use the owning test guides instead of treating this file as the test matrix owner:

- `../AGENTS.md` for runtime/compiler integration
- `../../tests/AGENTS.md` for checked, runner, and benchmark workflows

## Build

Sources are compiled into the `arkruntime` target. Offline inspection is done through the separate `ark_aptool` executable.
