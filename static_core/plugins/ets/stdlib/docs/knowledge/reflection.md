# Reflection Knowledge

This document only covers architectural constraints of the ArkTS reflection system and its binding relationship with the runtime. For performance constraints, see `performance.md`.

## Core Model

Reflection directly manipulates VM internal objects via intrinsics, making extensive use of boxing/unboxing (`EtsBoxPrimitive<T>`).

```
ETS application layer (Class.from<T>() / method.invoke() / field.getValue())
  │ intrinsic
  ↓
C++ Intrinsic layer
  ├ std_core_Class.cpp           class operations
  ├ std_core_reflect_Method.cpp  method invocation (param validation → unbox → VM Invoke → box return)
  ├ std_core_reflect_Field.cpp   field access (type validation → read/write → box/unbox)
  └ std_core_Reflect.cpp         auxiliary
  │
  ↓ operates on VM internal structures
C++ runtime types
  EtsClass   ← same object as managed Class (tail-embedded ark::Class)
  EtsMethod  ← thin wrapper, delegates to ark::Method (reinterpret_cast interop)
  EtsField   ← thin wrapper, delegates to ark::Field
```

### Inheritance Hierarchy

```
Method (base: methodPtr, attributes)           Field (base: fieldPtr, attributes)
  ├─ InstanceMethod  (ETS layer: isFinal/isAsync)   ├─ InstanceField (ETS layer: readonly check)
  ├─ StaticMethod                                   └─ StaticField
  └─ Constructor (createInstance)
```

### ETS Class to Runtime Type Mapping

| ETS managed type | C++ mirror object | Held pointer | Core runtime type |
| --- | --- | --- | --- |
| `Class` | `EtsClass` (same object) | tail-embedded | `ark::Class` |
| InstanceMethod/StaticMethod/Constructor | `EtsReflectMethod` | `etsMethod_` → `EtsMethod*` | `ark::Method` |
| InstanceField/StaticField | `EtsReflectField` | `etsField_` → `EtsField*` | `ark::Field` |

## Mirror Classes vs Runtime Wrapper Classes

| | Mirror class (EtsReflectMethod/Field) | Runtime wrapper class (EtsMethod/Field) |
| --- | --- | --- |
| Nature | Managed heap GC object, layout **must** correspond one-to-one with ETS fields | Lightweight C++ object on stack/heap, reinterpret_cast interop with `ark::Method`/`Field` |
| Lifetime | GC managed | Same lifetime as VM internal Class file structures |
| Purpose | "Handle" held and passed by ETS code for reflection metadata | Actually performs metadata queries, method invocation, field read/write |
| Ownership | Holds raw pointer via `long` field (`etsMethod_`/`etsField_`) | Directly operates on `ark::Method`/`Field` |

`EtsClass` **does not go through the mirror layer**: it is the same object as the managed `Class`. `methodPtr`/`fieldPtr` are `long` raw pointers, not managed by GC.

## Intrinsic Inventory

There are 30+ intrinsics in total:

| Group | Key methods |
| --- | --- |
| Class operations | `isSubtypeOf`, `isEnum`/`isInterface`/`isFinal`, `getInstanceMethodsInternal`, `getInstanceFieldsInternal`, `getConstructorsInternal`, `of`/`current`/`ofCaller` |
| Method operations | `invokeInternal` (param validation → unbox → Invoke → box), `getReturnTypeImpl`, `getParameterTypesImpl`, `createInstanceInternal` |
| Field operations | `getValueInternal` (validation → read → box), `setValueInternal` (validation → unbox → write), `getTypeInternal` |

Attributes bitmask: `STATIC`(0), `READONLY`(1), `FINAL`(2), `ABSTRACT`(3), `CONSTRUCTOR`(4), `NATIVE`(7), `ASYNC`(8), `GETTER`(9), `SETTER`(10). The ETS layer checks these via bitwise operations, no intrinsic needed.

## Constraints

- **Do not** introduce third-party library calls within reflection intrinsics — reason: safepoint constraints, see `performance.md`
- `methodPtr`/`fieldPtr` are raw pointers, you **must** ensure the underlying EtsMethod/EtsField lifetime covers the usage period — reason: not managed by GC; if the underlying object is unloaded the pointer becomes dangling
- `EtsClass` and the managed `Class` are the same object, **do not** allocate additional Class instances — reason: violating identity would cause type system confusion
- Mirror object layout **must** correspond one-to-one with ETS fields; when changed, you **must** synchronously update the C++ header files — reason: layout mismatch causes field read/write misalignment
- `Class.from<T>()` is replaced by the frontend at compile time; the runtime **will not** execute its method body — reason: generics are erased after compilation, no type parameter information at runtime
- Virtual method resolution is done by `ResolveVirtualMethod` inside intrinsics, **do not** implement it in the ETS layer — reason: requires vtable/itable lookup, which the ETS layer cannot access

## Pre-modification Checklist

- Will the new intrinsic affect safepoint intervals?
- Have mirror object layout changes been synchronously updated in C++ header files?
- Does boxing/unboxing cover all EtsType values?
- Is the `methodPtr`/`fieldPtr` lifetime safe?
- Have `mirror_class` declarations and intrinsic entries been added to YAML?

## Code and Tests

- ETS source: `std/core/Class.ets`, `std/core/Reflect*.ets`
- Native: `../runtime/intrinsics/std_core_Class.cpp`, `std_core_reflect_*.cpp`
- Mirror types: `../runtime/types/ets_reflect_method.h`, `ets_reflect_field.h`
- Functional tests: `../tests/ets_func_tests/std/core/Reflect*.ets`
