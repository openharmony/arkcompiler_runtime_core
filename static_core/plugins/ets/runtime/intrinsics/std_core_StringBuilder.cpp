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
#include "runtime/include/class.h"
#include "runtime/include/exceptions.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_string_builder.h"
#include "plugins/ets/runtime/types/ets_array.h"
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

ObjectHeader *StdCoreStringBuilderAppendString(ObjectHeader *sb, EtsString *str)
{
    return StringBuilderAppendString(sb, str);
}

ObjectHeader *StdCoreStringBuilderAppendBool(ObjectHeader *sb, EtsBoolean v)
{
    return StringBuilderAppendBool(sb, v);
}

ObjectHeader *StdCoreStringBuilderAppendChar(ObjectHeader *sb, EtsChar v)
{
    return StringBuilderAppendChar(sb, v);
}

ObjectHeader *StdCoreStringBuilderAppendLong(ObjectHeader *sb, EtsLong v)
{
    return StringBuilderAppendLong(sb, v);
}

ObjectHeader *StdCoreStringBuilderAppendByte(ObjectHeader *sb, EtsByte v)
{
    return StringBuilderAppendLong(sb, static_cast<EtsLong>(v));
}

ObjectHeader *StdCoreStringBuilderAppendShort(ObjectHeader *sb, EtsShort v)
{
    return StringBuilderAppendLong(sb, static_cast<EtsLong>(v));
}

ObjectHeader *StdCoreStringBuilderAppendInt(ObjectHeader *sb, EtsInt v)
{
    return StringBuilderAppendLong(sb, static_cast<EtsLong>(v));
}

EtsString *StdCoreStringBuilderToString(ObjectHeader *sb)
{
    return StringBuilderToString(sb);
}

}  // namespace panda::ets::intrinsics
