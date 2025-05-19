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

#ifndef PANDA_PLUGINS_ETS_SDK_NATIVE_CORE_UTIL_H
#define PANDA_PLUGINS_ETS_SDK_NATIVE_CORE_UTIL_H

#include <ani.h>

extern "C" {
namespace ark::ets::sdk::util {

ANI_EXPORT ani_string ETSApiUtilHelperGenerateRandomUUID(ani_env *env, [[maybe_unused]] ani_class klass,
                                                         ani_boolean entropyCache);

ANI_EXPORT void ThrowNewError(ani_env *env, std::string_view classDescriptor, std::string_view msg,
                              const char *ctorSignature = nullptr);

ANI_EXPORT ani_string CreateUtf8String(ani_env *env, const char *data, ani_size size);
ANI_EXPORT ani_string CreateUtf16String(ani_env *env, const uint16_t *data, ani_size size);
}  // namespace ark::ets::sdk::util
}

#endif  //  PANDA_PLUGINS_ETS_SDK_NATIVE_CORE_UTIL_H
