# Interop Calling Convention and Assembly Bridge Knowledge

This document records the design of Calling Conventions and Call Bridges within the cross-language call chain.

## Call Linkage Model

Cross-language calls involve stack switching, parameter rearrangement, and ABI conversion.

### 1. JS -> ETS (JavaScript calling static code)
1.  **Interception**: JS triggers the `napi_callback` of a proxy object.
2.  **Processing**: Enter `EtsMethodCallHandler` or `JSRuntimeCallJSBridge`.
3.  **Conversion**: `CallETSHandler` parses the `napi_value` and converts it into an internal ETS `Value`.
4.  **Jump**: Executes managed code via `Method::Invoke`.

### 2. ETS -> JS (Static code calling JavaScript)
1.  **Inline Point**: The compiler identifies an `ESValue` operation or a proxy call.
2.  **Assembly Bridge**: Jumps to architecture-specific bridge code (e.g., `CallJSProxyBridge`).
3.  **State Machine**: `CallJSHandler` uses `ArgReader` to extract parameters from the assembly stack.
4.  **Dispatch**: Executes JS code via `napi_call_function`.

## Assembly Bridge Implementation (`arch/`)

Provides low-level support for different hardware architectures:
- **`amd64`**: `call_bridge_amd64.S`
- **`arm64`**: `call_bridge_aarch64.S`
- **`arm32`**: `call_bridge_arm32.S` / `call_bridge_arm32hf.S`

**Core Macro**: `PROXY_ENTRYPOINT`. It defines how to save register state, switch to Native mode, and call the C++ handler.

## Key Rules

- **Thread State Transition**: Explicitly call state switching macros (e.g., `INTEROP_CODE_SCOPE`) between managed and native code. Consolidate consecutive state transitions into a single scope to minimize runtime overhead.
- **Register Preservation**: The assembly bridge must save all non-volatile registers to ensure the ETS runtime state remains intact after returning from JS.
- **Stack Alignment**: Ensure compliance with the target architecture's stack alignment requirements before cross-language calls.
- **Error Code Propagation**: The assembly bridge must support capturing NAPI status and mapping it to `Expected<T, Error>`.
- **Thread State**: Must switch to `ManagedThread::Status::NATIVE_CODE` before entering the JS environment.

## Typical Boundaries

| Scenario | Processor | Responsibility |
| --- | --- | --- |
| **QName Call** | `JSRuntimeCallJSQName` | Handles Qualified Name lookups. |
| **Value Call** | `JSRuntimeCallJSByValue` | Direct dispatch by value. |
| **Proxy Call** | `CallJSProxy` | Automatically handles unpacking of the receiver (`this`). |
| **Constructor Call** | `JSRuntimeNewCallJS` | Handles `new` semantics. |

## Test Guidelines

- **Architecture Verification**: Run `ets_interop_js_gtests` on different simulators/devices.
- **Inline Verification**: Inspect compiler-generated IR to ensure the correct `IntrinsicId` is called.
- **Performance Benchmarking**: Monitor template expansion overhead in `call/arg_convertors.h`.
