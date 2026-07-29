# Interop Thread and Context Access Knowledge

This document records the thread safety rules, context access constraints, and cross-thread interaction patterns for ArkTS-Sta and ArkTS-Dyn interoperability.

## Fundamental Model

- **ArkTS-Dyn objects**: Bound to an Owner Context (thread context). All property access, function calls, and Promise continuations must execute on the Owner Context thread.
- **ArkTS-Sta objects**: Bound to process context. Support cross-thread access (shared memory model).

## Three Cross-Context Access Modes

### 1. Direct Mode
- **Condition**: Current thread is the Owner Context thread.
- **Behavior**: Synchronous direct call allowed.

### 2. Explicit Scheduling Mode
- **Condition**: Current thread is NOT the Owner Context thread.
- **Behavior**: Developer explicitly dispatches operation back to Owner Context via scheduling APIs (e.g., `EAWorker.postToMain` returning `Job<R>`); no automatic dispatch or Promise wrapping occurs.

### 3. Serialization/Deserialization Mode
- **Condition**: Developer needs cross-thread safe data transfer.
- **Behavior**: Hybrid structured serialization via `InteropSerializeHelper` / `napi_serialize_hybrid`; eligible ArkTS-Sta arguments remain direct, JS interop references represented by opaque serialized-data handle. `JSON.stringify` is a separate, lossy option for simple value transfer.

## Default Rules

- Same-thread access: synchronous execution.
- Cross-thread access: must use Explicit Scheduling or Serialization/Deserialization.
- **Wrong-thread direct access**: ESValue API rejects with `Interop object must be used in the same InteropCtx as it was created`; no automatic dispatch or Promise wrapping occurs.

## Cross-Context Access Rejection

When accessing a Dyn object via ESValue on a thread that is not its Owner Context:
- ESValue property/invocation APIs execute synchronously in the current `InteropCtx`
- `JSValue::GetNapiRef` rejects a reference created in another context with the error: `Interop object must be used in the same InteropCtx as it was created`
- Use explicit scheduling APIs (e.g., `EAWorker.postToMain`) to access Dyn objects from a different thread

## Re-entry Rules

- If Sta calls Dyn, and Dyn calls back Sta **within the same Owner Context**, and Sta then accesses the same Dyn object — this is legal re-entry.
- If the callback has switched to a different thread, access to the Dyn object must be re-dispatched.

## Sta Developer Guidelines for Dyn Object Access

1. **Same-thread access**: Use synchronous Direct API (ESValue executes in current `InteropCtx`).
2. **Cross-thread access**: Use explicit scheduling APIs (e.g., `EAWorker.postToMain` returning `Job<R>`) or hybrid structured serialization/deserialization.
3. **Prohibition**: No direct cross-thread ESValue property access or method calls on Dyn objects — runtime rejects with `Interop object must be used in the same InteropCtx as it was created`.

```typescript
// Same-thread (main thread accessing main-thread Dyn object)
let name = ESValue.wrap(stu).getProperty('name');

// Sub-thread accessing sub-thread Dyn object (via ESValue.load in worker)
let mod = ESValue.load('dynamic');
return mod.getProperty('stu').getProperty('name');

// Sub-thread dispatching back to main thread
let job = EAWorker.postToMain<string>(() => stu.name);
```

## Cross-Thread Transfer Specification

- **Bare Dyn objects**: Prohibited from direct dereferencing across Sta threads.
- **Message boundaries** (postMessage, TaskPool, worker calls): Must follow Structured Clone semantics.
- **Structured Clone**: Does not automatically change the original object's Owner Context.

## Structured Clone Rules for Dyn Objects

When Structured Clone encounters Dyn values:

1. **Normal Dyn values**: Cloned per Structured Clone semantics.
2. **Sta host objects encountered during clone**: NOT deep-copied — passed as Sta reference directly.
3. **Dyn objects holding Sta references**: Dyn object cloned per Structured Clone; Sta host fields preserved as Sta references.
4. **Sta objects transferred**: Even if they internally hold Dyn objects, passed as Sta reference directly (not recursively expanded).
5. **Indirectly carried Dyn objects**: Do not become cross-thread accessible by transfer — wrong-thread dereferencing still throws exception or requires explicit dispatch.

## Cross-Context Shared Modification

- **Single-context shared modification**: Use current context's lock mechanism.
- **Cross-context shared modification**: Locks have context isolation — no cross-boundary physical lock transfer or same-name lock creation on both sides.
- **Solution**: Unify locking logic on the side holding the shared resource; other side triggers via cross-context function call.
- **Critical section rules**: Keep lightweight; no reverse-signal wait inside lock to prevent cross-runtime deadlock.

```typescript
// Sta side defines unified lock
const lockSta = AsyncLock.request('sta_lock');
export async function safeModify(): Promise<void> {
    await lockSta.lockAsync(() => { /* shared modification */ })
}

// Dyn side triggers via cross-context call
import { safeModify } from "./Static";
async function safeModifyInDyn(): Promise<void> {
    await safeModify();
}
```

## Dyn Object Semantic in Sta Context

Dyn objects in Sta are **"dynamic-type object references belonging to an Owner Context"** — they are:
- Callable
- Property-readable (within correct thread)
- Serializable/deserializable
- **NOT** thread-safe static objects freely shared across threads

## Sta Object Semantic in Dyn Context

Sta objects in Dyn are **"special-semantic dynamic-type proxy objects"** — they are:
- Callable
- Property-readable
- Inheritance/interface semantics applied to the proxy class (not the original Sta object)
