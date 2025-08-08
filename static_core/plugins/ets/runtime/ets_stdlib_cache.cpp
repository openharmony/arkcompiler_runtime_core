/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_stdlib_cache.h"
#include "utils/logger.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

namespace ark::ets {

template <typename Type>
static void GetGlobalRef(ani_env *env, EtsClass *cls, Type *result)
{
    ani::ScopedManagedCodeFix s(env);
    ani_status status = s.AddGlobalRef(cls->AsObject(), reinterpret_cast<ani_ref *>(result));
    LOG_IF(status != ANI_OK, FATAL, ANI) << "Cannot create global reference to class: name=" << cls->GetDescriptor()
                                         << " status=" << status;
}

static void CacheMethod(ani_env *env, ani_class cls, const char *methodName, const char *signature, ani_method *result)
{
    [[maybe_unused]] auto status = env->Class_FindMethod(cls, methodName, signature, result);
    ASSERT(status == ANI_OK);
    if (*result == nullptr) {
        LOG(FATAL, ANI) << "Cannot find method: name =" << methodName;
    }
};

static void CacheVariable(ani_env *env, ani_module module, const char *varName, ani_variable *result)
{
    [[maybe_unused]] auto status = env->Module_FindVariable(module, varName, result);
    ASSERT(status == ANI_OK);
    if (*result == nullptr) {
        LOG(FATAL, ANI) << "Cannot find variable: name =" << varName;
    }
};

PandaUniquePtr<StdlibCache> CreateStdLibCache(ani_env *env)
{
    const EtsPlatformTypes *types = PlatformTypes();
    auto stdlibCache = MakePandaUnique<StdlibCache>();

    // Cache modules
    GetGlobalRef(env, types->core, &stdlibCache->std_core);

    // Cache classes
    GetGlobalRef(env, types->coreStringBuilder, &stdlibCache->std_core_String_Builder);
    GetGlobalRef(env, types->coreConsole, &stdlibCache->std_core_Console);

    // Cache methods
    CacheMethod(env, stdlibCache->std_core_Console, "error",
                "C{escompat.Array}:", &stdlibCache->std_core_Console_error);
    CacheMethod(env, stdlibCache->std_core_String_Builder, "<ctor>", ":",
                &stdlibCache->std_core_String_Builder_default_ctor);
    CacheMethod(env, stdlibCache->std_core_String_Builder, "toString", ":C{std/core/String}",
                &stdlibCache->std_core_String_Builder_toString);
    CacheMethod(env, stdlibCache->std_core_String_Builder, "append",
                "C{std.core.String}C{std.core.String}C{std.core.String}:C{std.core.StringBuilder}",
                &stdlibCache->std_core_String_Builder_append);

    // Cache variables
    CacheVariable(env, stdlibCache->std_core, "console", &stdlibCache->std_core_console);
    return stdlibCache;
}

}  // namespace ark::ets
