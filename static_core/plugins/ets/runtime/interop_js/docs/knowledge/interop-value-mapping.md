# Interop Type Mapping and Value Conversion Knowledge

This document records the value conversion logic between ArkTS and JS, and the design rules for the `ESValue`/`STValue` system.

## Value Mapping Matrix

| ETS Type | JS Type | Converter / Path | Description |
| --- | --- | --- | --- |
| `boolean` | `boolean` | `JSConvertU1` | Direct conversion |
| `number` / `double` | `number` | `JSConvertF64` | Mapped to double-precision float |
| `string` | `string` | `JSConvertString` | Involves UTF-8/UTF-16 conversion and cache optimization |
| `bigint` | `bigint` | `JSConvertBigInt` | Array bit disassembly conversion |
| `Array` | `Array` | `JSRefConvertArray` | Wrapped as a proxy or copied on demand |
| `EtsObject` | `Object` | `JSRefConvertEtsProxy` | Generates a JS-side proxy object |
| `Promise` | `Promise` | `JSConvertPromise` | Asynchronous state machine linkage |

## Core Systems

### 1. JSValue (Runtime Representation)
A unified representation of JS values within the ETS runtime (`js_value.h`).
- Uses the `type_` field to identify `napi_valuetype`.
- Uses the `data_` field to store the raw value or a `SharedReference` pointer.
- **Principle**: A `Finalizer` must be attached to manage the lifecycle of non-primitive types (e.g., String, Object).

### 2. ESValue (ETS-to-JS)
API for ArkTS-Sta to manipulate dynamic objects.
- Encapsulates property reads/writes (`getProperty`/`setProperty`) and function calls (`invoke`) on JS objects.
- Supports strong type conversions such as `toBoolean` and `toNumber`.

### 3. STValue (JS-to-ETS)
API for ArkTS-Dyn to access static modules.
- **Mangling Rules**: Uses the `parameter_types:return_type` format to identify overloads (e.g., `i:C{std.core.String}`).
- **SType Enum**: Explicitly specifies the target static type (INT, LONG, REFERENCE, etc.).

## Conversion Principles

- **Object Access Safety**: Prohibit direct access to `EtsObject` via raw pointers. All ETS objects MUST be wrapped in `EtsHandle` (or `EtsPersistentHandle`) to prevent null-pointer access and ensure GC-safe reference tracking.
- **Exception Handling Protocol**: In scenarios where exceptions may be thrown, pre-check for `pendingError`. If an unhandled exception (e.g., in the NAPI env) exists, it must be processed or propagated before proceeding with further operations.
- **Minimal Intrusion**: Prioritize using templated basic converters like `JSConvertNumeric`.
- **NULL/Undefined Handling**: 
    - JS `undefined` maps to `nullptr` in ETS (for reference types) or a specific wrapper value.
    - JS `null` maps to `NullValue` in ETS.
- **Exception Safety**: Conversion failures must call `ForwardJSException` or `ForwardEtsException`.
- **Lazy Loading**: Meta-data (`ConstStringStorage`) is loaded only upon the first access to a class or namespace.

## Test Guidelines

- **Primitive Type Tests**: `js_value_test.cpp`
- **Complex Reference Tests**: Array, object, and function conversion scenarios in `ets_interop_js_gtests`.
- **API Compatibility**: `ESValueApi_en.md`, `stvalue_en.md`.
