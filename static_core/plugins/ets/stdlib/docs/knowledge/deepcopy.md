# DeepCopy Knowledge

This document records only the architectural constraints and implementation strategy of ArkTS deepcopy.

## Core Model

Pure managed-layer implementation with no dedicated intrinsic. Field-by-field copying with `Map<Object, Object>` for circular reference detection.

```
deepcopy<T>(src, depth=-1) → DeepCloner.clone(src)
  ├ null/undefined/String/BaseEnum/Function → returned as-is
  ├ Boolean/Byte/Char/Short/Int/Long/Float/Double/Date → copy constructor
  ├ Array → element-wise recursion
  ├ Set → element-wise recursion
  ├ Map → key + value recursion
  ├ FixedArray(primitive type) → copyOf() native memcopy
  ├ FixedArray(reference type) → createFixedArray + element-wise recursion
  └ Object
    ├ Cloneable + overrides clone() → obj.clone() (user-controlled)
    ├ Cloneable without overriding clone() → throws exception
    ├ Has default constructor → field-by-field recursive copy
    └ No default constructor → throws exception
```

## Cloneable Specification

`Cloneable` provides deepcopy support for classes without a default constructor: `clone(): Cloneable`.

| Condition | Behavior |
| --- | --- |
| Implements `Cloneable` + overrides `clone()` | deepcopy calls `obj.clone()`, fully user-controlled |
| Implements `Cloneable` + does not override `clone()` | Throws exception |
| Does not implement + has default constructor | Field-by-field recursive copy |
| Does not implement + no default constructor | Throws exception |

Override detection: iterates all instance methods of the class to find a method named `"clone"`.

## Generic Erasure

DeepCopy **is not affected by generic erasure**: it obtains the actual type from the runtime instance via `Class.of(obj)`, not from compile-time generic parameters. Even if a container is declared as `Array<Object>`, if an element is an instance of `MyClass`, it will still be correctly identified and deep-copied.

## Circular References

The `copies` Map registers the object immediately after creating the empty shell and before recursively copying fields (`copies.set(obj, objCopy)`). When recursion encounters an already-registered object, the cached copy is returned.

## Constraints

- Classes being copied **must** have a parameterless constructor or implement `Cloneable` and override `clone()` — reason: otherwise the target instance cannot be created
- Copying immutable types (String, BaseEnum, Function) is **prohibited** — reason: they are singletons or immutable; copying is meaningless and would break singleton semantics
- Circular references **must** be registered in `copies` before recursive field copying — reason: incorrect registration timing leads to infinite recursion
- **Do not** add new intrinsics for this — reason: the performance bottleneck is in type queries and field read/write; adding intrinsics yields limited benefit

## Pre-Modification Checklist

- Is the copy strategy for newly added types covered in the type dispatch within `clone()`?
- Is the `copies` Map registration timing before field recursion?
- Do classes without a default constructor correctly throw exceptions?

## Code and Tests

- ETS source: `std/core/deepcopy.ets`
- Tests: `../tests/ets_func_tests/std/core/DeepCopyTest.ets`, `DeepCopyTest2.ets`
