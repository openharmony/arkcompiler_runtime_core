/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PRODUCT_BUILD

#include "runtime/compiler.h"
#include "runtime/include/thread.h"
#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"

extern "C" {
namespace ark::ets::napi {
// NOLINTNEXTLINE(readability-identifier-naming)
ETS_EXPORT ets_int ETS_CALL ETS_ETSGLOBAL_compileMethod([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_string name)
{
    if (!Runtime::GetCurrent()->IsJitEnabled()) {
        return 1;
    }
    ScopedManagedCodeFix s(PandaEtsNapiEnv::ToPandaEtsEnv(env));
    auto str = s.ToInternalType(name);
    return ark::CompileMethodImpl(str->GetCoreType(), panda_file::SourceLang::ETS);
}
}  // namespace ark::ets::napi

}  // extern "C"
#endif
