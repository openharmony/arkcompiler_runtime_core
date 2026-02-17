/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "include/mem/panda_containers.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/regexp/ecmascript/regexp_parser.h"
#include "runtime/include/mem/panda_string.h"
#include "ets_exceptions.h"

namespace ark::ets::intrinsics {
using RegExpParser = ark::RegExpParser;

// Helper function to check for duplicate flags and set the flag
static void CheckAndSetFlag(char flagChar, uint32_t flagMask, uint32_t &nativeFlags)
{
    if ((nativeFlags & flagMask) != 0) {
        PandaString errorMsg = "SyntaxError: Duplicate RegExp flag '";
        errorMsg += flagChar;
        errorMsg += "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), PlatformTypes()->coreSyntaxError, errorMsg);
    }
    nativeFlags |= flagMask;
}

extern "C" void StdCoreRegExpParse(EtsString *pattern, EtsString *flags)
{
    RegExpParser parse = RegExpParser();
    auto flagsStr = ark::PandaStringToStd(flags->GetUtf8());
    auto patternStr = pattern->GetUtf8();
    size_t patternLen = patternStr.length();
    const char *patternData = patternStr.data();

    // Parse string flag into integer bit mask
    uint32_t nativeFlags = 0;
    for (char c : flagsStr) {
        switch (c) {
            case 'g':
                CheckAndSetFlag('g', ark::RegExpParser::FLAG_GLOBAL, nativeFlags);
                break;
            case 'i':
                CheckAndSetFlag('i', ark::RegExpParser::FLAG_IGNORECASE, nativeFlags);
                break;
            case 'm':
                CheckAndSetFlag('m', ark::RegExpParser::FLAG_MULTILINE, nativeFlags);
                break;
            case 's':
                CheckAndSetFlag('s', ark::RegExpParser::FLAG_DOTALL, nativeFlags);
                break;
            case 'u':
                CheckAndSetFlag('u', ark::RegExpParser::FLAG_UTF16, nativeFlags);
                break;
            case 'y':
                CheckAndSetFlag('y', ark::RegExpParser::FLAG_STICKY, nativeFlags);
                break;
            case 'd':
                CheckAndSetFlag('d', ark::RegExpParser::FLAG_HASINDICES, nativeFlags);
                break;
            default:
                PandaString errorMsg = "SyntaxError: Invalid RegExp flag '";
                errorMsg += c;
                errorMsg += "'";
                ThrowEtsException(EtsCoroutine::GetCurrent(), PlatformTypes()->coreSyntaxError, errorMsg);
        }
    }

    parse.Init(const_cast<char *>(patternData), patternLen, nativeFlags);
    parse.Parse();
    if (parse.IsError()) {
        auto errormsg = ark::PandaStringToStd(parse.GetErrorMsg());
        ThrowEtsException(EtsCoroutine::GetCurrent(), PlatformTypes()->coreSyntaxError, errormsg);
    }
}

}  // namespace ark::ets::intrinsics
