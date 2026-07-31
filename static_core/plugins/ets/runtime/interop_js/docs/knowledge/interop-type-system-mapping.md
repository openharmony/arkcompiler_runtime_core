# Interop Type System Mapping Knowledge

This document records the type mapping rules between ArkTS-Sta (static) and ArkTS-Dyn (dynamic) contexts, covering primitives, utility types, standard built-in classes, functions, interfaces/classes, enums, generics, and overloads.

## Primitive Type Mapping

### ArkTS-Sta → ArkTS-Dyn

| ArkTS-Sta Type | ArkTS-Dyn Type | Notes |
|---|---|---|
| double/Double | number | — |
| float/Float | number | — |
| boolean/Boolean | boolean | — |
| char/Char | string | — |
| string/String | string | — |
| null | null | — |
| undefined | undefined | — |
| long | number | No long type in Dyn |
| int | number | No int type in Dyn |
| short | number | No short type in Dyn |
| byte | number | No byte type in Dyn |
| enum string | enum string | — |
| enum int | enum int | — |

Union types map individually then merge: `number|byte|int|long` → `number`.

### ArkTS-Dyn → ArkTS-Sta

Two-layer semantics:
- **Compile-time type**: Provides ArkTS-Sta static type checking (non-structural typing)
- **Runtime type**: `Any` type for dynamic objects, subject to thread-access constraints

| ArkTS-Dyn Type | ArkTS-Sta Type |
|---|---|
| number/Number | number |
| boolean/Boolean | boolean |
| string/String | string |
| null | null |
| undefined | undefined |
| enum string | enum string |
| enum int | enum int |

**Note:** Values exceeding `Number` safe integer range (`[-2^53+1, 2^53-1]`) suffer precision loss when crossing from Dyn to Sta.

## Utility Type Mapping

### Sta → Dyn (Supported)

| Sta Type | Dyn Type |
|---|---|
| Partial\<T\> | Partial\<T\> |
| Required\<T\> | Required\<T\> |
| Readonly\<T\> | Readonly\<T\> |
| Record\<K,T\> | Record\<K,T\> |

All other utility types (Awaited, Pick, Omit, Exclude, Extract, NonNullable, Parameters, ReturnType, etc.) are **unsupported** in Sta→Dyn mapping.

### Dyn → Sta

Supported types map identically (Partial, Required, Readonly, Record). All other utility types map to `Any`.

## Standard Built-in Class Mapping

| Direction | Sta Type | Dyn Type |
|---|---|---|
| Sta→Dyn | Array | st.Array |
| Sta→Dyn | Map | st.Map |
| Sta→Dyn | Set | st.Set |
| Sta→Dyn | Date | any |
| Sta→Dyn | bigint/BigInt | bigint |
| Dyn→Sta | Array | es.Array |
| Dyn→Sta | Map | es.Map |
| Dyn→Sta | Set | es.Set |
| Dyn→Sta | Date | Any |
| Dyn→Sta | bigint/BigInt | bigint |

ArrayBuffer, Atomics, FinalizationRegistry, IterableIterator, Iterator, IteratorResult, JSON, Math, RegExpExecArray, WeakMap, WeakSet, WeakRef are **not supported** in both directions.

## Function and Method Mapping

**Core constraints:**
- Supported entities: normal functions, class methods, getter/setter, lambda expressions
- Parameter/return types must be in supported mapping list
- No decorators/annotations on interop functions
- No complex syntactic sugar by default

### Sta → Dyn

| Category | Sta Type | Dyn Type |
|---|---|---|
| Normal function | `foo(arg: Klass, len: number): int` | `foo(arg: Klass, len: number): number` |
| Default param | `foo(arg: number = 1): void` | `foo(arg: number = 1): void` |
| Optional param | `foo(arg?: number): void` | `foo(arg: number|undefined): void` |
| Rest param | `foo(...args: number[]): number[]` | `foo(...args: number[]): st.Array<number>` |
| Getter/setter | `get name(): string` | `get name(): string` |
| Function with receiver | `foo(this: Klass, arg: number): void` | **Unsupported** |
| Lambda | `let foo: Any = (arg: string) => arg` | `let foo: ESObject` |

**Note:** Sta numeric types (byte, short, int, long, float, double) are passed as `number` on Dyn side with automatic narrowing conversion at runtime.

### Dyn → Sta

