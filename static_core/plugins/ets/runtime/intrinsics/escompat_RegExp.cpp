/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/regexp/ecmascript/regexp_parser.h"
#include "runtime/include/mem/panda_string.h"

namespace ark::ets::intrinsics {
using RegExpParser = ark::RegExpParser;

extern "C" EtsString *EscompatRegExpParse(EtsString *pattern)
{
    RegExpParser parse = RegExpParser();
    auto patternstr = ark::PandaStringToStd(pattern->GetUtf8());
    parse.Init(const_cast<char *>(reinterpret_cast<const char *>(patternstr.c_str())), patternstr.length(), 0);
    parse.Parse();
    if (parse.IsError()) {
        auto errormsg = ark::PandaStringToStd(parse.GetErrorMsg());
        return EtsString::CreateFromMUtf8(errormsg.c_str(), errormsg.length());
    }
    return nullptr;
}

}  // namespace ark::ets::intrinsics
