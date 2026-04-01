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

#include "regexp_executor.h"

#include "regexp_8.h"
#include "regexp_16.h"

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

namespace ark::ets::stdlib {

bool EtsRegExp::Compile(const uint8_t *pattern, int len)
{
    if (re8_ != nullptr) {
        return true;
    }
    re8_ = RegExp8::CreatePcre2Object(pattern, GetCompileFlags(), GetExtraCompileFlags(), len);
    return re8_ != nullptr;
}

bool EtsRegExp::Compile(const uint16_t *pattern, int len)
{
    if (re16_ != nullptr) {
        return true;
    }
    re16_ = RegExp16::CreatePcre2Object(pattern, GetCompileFlags(), GetExtraCompileFlags(), len);
    return re16_ != nullptr;
}

void EtsRegExp::Destroy()
{
    if (re8_ != nullptr) {
        RegExp8::FreePcre2Object(re8_);
        re8_ = nullptr;
    }
    if (re16_ != nullptr) {
        RegExp16::FreePcre2Object(re16_);
        re16_ = nullptr;
    }
}

uint32_t EtsRegExp::GetCompileFlags() const
{
    uint32_t flags = 0U;
    if (flagMultiline_) {
        flags |= PCRE2_MULTILINE;
    }
    if (flagCaseInsentitive_) {
        flags |= PCRE2_CASELESS;
        flags |= PCRE2_UCP;
    }
    if (flagSticky_) {
        flags |= PCRE2_ANCHORED;
    }
    if (flagDotAll_) {
        flags |= PCRE2_DOTALL;
    }
    if (flagUnicode_) {
        flags |= PCRE2_UTF;
    }
    if (flagVnicode_) {
        flags |= PCRE2_UCP;
    }
    flags |= PCRE2_MATCH_UNSET_BACKREF;
    flags |= PCRE2_ALLOW_EMPTY_CLASS;
    return flags;
}

uint32_t EtsRegExp::GetExtraCompileFlags()
{
    uint32_t extraFlags = 0U;
    extraFlags |= PCRE2_EXTRA_ALT_BSUX;
    return extraFlags;
}

void EtsRegExp::SetUnicodeFlag(const char &chr)
{
    if (chr == 'u') {
        if (flagUnicode_ || flagVnicode_) {
            ThrowBadFlagsException(env_);
        }
        flagUnicode_ = true;
    } else if (chr == 'v') {
        if (flagVnicode_ || flagUnicode_) {
            ThrowBadFlagsException(env_);
        }
        flagVnicode_ = true;
    }
}

void EtsRegExp::ThrowBadFlagsException(ani_env *env)
{
    ThrowNewError(env, "std.core.RuntimeError", "invalid regular expression flags");
}

void EtsRegExp::SetIfNotSet(bool &flag)
{
    if (flag) {
        ThrowBadFlagsException(env_);
    }
    flag = true;
}

void EtsRegExp::SetFlag(const char &chr)
{
    switch (chr) {
        case 'g':
            SetIfNotSet(flagGlobal_);
            break;
        case 'm':
            SetIfNotSet(flagMultiline_);
            break;
        case 'i':
            SetIfNotSet(flagCaseInsentitive_);
            break;
        case 'y':
            SetIfNotSet(flagSticky_);
            break;
        case 's':
            SetIfNotSet(flagDotAll_);
            break;
        case 'd':
            SetIfNotSet(flagIndices_);
            break;
        default: {
            SetUnicodeFlag(chr);
        }
    }
}

void EtsRegExp::SetFlags(const std::string &flagsStr)
{
    for (const auto &c : flagsStr) {
        SetFlag(c);
    }
}

}  // namespace ark::ets::stdlib
