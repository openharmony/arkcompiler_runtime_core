# Branch Profiling Cost In Loop-Heavy Startup Paths

This note records a runtime profile collection cost pattern. It is about branch profiling overhead while collecting
profile data.

## Problem Pattern

An application cold-start time increased from 30+ ms to 600+ ms. After disabling PGO, the time dropped to about
300+ ms. `hiperf` showed that the main VM execution hotspot was `UpdateBranchUntaken`.

## Root Cause

The cold-start phase executed tens of millions of loop branches. When PGO branch profiling is enabled, every branch
updates the taken / not-taken counters. Empty loops or other high-frequency loops can amplify the collection cost on the
cold-start critical path.

## Root Cause Category

Technical root cause: the PGO collection specification and default enablement policy did not sufficiently account for
edge cases such as loop-heavy workloads and high-frequency startup branches.

## Code Path

- `static_core/runtime/options.yaml`: `profile-branches` defaults to `false`.
- `static_core/runtime/runtime.cpp`: branch profiling is automatically enabled when `profile-branches` was not set
  explicitly and JIT is enabled.
- `static_core/runtime/interpreter/instruction_handler_base.h`: interpreter branch handling checks
  `IsBranchProfilingEnabled()` before updating counters.
- `static_core/runtime/entrypoints/entrypoints.cpp`: `UpdateBranchTaken` / `UpdateBranchUntaken` are the irtoc branch
  update entry points.
- `static_core/runtime/jit/profiling_data.h`: `UpdateBranchTaken` / `UpdateBranchNotTaken` find branch data and
  increment the corresponding counters.
- `static_core/runtime/jit/profiling_saver.cpp`: branch data is saved only when branch profiling is enabled.

## Diagnostics

- Compare cold-start time with PGO enabled and disabled.
- Use `hiperf` to confirm whether the hotspot is concentrated in `UpdateBranchUntaken` / `UpdateBranchTaken`.
- Check whether `profile-branches` is set explicitly, and separate default policy from test configuration.

## Handling

Because branch profiling has runtime collection cost, it should not be enabled by default for every PGO scenario. It
should remain an optional collection item, enabled only for JIT or for experiments and optimizations that explicitly need
branch profiles, while preserving explicit configuration precedence.

## Improvement Suggestions

1. PGO design should cover edge cases such as loop-heavy workloads, startup paths, empty-loop tests, and low-benefit
   high-frequency branches.
2. Review all PGO collection items, including inline caches, branch counters, and throw counters, to check for hidden
   high-frequency overhead points.
3. When evaluating branch profiling, describe the collection cost, consumer benefit, default policy, and explicit
   enablement method together.
