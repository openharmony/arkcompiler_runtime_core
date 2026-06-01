# Interop Shared Reference and XGC Knowledge

This document records the core logic for cross-runtime reference tracking (`SharedReference`) and customized garbage collection (`XGC`).

## Shared Reference Model

`SharedReference` is the vital bridge connecting two runtimes. It is managed within the `SharedReferenceStorage`.

### Reference Flags
- **ETS**: The state of this object resides primarily in the ETS runtime.
- **JS**: The state of this object resides primarily in the JS runtime.
- **MARK**: XGC mark bit, used for concurrent mark-and-sweep.
- **NextIndex**: Forms a reference chain (one ETS object may be referenced across multiple interop contexts).

### SharedReferenceStorage (Storage Repository)
- Efficient memory management based on `ItemsPool`.
- **Indexing Mechanism**: Uses the `MarkWord` of an ETS object to store the interop index (`interop_index`).
- **Limit**: The maximum pool size on X64 architecture is typically 1GB.

## XGC (Cross-Runtime GC)

XGC is responsible for cleaning up `SharedReference`s that are no longer used by either runtime.

### Core Phases
1.  **Initial Mark**: Clears all reference mark bits and starts the `StartXGCBarrier`.
2.  **Concurrent Mark**: Tracks bidirectional references from JS to ETS and ETS to JS.
3.  **Remark**: Processes references newly allocated during the concurrent marking phase.
4.  **Sweep**: Cleans up unmarked `SharedReference`s and releases associated `napi_ref`s.

### Trigger Policy (`TriggerPolicy`)
Configurable via `runtime_options.yaml`:
- `default`: Triggered based on `xgc-min-trigger-threshold`.
- `force`: Attempt to trigger on every allocation.
- `never`: Disable XGC.

## Key Principles

- **Consistency**: Ensure the object graphs on both sides are in a stable state during `SuspendAll`.
- **Exception Safety**: `ResumeGuard` must be used to ensure the mutator can be resumed even in exception paths.
- **Reference Integrity**: Direct deletion of `SharedReference` is disabled; it must pass through `SweepUnmarkedRefs` or explicit `DeleteReference` logic.
- **Multi-context Support**: All context references corresponding to one `EtsObject` should form a linked structure to prevent leaks in complex Worker scenarios.

## Code and Test Pointers

- **Storage Management**: `shared_reference_storage.h/cpp`, `items_pool.h`
- **GC Core**: `xgc.h/cpp`, `sts_vm_interface_impl.cpp`
- **Verification System**: `shared_reference_storage_verifier.cpp`
- **Configuration**: `runtime_options.yaml`
