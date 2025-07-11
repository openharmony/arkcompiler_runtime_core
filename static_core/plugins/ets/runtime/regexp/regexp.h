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

#ifndef PANDA_RUNTIME_REGEXP_H
#define PANDA_RUNTIME_REGEXP_H

#include "plugins/ets/runtime/regexp/regexp_executor.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

class pcre2_code;

class EtsRegExp {
public:
    void SetFlags(EtsString *flagsStr);
    bool Compile(const PandaVector<uint8_t> &pattern, const bool isUtf16, const int len);
    RegExpExecResult Execute(const PandaVector<uint8_t> &str, const int len, const int startOffset);
    void Destroy();

    bool IsUtf16() const
    {
        return utf16_;
    }

private:
    void SetFlag(const char &chr);
    void SetUnicodeFlag(const char &chr);
    void SetIfNotSet(bool &flag);
    void ThrowBadFlagsException();

    void *re_ = nullptr;
    bool flagGlobal_ = false;           // g
    bool flagMultiline_ = false;        // m
    bool flagCaseInsentitive_ = false;  // i
    bool flagSticky_ = false;           // y
    bool flagUnicode_ = false;          // u
    bool flagVnicode_ = false;          // v
    bool flagDotAll_ = false;           // s
    bool flagIndices_ = false;          // d
    bool utf16_ = false;
};

}  // namespace ark::ets
#endif  // PANDA_RUNTIME_REGEXP_H