# ArkTS Type System Knowledge

This document records only the special constraints and easily confused concepts of the ArkTS type system.

## Core Model

ArkTS **has no primitive types**. At the spec level, all basic types (`int`, `double`, `boolean`, etc.) are reference types (classes).

This is similar to Kotlin: **bytecode generation automatically boxes/unboxes based on context, but the spec level does not distinguish between primitive and boxed**. The compiler is responsible for optimized representation at the underlying bytecode level; developers do not need to and should not be aware of this.

```
Spec Level (ArkTS Language Specification)
  All basic types = reference types (classes), no primitive concept
       |
       ↓ Compiler handles automatically
Bytecode Level
  Underlying representation determined by usage context:
  ├─ Local variables, arithmetic operations → may be optimized to primitive representation (similar to unboxed)
  ├─ Generic parameters, Object fields, array elements → boxed to reference representation
  └─ Boxing/unboxing inserted by the compiler, transparent to developers
```

## Boundaries / Identity

| Concept | Spec-Level Identity | Bytecode-Level Behavior | Common Misuse |
| --- | --- | --- | --- |
| `int`, `long`, `double`, `float` | Class, not primitive | Boxing/unboxing determined by context | Treating as Java primitive and assuming default value of 0 |
| `boolean`, `byte`, `short`, `char` | Class, not primitive | Same as above | Expecting false/0 default values |
| `Int`, `Long`, `Double`, `Float` | **Completely identical** to lowercase forms | No difference | Assuming they are boxed wrappers |
| `Boolean`, `Byte`, `Short`, `Char` | **Completely identical** to lowercase forms | No difference | Treating case-sensitive forms as different types |
| Lowercase type names (e.g., `double`) | Type alias, equivalent to uppercase form | No difference | Assuming they are different boxed/unboxed pairs |

## FixedArray vs ValueArray vs Array

Three array types exist in ArkTS, each with distinct semantics.

| | `FixedArray<T>` | `ValueArray<T>` | `Array<T>` |
|---|---|---|---|
| Length | Fixed | Fixed | Dynamic |
| Generic T | Any type (references, `Any`) | **Primitives only** (`boolean/byte/char/short/int/long/float/double`) | Any type |
| Can store references | Yes | No | Yes (all elements are `Any` at runtime) |
| GC write barrier | Needed for reference elements | Not needed (no references) | Needed (via native `$_set`) |
| C++ mirror | `EtsTypedObjectArray` | `EtsPrimitiveArray` only | `EtsEscompatArray` |

### When to use which

- **`FixedArray<Any>`** — need to store references, or use as a general-purpose buffer (e.g., `Array<T>.buffer`, Map's `data` array holding keys + values + nested references)
- **`ValueArray<int>`** / **`ValueArray<char>`** etc. — pure numeric buffer with no GC overhead (e.g., Map's bucket indices, BigInt's digit array, String's character buffer in `padStart`/`padEnd`)
- **`Array<T>`** — dynamic sizing needed; internally wraps `FixedArray<Any>` with grow/shrink logic

### Key facts

- `FixedArray<int>` compiles but boxes each element to `Int` objects (`EtsTypedObjectArray<Int>`, with GC write barrier); use `ValueArray<int>` instead (`EtsPrimitiveArray`, no GC overhead)
- `ValueArray<T>` signals "no GC-managed references here, purely doing value computation" — prefer it for numeric buffers and hash table internals
- `BuiltinArray.ets` is auto-generated from templates and provides methods (at, copyWithin, fill, join, map, slice, sort, etc.) for each `FixedArray<T>` variant — do not edit directly

## Constraints

- **Do not** write `new BigInt(bigintValue.toString())` when `bigintValue` is already a `BigInt` — Reason: same as above
- **Do not** assume basic types have primitive default values (e.g., `int` defaults to `0`, `boolean` defaults to `false`) — Reason: primitives do not exist at the ArkTS spec level
- **Do not** attempt to manually control boxing/unboxing — Reason: this is the compiler's responsibility; manual control conflicts with compiler optimizations
- **Do not** assume case-variant type names have performance differences — Reason: they are the same type with no differences whatsoever
- **Do not** use `ValueArray<T>` for reference types — Reason: `ValueArray` only supports primitives; use `FixedArray<T>` instead
- **Do not** call methods on `ValueArray<T>` — Reason: it has no methods beyond `[]` and `length`; if you need array methods, convert to `FixedArray` or use `Array`

## Pre-Modification Checklist

- Did you distinguish between "primitive types" and "reference types"? Primitive types do not exist at the ArkTS spec level
- Are there redundant `new X(...)` constructions? Check whether the argument is already the target type
- Did you assume primitive default value behavior?
- Are you attempting to manually optimize boxing/unboxing? This should be left to the compiler
- Did you choose the correct array type? `ValueArray` for pure numeric buffers, `FixedArray` when references are involved, `Array` for dynamic sizing
- Are you converting between `FixedArray` and `ValueArray` unnecessarily? Each conversion is an O(n) copy

## Code and Tests

- Core type definitions: `std/core/`
- Type-related tests: `../tests/ets_func_tests/std/core/`
