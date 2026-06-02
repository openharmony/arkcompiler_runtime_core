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

## Constraints

- **Do not** write `new Double(value.value)` when `value.value` is already a `Double` — Reason: redundant construction causes unnecessary object allocation
- **Do not** write `new BigInt(bigintValue.toString())` when `bigintValue` is already a `BigInt` — Reason: same as above
- **Do not** assume basic types have primitive default values (e.g., `int` defaults to `0`, `boolean` defaults to `false`) — Reason: primitives do not exist at the ArkTS spec level
- **Do not** attempt to manually control boxing/unboxing — Reason: this is the compiler's responsibility; manual control conflicts with compiler optimizations
- **Do not** assume case-variant type names have performance differences — Reason: they are the same type with no differences whatsoever

## Pre-Modification Checklist

- Did you distinguish between "primitive types" and "reference types"? Primitive types do not exist at the ArkTS spec level
- Are there redundant `new X(...)` constructions? Check whether the argument is already the target type
- Did you assume primitive default value behavior?
- Are you attempting to manually optimize boxing/unboxing? This should be left to the compiler

## Code and Tests

- Core type definitions: `std/core/`
- Type-related tests: `../tests/ets_func_tests/std/core/`