| Category | Dyn Type | Sta Type |
|---|---|---|
| Normal function | `foo(arg: Klass, len: number): Result` | `foo(arg: Klass, len: number): Result` |
| Default param | `foo(arg: number = 1): void` | `foo(arg: number): void` (default removed) |
| Optional param | `foo(arg?: number): void` | `foo(arg: number|undefined): void` |
| Rest param | `foo(...args: number[]): number[]` | `foo(...args: number[]): es.Array<number>` |
| Function with receiver | **Unsupported** | — |

## Interface and Class Mapping

**Sta classes expose two proxy types in Dyn:**
- **ArkTS-Sta-Class**: Class proxy — supports `new`, static member access, static method calls
- **ArkTS-Sta-Object**: Object instance proxy — supports property access, member method calls

**Key rules:**
- Sta instance fields → Dyn getter/setter methods
- Sta static fields → Dyn getter/setter methods
- Dyn fields → Sta fields (both instance and static)
- Cross-context inheritance/interface implementation is **unsupported**
- Private fields and methods are **unsupported**
- Override must follow Sta covariant/contravariant rules

**Warning:** Sta classes should not declare static properties or methods named `name`, `length`, or `prototype` — they conflict with reserved Dyn class proxy names and prevent creation of the Dyn class proxy with an ETS exception.

## Enum Mapping

| Direction | Category | Mapping |
|---|---|---|
| Sta→Dyn | Regular enum (all same type) | Preserved unchanged |
| Sta→Dyn | Heterogeneous enum | Unsupported |
| Sta→Dyn | Const enum | Unsupported |
| Dyn→Sta | Regular enum | Preserved unchanged |
| Dyn→Sta | Heterogeneous enum | Initializer removed; user must convert type with `as` |
| Dyn→Sta | Const enum | Unsupported |

## Generic Mapping

**Core principle:** "Compile-time preserves checking, runtime erases and dispatches, bridge-impacting generics must be explicitly specified."

### Sta → Dyn

| Category | Sta | Dyn |
|---|---|---|
| Generic function | `foo<T>(arg: T): void` | `foo<T>(arg: T): void` |
| Generic default | `foo<T = number>(arg: T): void` | `foo<T = number>(arg: T): void` |
| Generic constraint | `foo<T extends Klass>(arg: T): void` | `foo<T extends Klass>(arg: T): void` |
| Generic class | `class GClass<T>{}` | `class GClass<T>{}` |
| Contravariant class | `class Producer<in T>{}` | `class Producer<in T>{}` |
| Covariant class | `class Consumer<out T>{}` | `class Consumer<out T>{}` |
| keyof constraint | `class G<K extends keyof keys>{}` | `class G<K extends Any>{}` |
| Generic constraint keyof | `class G<T, K extends keyof T>{}` | Unsupported |
| Generic interface | `interface GInterface<T>{}` | Unsupported |

### Dyn → Sta

Same as above, except:
- Generic constraint keyof → `class G<K extends Union>` (not `Any`)
- Generic lambda → `foo = (arg: Any) => arg`

## Overload Mapping

### Sta Overloads → Dyn Context

- **First-Fit strategy**: Overloads matched by definition order; returns first successful match
- **Numeric narrowing priority**: byte → short → int → float → double
- **Unsupported**: Any, Array, Promise, Function in overload matching; union types, default/optional/rest params in overloads
- **Note**: `undefined` matches Object-type parameter; extra dispatch overhead exists vs. non-overloaded calls

### Dyn Overloads → Sta Context

- Supported by both parameter-count and parameter-type differences
- Compile-time dispatch decision; Dyn side only executes single function logic
- Sta overloaded signatures map to the same Dyn function via Declgen

### Sta Overloads → Dyn (Supported Table)

| Overload Type | Normal Method | Constructor | Static Method | Instance Method |
|---|---|---|---|---|
| Different param count | Unsupported | Supported | Supported | Supported |
| Different param types | Unsupported | Supported | Supported | Supported |

## Logical Operations on Cross-Context Objects

| Category | Operators | Operand Type Requirement |
|---|---|---|
| Arithmetic | +, -, *, /, %, +=, -=, *=, /=, %=, ++, -- | number |
| Bitwise | &, \|, ~, ^, &=, \|=, ^=, <<, >>, >>>, <<=, >>=, >>>= | number |
| Logical | &&, \|\|, !, &&=, \|\|= | boolean |
| Comparison | >, <, >=, <= | number |
| Equality | ==, ===, !=, !== | unrestricted |
