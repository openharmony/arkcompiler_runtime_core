/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_CONVERTERS_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_CONVERTERS_H

#include "ani.h"
#include "plugins/ets/runtime/ani/ani_type_check.h"
#include "runtime/hotreload/redirect.h"

namespace ark::ets::ani {

/**
 * @brief Translate a raw handle that may predate a hot reload.
 *
 * `ani_method` and friends are a `reinterpret_cast` of `EtsMethod *`, which is a `Method *`, and
 * `ani_field` of `EtsField *`, which is a `Field *` --- so a handle a native module resolved once
 * and cached is a raw pointer into a method or field array. Hot reload EXCHANGES those arrays, so
 * after a reload the cached handle still points at a perfectly valid object that now belongs to the
 * obsolete class: calling it runs the old body, and reading a static field through it reads the
 * obsolete class's storage. Nothing reports an error --- this is the fix for that.
 *
 * ANI itself is unchanged: the redirection happens on the way in, so a caller keeps using the
 * handle it was given and gets the live entity. On a process that has never hot reloaded the whole
 * thing is one relaxed atomic load.
 *
 * The kind of the entity survives the swap --- a reload is refused unless every method matches by
 * name, prototype and access flags and every field by name, type and access flags --- so the
 * assertions below still hold for the entity the handle is redirected to.
 */
/*
 * WHERE these are called matters as much as that they are called.
 *
 * The translation answers "which entity is current NOW", and the answer expires: another reload
 * makes the result stale exactly as the caller's cached handle was stale. So the translation has to
 * sit after the last point at which this thread can be suspended for a commit, not before it.
 *
 * A NATIVE-state thread is not stopped by stop-the-world, but its transition into managed state is
 * a guaranteed suspension point --- it blocks for the duration of the commit and resumes on the
 * other side. Translating before that transition therefore hands the caller a pointer that was
 * current when it was produced and obsolete when it was used, and for a static field that means
 * reading and writing the obsolete class's own storage, which nothing else looks at. Every ANI
 * entry point translates AFTER entering managed state for that reason.
 *
 * Entering managed state is the last suspension point in four of the six field accessors, where the
 * only things between the translation and the use are a type read and a handle dereference. It is
 * NOT the last one in `ClassGetStaticField` / `ClassSetStaticField`: `InitializeClass` sits between
 * them and runs the class's `<cctor>`, which is arbitrary managed code and polls. What protects
 * those two is a different property, and it is worth naming rather than assuming: a class being
 * initialized is `INITIALIZING`, and `QuiesceCheck` refuses the whole transaction while any target
 * class is in that state, so the commit that would invalidate the handle cannot commit there. Two
 * further properties make the residue harmless --- a reload rewrites the `Class` object in place, so
 * the `EtsClass *` read before the call stays live, and field offsets are pinned equal across
 * generations by `SameFieldStorage`. Reordering the initialization or relaxing that gate reopens
 * this; the ordering alone does not close it.
 */
ALWAYS_INLINE inline EtsMethod *RedirectAcrossHotreload(EtsMethod *entity)
{
    return reinterpret_cast<EtsMethod *>(::ark::hotreload::RedirectMethod(reinterpret_cast<Method *>(entity)));
}

ALWAYS_INLINE inline EtsField *RedirectAcrossHotreload(EtsField *entity)
{
    return reinterpret_cast<EtsField *>(::ark::hotreload::RedirectField(reinterpret_cast<Field *>(entity)));
}

inline ani_field ToAniField(EtsField *field)
{
    ASSERT(field != nullptr);
    ASSERT(IsInstanceField(field));
    return reinterpret_cast<ani_field>(field);
}

inline EtsField *ToInternalField(ani_field field)
{
    auto *f = RedirectAcrossHotreload(reinterpret_cast<EtsField *>(field));
    ASSERT(f != nullptr);
    ASSERT(IsInstanceField(f));
    return f;
}

inline ani_static_field ToAniStaticField(EtsField *field)
{
    ASSERT(field != nullptr);
    ASSERT(IsStaticField(field));
    return reinterpret_cast<ani_static_field>(field);
}

inline EtsField *ToInternalField(ani_static_field field)
{
    auto *f = RedirectAcrossHotreload(reinterpret_cast<EtsField *>(field));
    ASSERT(f != nullptr);
    ASSERT(IsStaticField(f));
    return f;
}

inline ani_variable ToAniVariable(EtsField *variable)
{
    ASSERT(variable != nullptr);
    ASSERT(IsVariable(variable));
    return reinterpret_cast<ani_variable>(variable);
}

inline EtsField *ToInternalField(ani_variable variable)
{
    auto *v = RedirectAcrossHotreload(reinterpret_cast<EtsField *>(variable));
    ASSERT(v != nullptr);
    ASSERT(IsVariable(v));
    return v;
}

inline EtsMethod *ToInternalMethod(ani_method method)
{
    auto *m = RedirectAcrossHotreload(reinterpret_cast<EtsMethod *>(method));
    ASSERT(m != nullptr);
    ASSERT(IsInstanceMethod(m));
    return m;
}

inline ani_method ToAniMethod(EtsMethod *method)
{
    ASSERT(method != nullptr);
    ASSERT(IsInstanceMethod(method));
    return reinterpret_cast<ani_method>(method);
}

inline EtsMethod *ToInternalMethod(ani_static_method method)
{
    auto *m = RedirectAcrossHotreload(reinterpret_cast<EtsMethod *>(method));
    ASSERT(m != nullptr);
    ASSERT(IsStaticMethod(m));
    return m;
}

inline ani_static_method ToAniStaticMethod(EtsMethod *method)
{
    ASSERT(method != nullptr);
    ASSERT(IsStaticMethod(method));
    return reinterpret_cast<ani_static_method>(method);
}

inline EtsMethod *ToInternalMethod(ani_function fn)
{
    auto *m = RedirectAcrossHotreload(reinterpret_cast<EtsMethod *>(fn));
    ASSERT(m != nullptr);
    ASSERT(IsFunction(m));
    return m;
}

inline ani_function ToAniFunction(EtsMethod *method)
{
    ASSERT(method != nullptr);
    ASSERT(IsFunction(method));
    return reinterpret_cast<ani_function>(method);
}

}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_ANI_CONVERTERS_H
