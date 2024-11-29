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
#include "IntlCommon.h"

namespace ark::ets::stdlib::intl {

std::string EtsToStdStr(EtsEnv *env, ets_string etsStr)
{
    const char *charString = env->GetStringUTFChars(etsStr, nullptr);
    std::string stdStr {charString};
    env->ReleaseStringUTFChars(etsStr, charString);
    return stdStr;
}

icu::UnicodeString EtsToUnicodeStr(EtsEnv *env, ets_string etsStr)
{
    auto str = icu::StringPiece(EtsToStdStr(env, etsStr).c_str());
    return icu::UnicodeString::fromUTF8(str);
}

}  // namespace ark::ets::stdlib::intl