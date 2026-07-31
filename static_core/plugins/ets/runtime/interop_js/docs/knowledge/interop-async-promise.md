# Interop Async Behavior and Promise Knowledge

This document records the specification rules for asynchronous interaction between ArkTS-Sta and ArkTS-Dyn via Promise.

## Interaction Scenarios

| Scenario | Direction | Call Method | Supported Callable Types |
|---|---|---|---|
| S1 | Sta calls Dyn | Direct module import | Promise instance, function returning Promise, async public instance/static methods |
| S2 | Sta calls Dyn | Via ESValue | Same as S1 |
| S3 | Dyn calls Sta | Direct module import | Same as S1 |

All interop APIs related to async behavior follow the respective scenario's rules unless explicitly stated otherwise.

## Core Principle

ArkTS-Sta's `Promise<T>` and ArkTS-Dyn's `Promise<T>` are **peer interop objects** — async semantics are preserved by default.

## Promise Instance Methods

| API | Support |
|---|---|
| `promise.then(onFulfilled[, onRejected])` | Normal form: `onFulfilled(value)` receives resolved value; `onRejected` optional, receives Error callback |
| `promise.then()` / `promise.then(undefined)` | Pass-through: no handler registered, resolves with same value or rejects with same reason |
| `promise.catch(onRejected)` | Optional; receives Error-type or no-arg callback |
| `promise.finally(onFinally)` | Optional; receives no-arg callback |

## Promise Static Methods

| API | Support |
|---|---|
| `Promise.reject()` | Only Error-type parameter |
| `Promise.resolve()` | Accepts cross-context Promise (Sta→Dyn or Dyn→Sta) |
| `Promise.all()` | Same cross-context support as resolve |
| `Promise.any()` | Same cross-context support as resolve |
| `Promise.race()` | Same cross-context support as resolve |
| `Promise.allSettled()` | **Unsupported** |

## async/await

- Supported: `await` cross-context Promise in both Dyn async functions (awaiting Sta Promise) and Sta async functions (awaiting Dyn Promise)

## ESValue.toPromise() (S2 Scenario Only)

Converts a cross-context Dyn object to a `Promise<ESValue>`:

```typescript
let dynPromise = ESValue.load(...).getProperty(...);
let promise = dynPromise.toPromise();
```

## Promise\<T\> Generic Support

Generic `T` supports: interop basic types, Array, Map, Set.

## Cross-Thread Promise Rules

| Scenario | Support |
|---|---|
| Sta sub-thread calling Dyn Promise | Only S2 scenario: via ESValue in sub-thread |
| Dyn sub-thread calling Sta Promise | **Unsupported** — no cross-thread Promise calls between Dyn threads or main/sub threads |

## Supported Dynamic Promise Operations for toPromise

| Type | Example | Behavior |
|---|---|---|
| Constructor | `new Promise((res, rej) => {})` | Pending on Sta side until Dyn resolves |
| resolve | `Promise.resolve(value)` | Awaiting yields ESValue wrapping value |
| reject | `Promise.reject(reason)` | Maps to exception, must catch via try-catch |
| allSettled | `Promise.allSettled([p1, p2])` | Awaiting yields ESValue array |
| then | `Promise.then(val => {})` | Chain result obtainable via toPromise |
