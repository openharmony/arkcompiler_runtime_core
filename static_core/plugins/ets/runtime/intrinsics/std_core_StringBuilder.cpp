/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "intrinsics.h"
#include "libpandabase/utils/logger.h"
#include "runtime/include/exceptions.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "libpandabase/utils/utf.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "runtime/arch/memory_helpers.h"
#include <unistd.h>

namespace panda::ets::intrinsics {

EtsString *StdCoreStringBuilderConcatStrings(EtsString *lhs, EtsString *rhs)
{
    ASSERT(lhs != nullptr && rhs != nullptr);

    return EtsString::Concat(lhs, rhs);
}

EtsString *StdCoreToStringBoolean(EtsBoolean i)
{
    std::string s = i == 1 ? "true" : "false";
    return EtsString::CreateFromMUtf8(s.c_str());
}

EtsString *StdCoreToStringByte(EtsByte i)
{
    std::string s = std::to_string(i);
    return EtsString::CreateFromMUtf8(s.c_str());
}

EtsString *StdCoreToStringChar(EtsChar i)
{
    return EtsString::CreateFromUtf16(&i, 1);
}

EtsString *StdCoreToStringShort(EtsShort i)
{
    std::string s = std::to_string(i);
    return EtsString::CreateFromMUtf8(s.c_str());
}

EtsString *StdCoreToStringInt(EtsInt i)
{
    std::string s = std::to_string(i);
    return EtsString::CreateFromMUtf8(s.c_str());
}

EtsString *StdCoreToStringLong(EtsLong i)
{
    std::string s = std::to_string(i);
    return EtsString::CreateFromMUtf8(s.c_str());
}

EtsString *StdCoreToStringFloat(EtsFloat i)
{
    std::stringstream ss;
    ss << i;
    return EtsString::CreateFromMUtf8(ss.str().c_str());
}

EtsString *StdCoreToStringDouble(EtsDouble i)
{
    std::stringstream ss;
    ss << i;
    return EtsString::CreateFromMUtf8(ss.str().c_str());
}

ObjectHeader *StdCoreStringBuilderBool(ObjectHeader *sb, EtsBoolean v)
{
    return CoreStringBuilderBool(sb, v);
}

ObjectHeader *StdCoreStringBuilderChar(ObjectHeader *sb, EtsChar c)
{
    return CoreStringBuilderChar(sb, c);
}

ObjectHeader *StdCoreStringBuilderInt(ObjectHeader *sb, int32_t n)
{
    return CoreStringBuilderInt(sb, n);
}

ObjectHeader *StdCoreStringBuilderLong(ObjectHeader *sb, int64_t n)
{
    return CoreStringBuilderLong(sb, n);
}

ObjectHeader *StdCoreStringBuilderString(ObjectHeader *sb, EtsString *s)
{
    return CoreStringBuilderString(sb, s);
}

}  // namespace panda::ets::intrinsics
