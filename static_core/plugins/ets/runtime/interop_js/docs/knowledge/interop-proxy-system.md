# Interop Proxy Mechanism Knowledge

This document records the implementation logic for `ets_proxy` (JS-side proxy for ETS) and `js_proxy` (ETS-side proxy for JS).

## Proxy System Model

The proxy mechanism works by creating a surrogate object in the target language that intercepts access to properties and methods and forwards them to the original language.

### 1. ets_proxy (JavaScript accessing ArkTS-Sta)
- **Core Classes**: `EtsClassWrapper`, `EtsMethodWrapper`, `EtsFieldWrapper`.
- **Implementation Principle**: Defines a constructor and prototype in JS that corresponds to the ETS class via `napi_define_class`.
- **Method Dispatch**:
    - **Static Methods**: Bound to the JS constructor.
    - **Instance Methods**: Intercepted by `EtsMethodCallHandler`, which uses the `SharedReference` on `this` to find the original ETS object.
- **Field Access**: Uses a `Getter`/`Setter` pattern.

### 2. js_proxy (ArkTS-Sta accessing JavaScript)
- **Core Class**: `js_proxy::JSProxy`.
- **Implementation Principle**: Dynamically constructs an ETS class with the `ACC_PROXY` flag at runtime, intercepting all virtual method calls.
- **Assembly Bridging**: Uses `CallJSProxyBridge` (arch-specific) to redirect ETS calls to the JS engine.

## Key Design Decisions (ADR)

- **Inheritance Simulation**: Simulates ETS class inheritance on the JS side using `setPrototypeOf`.
- **Overload Resolution**: 
    - Static frontend overloads are based on **Bytecode Line Number** sorting and a **First-Fit** algorithm.
    - Numeric narrowing follows the priority: `byte` -> `short` -> `int` -> `float` -> `double`.
- **Lazy Binding**: The corresponding `EtsClassWrapper` is generated only when the class is first created or accessed on the JS side.
- **XRef Types**: Proxy classes are typically marked as `XRefClass` for special handling and identification by the runtime.

## Interaction Flow

1.  **Creation**: JS calls `etsvm.getClass("MyClass")`.
2.  **Encapsulation**: `EtsClassWrapper::Get` lookups or creates a wrapper.
3.  **Instantiation**: JS `new MyProxy()` triggers `JSCtorCallback`.
4.  **Linkage Establishment**: Creates a `SharedReference` and binds `jsThis` with `etsObject`.
5.  **Invocation**: JS accesses a method, triggering `EtsMethodCallHandler` -> `CallETSInstance`.

## Code and Test Guidelines

- **ETS-side Proxy**: `ets_proxy/ets_class_wrapper.cpp`, `ets_proxy/ets_proxy.cpp`
- **JS-side Proxy**: `js_proxy/js_proxy.cpp`, `js_refconvert.cpp`
- **Method Overloading**: `JS_TO_ETS_OVERLOAD_DESIGN.md`, `ets_method_set.cpp`
- **Verification**: `ets_interop_js_gtests`
