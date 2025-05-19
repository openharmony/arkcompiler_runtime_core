/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/stdlib/native/init_native_methods.h"
#include "plugins/ets/stdlib/native/escompat/Process.h"
#include "plugins/ets/stdlib/native/core/Intl.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"

namespace ark::ets::stdlib {
void InitNativeMethods(ani_env *env)
{
    RegisterProcessNativeMethods(env);
    // NOTE (ikorobkov): EtsNapiOnLoad needs to implement issue #18135
    ANI_FATAL_IF_ERROR(intl::InitCoreIntl(env));
}

}  // namespace ark::ets::stdlib
