# Interop JS Architecture Knowledge

This document records the core architectural model of the ArkTS (Static/ETS) and JavaScript (Dynamic/ArkTS-Dyn) interoperability layer (`interop_js`).

## Directory Structure and Layers

The interop layer is located in `plugins/ets/runtime/interop_js/`. As part of the ETS runtime, it bridges the ArkVM and the JavaScript engine via NAPI.

| Module | Responsibility | Core Code Anchors |
| --- | --- | --- |
| **`ets_proxy/`** | JS accessing ETS | `ets_class_wrapper.cpp`, `shared_reference.cpp` |
| **`js_proxy/`** | ETS accessing JS | `js_proxy.cpp`, `js_refconvert.cpp` |
| **`call/` & `arch/`** | Call bridging | `call_js.cpp`, `call_ets.cpp`, `arch/` assembly bridges |
| **`st_value/`** | Static value management | `ets_vm_STValue.cpp`, `STValue` API |
| **`intrinsics/`** | Compiler inlining | `intrinsics_api_impl.cpp`, `intrinsics.yaml` |
| **`xgc/`** | Cross-language GC | `xgc.cpp`, `shared_reference_storage.cpp` |
| **`napi_impl/`** | Wrappers for NAPI functions | `napi_impl.cpp` |
| **`docs/`** | API documentation and knowledge base | `ESValueApi_en.md`, `knowledge/` |

## Core Models

### 1. InteropCtx (Central Context)
The hub for all interoperability behaviors. It holds the `napi_env` and is associated with an `EtsExecutionContext`. It is responsible for caching `JSRefConvert`, managing string storage (`JSValueStringStorage`), and handling cross-language exceptions.

### 2. Bidirectional Interop APIs
- **ESValue**: A wrapper class in ArkTS-Sta for manipulating dynamic ArkTS-Dyn objects. It provides dynamic operations such as `getProperty`, `call`, and `invoke`.
- **STValue**: A wrapper class in ArkTS-Dyn for accessing static ArkTS-Sta modules. It handles overloads via mangling signature rules.

### 3. SharedReference (Shared Reference)
The core object for tracking references across runtimes. It is stored in `SharedReferenceStorage` and supports lifecycle management via XGC (Cross-Runtime GC).

## Key Principles

- **Separation Principle**: Policy decisions should not sink into assembly bridge code; they should be handled by `CallJSHandler`/`CallETSHandler`.
- **Performance First**: High-frequency paths (e.g., pointer movement, member access) should use `intrinsics` or optimized cache paths.
- **Type Safety**: Enforce SType validation in `STValue` operations; use `EtsClassWrapper` in `ets_proxy` to ensure type consistency.

## Technical Mandates and Design Rules

This section consolidates development constraints and architectural rules from across the interop subsystem.

### 1. Development and Safety Constraints
- **Thread State Discipline**: Always use explicit macros (`INTEROP_CODE_SCOPE`, `ScopedManagedCodeReady`, etc.) for switching between Managed and Native states. Consolidate adjacent switches into a single scope to minimize runtime overhead.
- **Safe Reference Management**: Never use raw pointers for `EtsObject`. All ETS objects MUST be wrapped in `EtsHandle` (or `EtsPersistentHandle`) to ensure GC safety and prevent null-pointer access.
- **Exception Protocol**: Before operations that may throw, check for `pendingError` in the `InteropCtx` or NAPI environment. Handle or propagate existing errors before introducing new ones.
- **Resource Centralization**: Do not store critical resources like `napi_env` or `jsenv` locally. Always retrieve them via `InteropCtx::Current()`.
- **Leak Prevention**: Every `napi_ref` created must have a corresponding deallocation mechanism (e.g., in a destructor or lifecycle hook).

### 2. Calling and Bridge Mandates
- **State Switch Before JS**: Must switch to `ManagedThread::Status::NATIVE_CODE` before entering the JS engine environment.
- **Register & Stack Integrity**: Assembly bridges must preserve all non-volatile registers and maintain target architecture stack alignment requirements.
- **Error Mapping**: Bridge logic must capture NAPI status codes and map them correctly to `Expected<T, Error>` for the ETS runtime.

### 3. Object and Lifecycle Rules
- **XGC Stability**: During `SuspendAll`, the object graphs on both sides must remain stable. Use `ResumeGuard` to ensure mutator resumption in exception paths.
- **Reference Integrity**: Direct deletion of `SharedReference` is prohibited; cleanup must be handled by `SweepUnmarkedRefs` or explicit `DeleteReference` logic.
- **Multi-Context Support**: References for a single `EtsObject` across multiple contexts should form a linked structure to prevent leaks in complex worker scenarios.

### 4. Conversion and Proxy Design
- **Overload Selection**: Use the **First-Fit** algorithm based on Bytecode Line Number for resolving front-end overloads.
- **Value Mapping**: Map JS `undefined` to ETS `nullptr` (for references) and JS `null` to `NullValue`.
- **Finalizer Requirement**: Non-primitive `JSValue` types (String, Object) must have a `Finalizer` attached to manage their cross-runtime lifecycle.
- **Lazy Metadata Loading**: Load metadata (like `ConstStringStorage`) only upon the first access to the corresponding namespace or class.

## Code and Test Guidelines

- **Core Entry Points**: `ets_vm_plugin.cpp` (NAPI plugin entry), `interop_context.h` (`InteropCtx` definition).
- **Build Targets**: `ets_interop_js_napi` (Shared Library), `libarkruntime.so` (contains core implementation).
- **Verification Command**: `ninja -v ets_interop_js_gtests` to run the full suite of interop tests.
- **Formatting Standards**: Modified C++/Header files must be formatted using `bash code-format.sh format-changed`.
