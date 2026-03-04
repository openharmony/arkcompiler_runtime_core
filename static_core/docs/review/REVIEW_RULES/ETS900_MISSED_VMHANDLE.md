# Raw managed object pointer used across GC point without VMHandle

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when a raw pointer to a managed heap object (`ObjectHeader *`, `EtsObject *`,
`EtsString *`, `EtsArray *`, `EtsObjectArray *`, or any other GC-managed type) is stored in a local
variable or field and then used **after an explicit GC point** (i.e. another heap allocation,
`ThrowException`, `ClassLinker` operations, `Method::Invoke`, etc.).

GC in moving mode (G1, CMC) relocates objects at allocation points (safepoints). A raw pointer
obtained before a GC point becomes dangling after the collection because the GC only updates
pointers registered through `HandleScope` / `VMHandle` / `EtsHandle`. Raw C++ local variables are
invisible to the GC.

**The necessary condition for a violation is:**
> A heap allocation (safepoint) occurs **between** obtaining the raw pointer and its next use.

Without an intervening allocation there is no safepoint and no violation.

**This rule applies to all native C++ code that works with managed heap objects.**
For ETS plugin code use `EtsHandle<T>` from `plugins/ets/runtime/ets_handle.h` with
`EtsHandleScope`. For core runtime code (`static_core/runtime/`) use `VMHandle<T>` with
`HandleScope<ObjectHeader *>`.

**Common GC-triggering operations (safepoints):**
- Any heap allocation: `EtsObjectArray::Create`, `EtsString::CreateFromMUtf8`, `EtsObject::Create`,
  `EtsReflectMethod::CreateFromEtsMethod`, `EtsReflectField::CreateFromEtsField`, `String::Concat`,
  `coretypes::Array::Create`, etc.
- `ThrowException` / `ThrowNullPointerException` / `ThrowOutOfMemoryError`
- `ClassLinker::GetClass` / `ClassLinker::ResolveMethod` (may allocate on first call)
- `Method::Invoke` — executes managed code which may allocate

**Not violations:**
- Pointer is created and used **before** any subsequent allocation in the same scope
- Pointer is immediately returned without any allocation between creation and return
- Object is **provably allocated in non-movable space** (e.g. humongous/large-object region,
  pinned allocation). GC does not relocate such objects, so the raw pointer remains valid across
  safepoints. The non-movable property must be explicitly documented at the call site (comment
  or `ASSERT`) so reviewers can verify it without tracing the allocator path.

## Example BAD code

**Pattern 1: array created before a loop that allocates on each iteration.**

The correct version of this function is
[`CreateEtsReflectMethodArray`](static_core/plugins/ets/runtime/intrinsics/std_core_Class.cpp#L228)
in `std_core_Class.cpp`; without the handle it would be:

```cpp
// BAD: arrayRaw may be stale after each CreateFromEtsMethod call inside the loop
[[maybe_unused]] EtsHandleScope scope(coro);
auto *arrayRaw = EtsObjectArray::Create(klass, methods.size());  // safepoint 1
if (UNLIKELY(arrayRaw == nullptr)) { ... }

for (size_t idx = 0; idx < methods.size(); ++idx) {
    auto *method = EtsReflectMethod::CreateFromEtsMethod(coro, methods[idx]);  // safepoint 2 — arrayRaw stale!
    arrayRaw->Set(idx, method->AsObject());  // use of potentially dangling pointer
}
return arrayRaw->AsObject()->GetCoreType();
```

**Pattern 2: input pointer used after an allocation.**

The correct version is
[`StringConcat3`](static_core/plugins/ets/runtime/intrinsics/std_core_String.cpp#L422)
in `std_core_String.cpp`; without the handle it would be:

```cpp
// BAD: raw str3 used after String::Concat allocates the intermediate result
auto str = coretypes::String::Concat(str1, str2, ctx, vm);  // safepoint — str3 stale!
if (UNLIKELY(str == nullptr)) { HandlePendingException(); UNREACHABLE(); }
str = coretypes::String::Concat(str, str3, ctx, vm);  // str3 is a dangling pointer
```

## Example GOOD code

**Pattern 1 — fixed** ([`CreateEtsReflectMethodArray`](static_core/plugins/ets/runtime/intrinsics/std_core_Class.cpp#L228)):

```cpp
// GOOD: allocation result immediately wrapped in EtsHandle before the loop
[[maybe_unused]] EtsHandleScope scope(coro);

EtsHandle<EtsObjectArray> arrayH(coro, EtsObjectArray::Create(klass, methods.size()));  // wrap immediately
if (UNLIKELY(arrayH.GetPtr() == nullptr)) { ... }

for (size_t idx = 0; idx < methods.size(); ++idx) {
    auto *method = EtsReflectMethod::CreateFromEtsMethod(coro, methods[idx]);  // safepoint — arrayH updated by GC
    arrayH->Set(idx, method->AsObject());  // safe: operator-> dereferences current address
}
return arrayH->AsObject()->GetCoreType();
```

**Pattern 2 — fixed** ([`StringConcat3`](static_core/plugins/ets/runtime/intrinsics/std_core_String.cpp#L422)):

```cpp
// GOOD: str3 wrapped in VMHandle before the first allocation that could invalidate it
[[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
VMHandle<coretypes::String> str3Handle(coroutine, str3);  // register with GC before safepoint

auto str = coretypes::String::Concat(str1, str2, ctx, vm);  // safepoint — str3Handle updated
if (UNLIKELY(str == nullptr)) { HandlePendingException(); UNREACHABLE(); }
str = coretypes::String::Concat(str, str3Handle.GetPtr(), ctx, vm);  // always current pointer
```

## Fix suggestion

1. Add `[[maybe_unused]] EtsHandleScope scope(coro);` at the top of the function (or the
   innermost scope that contains both the allocation and the subsequent use).
2. Wrap the raw pointer **immediately** after it is obtained, before any allocation:
   ```cpp
   // Before (BAD):
   auto *arr = EtsObjectArray::Create(klass, n);
   ... SomeAllocation(); ...  // safepoint
   arr->Set(i, ...);          // dangling

   // After (GOOD):
   EtsHandle<EtsObjectArray> arr(coro, EtsObjectArray::Create(klass, n));
   ... SomeAllocation(); ...  // safepoint — arr updated by GC
   arr->Set(i, ...);          // safe
   ```
3. Replace all uses of the raw pointer after the safepoint with `handle.GetPtr()` or `handle->`.
4. For mutable handles (pointer reassigned after e.g. `String::Flatten`), use
   `VMMutableHandle<T>` and call `handle.Update(newPtr)` instead of reassigning the raw pointer.
5. For language-agnostic runtime code (`static_core/runtime/`) use `VMHandle<T>` with
   `HandleScope<ObjectHeader *>` instead of `EtsHandle<T>` / `EtsHandleScope`.

## Message

Raw managed-object pointer `{name}` of type `{type}` is used after allocation at line `{gcLine}`.
Wrap it in `EtsHandle<T>` (with an enclosing `EtsHandleScope`) before the allocation to keep the
reference valid across the GC safepoint.
