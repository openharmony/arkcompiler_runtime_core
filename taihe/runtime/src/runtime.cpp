/*
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
#include <core/runtime.hpp>

namespace taihe::core {
__thread ani_env *cur_env;

void set_env(ani_env *env)
{
    cur_env = env;
}

ani_env *get_env()
{
    return cur_env;
}

void ani_set_error(ani_env *env, taihe::core::string_view msg)
{
    ani_class errCls;
    const char *className = "Lescompat/Error;";
    if (ANI_OK != env->FindClass(className, &errCls)) {
        std::cerr << "Not found '" << className << std::endl;
        return;
    }

    ani_method errCtor;
    if (ANI_OK != env->Class_FindMethod(errCls, "<ctor>", "Lstd/core/String;Lescompat/ErrorOptions;:V", &errCtor)) {
        std::cerr << "get errCtor Failed'" << className << "'" << std::endl;
        return;
    }

    ani_string result_string {};
    env->String_NewUTF8(msg.c_str(), msg.size(), &result_string);

    ani_object errObj;
    if (ANI_OK != env->Object_New(errCls, errCtor, &errObj, result_string)) {
        std::cerr << "Create Object Failed'" << className << "'" << std::endl;
        return;
    }
    env->ThrowError(static_cast<ani_error>(errObj));
}

void set_error(taihe::core::string_view msg)
{
    ani_env *env = get_env();
    ani_set_error(env, msg);
}
}  // namespace taihe::core
