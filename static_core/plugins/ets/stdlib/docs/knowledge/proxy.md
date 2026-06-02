# Interface-Based Proxy Knowledge

This document only covers the architectural constraints and interception mechanism of the ArkTS interface-based proxy. For performance decisions, see `performance.md`.

## Core Model

An **interface-based proxy** consistent with Java's `java.lang.reflect.Proxy`: the runtime dynamically generates implementation classes for interfaces, and method calls are intercepted via an assembly bridge and dispatched to the `InvocationHandler`. **Only interfaces are supported; concrete classes are not.**

```
User calls proxy.someMethod(arg)  →  Compiler treats it as a normal interface call
  │ vtable/itable call
  ↓
Proxy class ("$Proxy$N", ACC_PROXY | ACC_FINAL)
  Entry point for each interface method → Assembly bridge (EtsProxyEntryPoint, 4 architecture variants)
    │ Save argument registers → Set up C2I boundary frame → Call C++ dispatch function
    ↓
EtsProxyMethodInvoke (C++)
  │ Retrieve original interface method → ArgReader parses arguments → Box primitive types
  │ Force interpreter dispatch via IS_PROXY flag:
  │   Regular method → Proxy.invoke() → handler.invoke()
  │   Getter         → Proxy.invokeGet() → handler.get()
  │   Setter         → Proxy.invokeSet() → handler.set()
  └ Unbox return value → Assembly bridge places it back into ABI registers
```

## Key Components

| Component | Responsibility |
| --- | --- |
| `InvocationHandler` interface | `invoke(proxy, method, args)` / `get(proxy, method)` / `set(proxy, method, value)` |
| `Proxy.create()` | Calls native `generateProxy` → `BuildProxyClass` → Sets assembly bridge entry points for each interface method |
| Assembly bridge | Calling convention adaptation + argument collection + trampoline between compiled and managed code (amd64/aarch64/arm/armhf) |
| `EtsProxyMethodInvoke` | Argument parsing, boxing, dispatching, and unboxing |

## File Locations

| Layer | Path | Responsibility |
| --- | --- | --- |
| ETS | `std/core/ReflectProxy.ets` | Proxy class, InvocationHandler |
| Native intrinsic | `../runtime/intrinsics/reflect_Proxy.cpp` | `generateProxy`, `hasProxyFlag` |
| Class linker | `../runtime/ets_class_linker_extension.cpp` | `BuildProxyClass`, `CreateProxyMethod` |
| Assembly bridge | `../runtime/entrypoints/arch/`, `../../../runtime/bridge/arch/` | Calling convention adaptation for 4 architectures |
| C++ dispatch | `../runtime/entrypoints/ets_proxy_entrypoints.cpp` | `EtsProxyMethodInvoke` |

## Constraints

- **Only interfaces are supported**; proxying concrete classes is not supported — Reason: Consistent with Java's design; class-based proxying requires a completely different bytecode enhancement mechanism
- The compiler **has no awareness** of proxies; they are generated entirely at runtime — Reason: Proxy classes do not exist in compile-time type information
- The assembly bridge is the sole interception point; **modifying** `SetCompiledEntryPoint` in `CreateProxyMethod` is **forbidden** — Reason: Changing the entry point setup would cause interception to fail
- Handler dispatch is forced through the interpreter via the `IS_PROXY` flag; **do not** attempt JIT compilation — Reason: JIT optimization may break the dynamic dispatch semantics of proxies
- All 4 architecture variants of the assembly bridge **must** be updated in sync — Reason: Updating only one architecture would cause proxies to fail entirely on that architecture
- `proxyPointer` reuses the Method's `native_pointer` field; **storing other data** in proxy methods is **forbidden** — Reason: Proxy methods are not native; the `native_pointer` field is already occupied by the proxy mechanism

## Pre-Modification Checklist

- Have all architecture variants of the assembly bridge been updated in sync?
- Does argument boxing/unboxing cover all EtsType values?
- Is handler exception propagation correct? (The assembly bridge checks for pending exceptions)
- Is the `IS_PROXY` call flag semantics correctly maintained?

## Code and Tests

- ETS source: `std/core/ReflectProxy.ets`
- Assembly bridge: `../runtime/entrypoints/arch/`, `../../../runtime/bridge/arch/`
- Functional tests: `../tests/ets_func_tests/std/core/Proxy*.ets`
- Linker tests: `../tests/ets_test_suite/linker/proxy/`
