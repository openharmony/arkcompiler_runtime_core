# AOT PGO

Ahead-of-time compilation in the current tree can consume runtime profile data from `.ap` files.

The current source of truth for the data model and loader/saver code is:

- `runtime/jit/profiling_data.h`
- `runtime/jit/profiling_loader.h`
- `runtime/jit/profiling_saver.cpp`
- `runtime/jit/libprofile/aot_profiling_data.h`
- `runtime/jit/libprofile/pgo_file_builder.cpp`
- `compiler/tools/paoc/paoc.cpp`

Use this document as the high-level map rather than as a binary-format specification dump. For the current `.ap`
on-disk layout, use `../../docs/bytecode_profiling.md` as the overview and `../../runtime/jit/libprofile/` as the
format owner.

## What Is Collected

Current profile data includes:

- inline caches for virtual call sites
- branch taken / not-taken counters
- throw counters

Relevant current runtime structures:

- `CallSiteInlineCache`
- `BranchData`
- `ThrowData`
- `ProfilingData`

Inline caches currently retain up to four observed receiver classes per call site and mark the site megamorphic once
the cache overflows.

## How Profiles Are Saved

The runtime can save `.ap` profile data to the path from `--profile-output`.

Important current options:

- `--profiler-enabled=true`
- `--profilesaver-enabled=true`
- `--incremental-profilesaver-enabled=true`
- `--profile-output=<file.ap>`
- `--compiler-profiling-threshold=<n>`
- `--profile-branches=true` when detailed branch data is needed

Current behavior:

- runtime profiling starts once methods cross the profiling threshold
- shutdown saving is performed from `Runtime::Destroy()` when profiling and profile saving are enabled
- incremental saving can also run through the profile saver worker
- current saving logic can split profiled panda files by their current profile path snapshot; it is no longer accurate to
  describe this only as "boot and initial app context" output

## How `ark_aot` Uses Profiles

`ark_aot` consumes AOT profiles through:

```bash
--paoc-use-profile:path=<profile.ap>[,force]
```

When reusing a profile, also load the same application panda files through `--panda-files` so they are already present
in the runtime context before the profile is checked and attached. If `--paoc-panda-files=file1.abc,file2.abc`, mirror
that set as `--panda-files=file1.abc:file2.abc`.

Current loading flow in `compiler/tools/paoc/paoc.cpp`:

1. load boot and application panda files into the current runtime context
2. load `.ap`
3. validate class context against the current runtime/AOT context
4. attach per-method profiling data back to loaded methods
5. let optimization passes query that data through the runtime interface

If the class context does not match, the profile is rejected.

## What The Compiler Uses Today

Current profile-guided decisions include:

- inline-cache driven monomorphic or polymorphic inlining
- branch-counter driven block ordering and if-conversion heuristics
- throw-counter driven try/catch and throw-heavy path heuristics

Relevant current code paths:

- `compiler/optimizer/optimizations/inlining.cpp`
- `compiler/optimizer/analysis/linear_order.cpp`
- `compiler/optimizer/analysis/hotness_propagation.h`
- `compiler/optimizer/optimizations/if_conversion.cpp`
- `compiler/optimizer/optimizations/try_catch_resolving.cpp`

## Current Limitations

AOT PGO has important limits:

- AOT mode does not use CHA-based guarded devirtualization in the same way JIT does; `Inlining::ResolveTarget`
  explicitly restricts the CHA path to non-AOT graphs
- external inline-cache driven inlining still follows the normal AOT external-inlining rules; it is gated by
  `--compiler-inline-external-methods-aot` plus the usual AOT availability constraints
- profile data is useful, but it does not remove the usual AOT constraints around unavailable classes, external methods,
  or mismatched load context

## Minimal Current Workflow

1. Run the workload with profiling enabled and profile saving turned on.
2. Inspect the saved `.ap` with `ark_aptool dump`.
3. Re-run `ark_aot` with `--paoc-use-profile:path=<profile.ap>` and matching `--panda-files` inputs.
4. Use `--compiler-regex`, `--compiler-dump`, and `--compiler-disasm-dump:single-file` to verify which methods and
   optimizations changed.

Example shape:

```bash
./out/bin/ark \
  --load-runtimes=ets \
  --boot-panda-files=./out/plugins/ets/etsstdlib.abc \
  --profile-output=workload.ap \
  --profilesaver-enabled=true \
  app.abc app.ETSGLOBAL::main

./out/bin/ark_aptool dump --ap-path workload.ap --output workload.yaml --abc-path app.abc

./out/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=./out/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --panda-files=app.abc \
  --paoc-output=app.an \
  --paoc-use-profile:path=workload.ap
```

## Related Documents

- `../../docs/aptool.md`
- `../../docs/bytecode_profiling.md`
- `../../runtime/jit/libprofile/`
- `performance_workflows.md`
- `../../runtime/jit/AGENTS.md`
