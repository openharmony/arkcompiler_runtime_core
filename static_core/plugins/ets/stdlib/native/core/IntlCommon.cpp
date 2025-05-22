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
#include "libpandabase/macros.h"
#include "stdlib_ani_helpers.h"
#include <cassert>

namespace ark::ets::stdlib::intl {

ani_string StdStrToAni(ani_env *env, const std::string &str)
{
    return CreateUtf8String(env, str.data(), str.size());
}

ani_string UnicodeToAniStr(ani_env *env, icu::UnicodeString &ustr)
{
    return CreateUtf16String(env, reinterpret_cast<const uint16_t *>(ustr.getBuffer()), ustr.length());
}

icu::UnicodeString AniToUnicodeStr(ani_env *env, ani_string aniStr)
{
    auto str = icu::StringPiece(ConvertFromAniString(env, aniStr));
    return icu::UnicodeString::fromUTF8(str);
}

}  // namespace ark::ets::stdlib::intl
