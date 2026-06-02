# JsonElement Knowledge

This document only records architectural constraints and design decisions for ArkTS `JsonElement`. For the type system, see `type_system.md`.

## Core Model

Container-based JSON DOM with a **single class + enum discriminator** design, purely managed implementation, no dedicated intrinsics.

```
jsonx namespace
├ enum JsonType (8 values: Undefined/Object/Array/String/Number/True/False/Null)
├ class JsonElement (internal tagged union: exactly one optional field holds a value)
├ enum BigIntMode (DEFAULT/PARSE_AS_BIGINT/ALWAYS_PARSE_AS_BIGINT)
├ JsonError / JsonTypeError
├ JsonElementSerializable    { toJSON(): JsonElement }
└ JsonElementDeserializable  { fromJSON(JsonElement): void }
```

Data flow: `parseJsonElement()` → `UnifiedJsonParser` character-by-character recursive descent → JsonElement tree; `stringifyJsonElement()` → `stringifyElementRecursively()` branches on JsonType for output.

## Type-Safe Access

| Style | Method Pattern | On Mismatch |
| --- | --- | --- |
| Strict | `asDouble()`, `getString(key)` | Throws `JsonTypeError` |
| Lenient | `tryAsDouble()`, `tryGetString(key, fallback)` | Returns `undefined` or fallback |

- `JsonTrue`/`JsonFalse` are independent enum values; `JsonNumber` uniformly covers double/long/bigint
- JSON `null` (`JsonNull`) and "no value" (`JsonUndefined`) are distinct states

## Number Parsing

Integers: `ALWAYS_PARSE_AS_BIGINT` → bigint; `PARSE_AS_BIGINT` → falls back to bigint on long overflow; `DEFAULT` → throws FormatError on long overflow. Floating-point always → double.

## Custom Serialization and Deserialization

| Interface | Method | Description |
| --- | --- | --- |
| `JsonElementSerializable` | `toJSON(): JsonElement` | Class customizes serialization output |
| `JsonElementDeserializable` | `fromJSON(JsonElement): void` | Instance method (not a static factory), operates on `this` fields, supports multiple calls for data updates |

## Container-Style Deserialization and Generic Erasure

After ArkTS generic erasure, the type parameters of `Map<string, MyClass>` are lost. JsonElement solves this through a **two-step separation**:

1. `parseJsonElement()` performs only syntactic parsing → JsonElement DOM (carries its own `JsonType` discriminator, independent of business types)
2. User code extracts values field by field from JsonElement → explicitly calls `asDouble()`/`asString()` to construct business objects

This shifts "type inference" from the framework (constrained by erasure) to user code (unconstrained), using the unified type `JsonElement` in place of erased concrete type parameters.

## Constraints

- **Do not** add native code for JsonElement — Reason: purely managed implementation is its core design; adding native code introduces unnecessary safepoint risks
- The tagged union **must** guarantee exactly one field holds a value; `set*()` must call `setUndefined()` first — Reason: multiple fields holding values causes `jsonType` resolution confusion
- `asInteger()` only returns from long storage; **do not** assume implicit conversion — Reason: double→int truncation is a silent operation that causes precision loss
- `setElement(key, value)` changes `JsonType` — Reason: automatically converts to JsonObject context; improper call ordering can overwrite existing values

## Pre-Modification Checklist

- Does a new `JsonType` enum value synchronize updates to the `jsonType` getter and all `as*()`/`tryAs*()` methods?
- Do number parsing changes cover all three `BigIntMode` values?
- Does `stringifyElementRecursively` handle the new `JsonType`?

## Code and Tests

- ETS source: `std/core/Jsonx.ets`, `std/core/json.ets`
- Tests: `../tests/ets_func_tests/std/core/json/JsonElement*.ets`
