# Interop Context and Lifecycle Knowledge

This document records the management rules, lifecycle, and initialization flow of the main `InteropCtx`.

## InteropCtx Management Model

`InteropCtx` is the state center of the interoperability runtime. Each thread/coroutine that supports interop possesses a local context, which is associated with an `EtsExecutionContext`.

### Core Responsibilities
1.  **Environment Binding**: Holds the `napi_env` for the current JS engine.
2.  **Conversion Cache**: Maintains a `JSRefConvertCache` to avoid redundant creation of type converters.
3.  **Reference Tracking**: Manages `pendingNewInstance` and `SharedReferenceStorage`.
4.  **Exception Propagation**: Responsible for converting between `ThrowJSError` (error on the JS side) and `ThrowETSError` (error on the ETS side).

## Initialization Timing

| Scenario | Initialization Path | Description |
| --- | --- | --- |
| **Main Thread Startup** | `CreateMainInteropContext` | Initializes the global singleton `SharedEtsVmState` and the main `InteropCtx`. |
| **Worker Threads** | `interfaceTable_.CreateInteropCtx` | Creates a local `InteropCtx` on demand when a thread switches to interop mode. |
| **NAPI Loading** | `Init` (ets_vm_plugin.cpp) | Registers `STValue`, `SType`, and other interop components into the JS environment. |

## Lifecycle Principles

- **Resource Centralization**: Prohibit ad-hoc storage of critical resources like `napi_env` or `jsenv` in local variables or custom members. All access must be mediated through `InteropCtx::Current()`.
- **Reference Lifecycle**: Every `napi_ref` created (e.g., in wrappers or caches) must have a clearly defined deallocation path to ensure no memory leaks occur.
- **Singleton State**: `SharedEtsVmState` holds global shared data (e.g., `jsRuntimeClass`), and its lifecycle spans across threads.
- **Resource Cleanup**: When `InteropCtx` is destroyed (`Destroy`), it must clean up `napi_ref`s in the `etsClassWrappersCache` to prevent JS-side reference leaks.
- **XGC Synchronization**: When a thread detaches, it must wait for XGC to complete and clean up all shared references associated with that context.

## Checklist Before Modification

- Is the current operation within `ManagedCode` or `NativeCode` scope? (Use the `INTEROP_CODE_SCOPE` macro).
- Is `napi_env` valid? (Obtain via `InteropCtx::Current()->GetJSEnv()`).
- Is stack information synchronization required? (`UpdateInteropStackInfoIfNeeded`).
- Does it involve cross-VM reference count updates?

## Code Pointers

- **Definition**: `interop_context.h/cpp`
- **Initialization APIs**: `interop_context_api.h` (`CreateMainInteropContext`)
- **Stack Management**: `interop_stacks.h`, `stack_info.h`
- **Lifecycle Hooks**: `napi_add_env_cleanup_hook` binds the `EtsVM` and `JSVM` lifecycles.
