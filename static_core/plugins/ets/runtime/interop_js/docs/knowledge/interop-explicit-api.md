# Interop Explicit API (ESValue/STValue) Knowledge

This document records the design rules and usage patterns for the explicit interop APIs — ESValue (Sta operating Dyn) and STValue (Dyn operating Sta).

## Overview

| API | Context | Purpose |
|---|---|---|
| **ESValue** | ArkTS-Sta | Wrapper for operating Dyn objects — dynamic operations like getProperty, setProperty, invoke, instantiate |
| **STValue** | ArkTS-Dyn | Wrapper for accessing Sta modules — type-safe operations with SType enumeration and Mangling signatures |

## ESValue (Sta → Dyn)

### Setup
- No import needed; directly available in Sta context (files with `'use static'` header)
- For same-module Dyn imports, configure `runtimeOnly.sources` in `build-profile.json5`

### StaticOrESValue
Union type: `ESValue | Object | null | undefined` — used in `setProperty`, `invoke`, etc.

### Key API Categories

| Category | Key APIs |
|---|---|
| **Wrap primitives** | wrapBoolean, wrapString, wrapNumber, wrapBigInt, wrapByte, wrapShort, wrapInt, wrapLong, wrapFloat, wrapDouble, wrap |
| **Unwrap** | unwrap, toBoolean, toString, toNumber, toBigInt, toUndefined, toNull, toStaticObject |
| **Type check** | isBoolean, isString, isNumber, isBigInt, isUndefined, isNull, isStaticObject, isECMAObject, isObject, isFunction, isPromise |
| **Equality** | areEqual (abstract), areStrictlyEqual (strict), isEqualTo, isStrictlyEqualTo |
| **Object ops** | getProperty (string/number/ESValue), setProperty (string/number/ESValue), hasProperty, hasOwnProperty, keys, values, entries |
| **Function ops** | invoke, invokeWithRecv, invokeMethod |
| **Instance ops** | instantiate, instantiateEmptyObject, instantiateEmptyArray |
| **Module ops** | load (sdkModule or module+bundleName), getGlobal |
| **Async ops** | isPromise, toPromise |
| **Iteration** | $_iterator |

### ESValue.load Usage

```typescript
let module = ESValue.load('dynHar/src/main/ets/pages/dynModule', 'com.example.esvalue');
let jsObjectA = module.getProperty('A');
```

### toPromise

```typescript
let dynPromise = ESValue.load(...).getProperty('getPromiseNumber');
let p = dynPromise.invoke().toPromise();
let resESValue = await p;
let res = resESValue.toNumber();
```

## STValue (Dyn → Sta)

### Setup

1. Create `config.json`:
```json
{ "static.@ohos.lang.interop": { "originalAPIName": "@ohos.lang.interop", "isStatic": true } }
```
2. Add to `build-profile.json5` buildOptions: `arkOptions.sdkAliasConfigPath: "./config.json"`
3. Import: `import {STValue, SType} from "static.@ohos.lang.interop"`

### SType Enumeration

| Enum | Sta Type |
|---|---|
| BOOLEAN | boolean |
| CHAR | char |
| BYTE | byte |
| SHORT | short |
| INT | int |
| LONG | long |
| FLOAT | float |
| DOUBLE | double/number |
| REFERENCE | reference types (String, Array, class instances) |
| VOID | void |

**SType usage note**: SType must match the actual type. For `enumGetValueByName(name, valueType)`, if the enum value is int, pass `SType.INT`. Reference types always use `SType.REFERENCE`.

### Mangling (Name Modifier) Rules

Format: `parameter_types:return_type`

| Type | Mangling |
|---|---|
| boolean | z |
| byte | b |
| char | c |
| short | s |
| int | i |
| long | l |
| float | f |
| double | d |
| number | d |
| string | C{std.core.String} |
| bigint | C{std.core.BigInt} |
| Array/int[] | C{std.core.Array} |
| FixedArray\<int\> | A{i} |
| null | C{std.core.Null} |
| undefined (standalone type) | U |
| void (return type) | No encoding after `:` |

**Rules:**
- `:` separates params and return type (e.g., `zz:i` = `(boolean, boolean): int`); void return omits trailing type (e.g., `zz:` = `(boolean, boolean): void`)
- Object format: `C{<module>.<class>}`; default module = filename
- Array: `A{elementType}`; multi-dimensional: `A{A{i}}`
- Generic → type constraint substituted; default = `C{std.core.Object}`
- Union → `X{constituent1 constituent2 ...}` with constituents ordered alphabetically by ANI encoding; `undefined` removed from union constituents; value types promoted to boxed primitives
- Optional primitive → boxed object (e.g., `arg?: int` → `C{std.core.Int}`)
- Function param → `C{std.core.FunctionN}` where N = required param count; rest params → `C{std.core.FunctionRN}`

### Key API Categories

| Category | Key APIs |
|---|---|
| **Accessor** | findClass, findNamespace, findEnum, classGetSuperClass, fixedArrayGetLength, enumGetIndexByName, enumGetValueByName, classGetStaticField, classSetStaticField, objectGetProperty, objectSetProperty, fixedArrayGet, fixedArraySet, namespaceGetVariable, namespaceSetVariable, objectGetType, arrayGet, arraySet, arrayPush, arrayPop, arrayGetLength |
| **Check** | isNull, isUndefined, isNumber, isFloat, isLong, isInt, isShort, isChar, isByte, isBoolean, isBigInt, isString |
| **Instance** | classInstantiate, objectInstanceOf, objectGetType |
| **Invoke** | namespaceInvokeFunction, objectInvokeMethod, classInvokeStaticMethod |
| **Wrap** | wrapBoolean, wrapString, wrapInt, wrapNumber, wrapFloat, wrapLong, wrapShort, wrapByte, wrapChar, wrapBigInt, getNull, getUndefined |
| **Unwrap** | unwrapToString, unwrapToNumber, unwrapToBoolean |

### Error Handling

STValue APIs follow strict error conventions:
- **Wrong parameter count**: Compile-time error `Invalid parameter count`
- **Invalid parameter type**: Runtime error `Invalid parameter type`
- **Invalid instance object**: Runtime error `Illegal instance object`
- **Lookup failure**: Runtime error `Lookup failed`
- **Type mismatch**: Runtime error `Mismatched field/property/element/variable type`
- **Not found**: Runtime error `Property/Field/Variable/Enumeration does not exist`
