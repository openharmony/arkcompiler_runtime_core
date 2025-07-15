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

#include "regexp.h"

#include "regexp_8.h"
#include "regexp_16.h"
#include "plugins/ets/runtime/ets_exceptions.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

namespace ark::ets {

bool EtsRegExp::Compile(const PandaVector<uint8_t> &pattern, const bool isUtf16, const int len)
{
    utf16_ |= isUtf16;
    uint32_t flags = 0U;
    if (flagMultiline_) {
        flags |= PCRE2_MULTILINE;
    }
    if (flagCaseInsentitive_) {
        flags |= PCRE2_CASELESS;
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
    uint32_t extraFlags = 0U;
    extraFlags |= PCRE2_EXTRA_ALT_BSUX;
    utf16_ |= flagUnicode_;
    if (utf16_) {
        re_ = RegExp16::CreatePcre2Object(reinterpret_cast<const uint16_t *>(pattern.data()), flags, extraFlags, len);
    } else {
        re_ = RegExp8::CreatePcre2Object(pattern.data(), flags, extraFlags, len);
    }
    return re_ != nullptr;
}

RegExpExecResult EtsRegExp::Execute(const PandaVector<uint8_t> &str, const int len, const int startOffset)
{
    RegExpExecResult result;
    if (utf16_) {
        result = RegExp16::Execute(re_, reinterpret_cast<const uint16_t *>(str.data()), len, startOffset);
    } else {
        result = RegExp8::Execute(re_, str.data(), len, startOffset);
    }

    return result;
}

void EtsRegExp::Destroy()
{
    if (utf16_) {
        RegExp16::FreePcre2Object(re_);
    } else {
        RegExp8::FreePcre2Object(re_);
    }
}

void EtsRegExp::SetUnicodeFlag(const char &chr)
{
    if (chr == 'u') {
        if (flagUnicode_ || flagVnicode_) {
            ThrowBadFlagsException();
        }
        flagUnicode_ = true;
    } else if (chr == 'v') {
        if (flagVnicode_ || flagUnicode_) {
            ThrowBadFlagsException();
        }
        flagVnicode_ = true;
    }
}

void EtsRegExp::ThrowBadFlagsException()
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ThrowEtsException(coroutine, panda_file_items::class_descriptors::UNSUPPORTED_OPERATION_EXCEPTION,
                      "invalid regular expression flags");
}

void EtsRegExp::SetIfNotSet(bool &flag)
{
    if (flag) {
        ThrowBadFlagsException();
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

void EtsRegExp::SetFlags(EtsString *flagsStr)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsString> flagsHandle(coroutine, flagsStr->GetCoreType());
    ASSERT(flagsHandle.GetPtr() != nullptr);
    for (int i = 0; i < flagsHandle.GetPtr()->GetLength(); i++) {
        SetFlag(flagsHandle.GetPtr()->At(i));
    }
    utf16_ |= flagUnicode_;
}

}  // namespace ark::ets