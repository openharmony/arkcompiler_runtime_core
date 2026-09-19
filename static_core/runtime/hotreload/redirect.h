/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_HOTRELOAD_REDIRECT_H_
#define PANDA_RUNTIME_HOTRELOAD_REDIRECT_H_

#include <atomic>

#include "libarkbase/macros.h"

namespace ark {
class Method;
class Field;
}  // namespace ark

namespace ark::hotreload {

/**
 * @file
 * @brief Old -> new entity redirection for raw pointers that escaped the runtime.
 *
 * The commit exchanges whole `Method` / `Field` arrays instead of rewriting the objects in place
 * (see `docs/01-design.md` D6). Everything the runtime owns is repaired during the commit --- the
 * class tables, the interpreter caches, the panda caches. What cannot be repaired is a raw pointer
 * that already left the runtime:
 *
 *   - `ani_method` / `ani_static_method` / `ani_function` are a `reinterpret_cast` of `Method *`,
 *     and `ani_field` / `ani_static_field` / `ani_variable` of `Field *`. Native code is explicitly
 *     encouraged to resolve them once and cache them;
 *   - the managed reflection objects keep the same pointers in a `long` field.
 *
 * After a reload such a pointer still dereferences fine --- it names the method that now belongs to
 * the obsolete class --- so the call SILENTLY runs the old body, and a cached static `Field *`
 * silently reads and writes the obsolete class's static storage instead of the live one. That is
 * the worst possible failure mode: no error, no log, just stale behaviour.
 *
 * This table is the fix. It is consulted at the few entry points where such a pointer re-enters the
 * runtime, and it costs one relaxed atomic load on a process that has never hot reloaded.
 *
 * Properties:
 *  - **transitive**: after gen1->gen2->gen3, a gen1 pointer resolves straight to gen3;
 *  - **process-wide**: it is not tied to a class linker context, because an `ani_method` is not
 *    either;
 *  - **grow-only**, reset when the class linker extension frees the obsolete classes the entries
 *    point at (`ClassLinkerExtension::FreeObsoleteData`), which is also what makes it safe for a
 *    process that creates and destroys several virtual machines.
 */

/*
 * False until the first hotreload transaction commits, and only then does anything pay for the
 * lookup. Kept out of line from the maps deliberately: the fast path must not touch the lock.
 */
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
extern PANDA_PUBLIC_API std::atomic<bool> g_redirectActive;

ALWAYS_INLINE inline bool IsRedirectActive()
{
    // Atomic with acquire order reason: pairs with the release store in `PublishRedirects`, so a
    // thread that observes `true` also observes the fully built maps
    return g_redirectActive.load(std::memory_order_acquire);
}

PANDA_PUBLIC_API Method *ResolveMethodRedirect(Method *method);
PANDA_PUBLIC_API Field *ResolveFieldRedirect(Field *field);

/// @returns the live replacement of @a method, or @a method itself if it was never reloaded.
ALWAYS_INLINE inline Method *RedirectMethod(Method *method)
{
    if (LIKELY(!IsRedirectActive())) {
        return method;
    }
    return ResolveMethodRedirect(method);
}

/// @returns the live replacement of @a field, or @a field itself if it was never reloaded.
ALWAYS_INLINE inline Field *RedirectField(Field *field)
{
    if (LIKELY(!IsRedirectActive())) {
        return field;
    }
    return ResolveFieldRedirect(field);
}

/**
 * @brief Drop every redirection.
 *
 * Must be called when the entities the table points at are freed, i.e. together with the obsolete
 * classes that own them. Without it a second VM in the same process could hand out a `Method *` at
 * a recycled address and have it redirected into freed memory.
 */
PANDA_PUBLIC_API void ResetRedirects();

}  // namespace ark::hotreload

#endif  // PANDA_RUNTIME_HOTRELOAD_REDIRECT_H_
