# Bytecode Profiling And `.ap` Profiles

This note is a compact cross-subsystem overview of the current profiling flow. The runtime-side source of truth lives
in `runtime/jit/`, especially `profiling_data.h`, `profiling_loader.{h,cpp}`, `profiling_saver.{h,cpp}`, and
`libprofile/`.

## What The Runtime Collects

Current profiling data includes:

- inline caches for virtual call sites
- branch taken / not-taken counters
- throw counters

Runtime structures for these live in `runtime/jit/profiling_data.h`.

## How It Is Collected

- profiling-capable bytecodes carry a profile index that selects a slot in the method profile data
- runtime allocates per-method profiling data only for methods that contain profiling-enabled instructions
- the profile layout is language-defined; core runtime does not interpret it generically
- interpreter execution reads the profile index from bytecode and updates the corresponding slot during bytecode handling
- profiling begins once methods cross `--compiler-profiling-threshold`
- additional branch detail is controlled by `--profile-branches=true`

Relevant runtime options are defined in `runtime/options.yaml`.

## How Profiles Are Saved

Current saved profiles use the `.ap` format.

Important options are:

- `--profiler-enabled=true`
- `--profilesaver-enabled=true`
- `--incremental-profilesaver-enabled=true` for background saves
- `--profile-output=<file.ap>`

The runtime can save profiles on shutdown or through incremental profile-saver flows.

## How The Compiler Uses Them

`ark_aot` consumes runtime profiles through:

```bash
--paoc-use-profile:path=<profile.ap>[,force]
```

Current compiler-facing usage is documented in `../compiler/docs/aot_pgo.md`.

## Inspection

Use `ark_aptool` to inspect a saved profile:

```bash
./out/bin/ark_aptool dump --ap-path workload.ap --output workload.yaml
```

Add `--abc-path` or `--abc-dir` when you need resolved class or method names.

## Related Guides

- `../runtime/jit/AGENTS.md` - runtime-side ownership and caveats
- `../compiler/docs/aot_pgo.md` - compiler consumption and current limitations
- `aptool.md` - `ark_aptool` usage
