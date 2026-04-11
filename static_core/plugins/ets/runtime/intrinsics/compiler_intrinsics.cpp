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

#include "plugins/ets/runtime/ets_execution_context.h"
#include "include/object_header.h"
#include "intrinsics.h"

#include "plugins/ets/runtime/ets_exceptions.h"
#include "runtime/include/exceptions.h"
#include "compiler/optimizer/ir/constants.h"

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
namespace ark::ets::intrinsics {

extern "C" uint8_t CompilerEtsEquals(ObjectHeader *obj1, ObjectHeader *obj2)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return static_cast<uint8_t>(
        EtsReferenceEquals(executionCtx, EtsObject::FromCoreType(obj1), EtsObject::FromCoreType(obj2)));
}

extern "C" uint8_t CompilerEtsStrictEquals(ObjectHeader *obj1, ObjectHeader *obj2)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return static_cast<uint8_t>(
        EtsReferenceEquals<true>(executionCtx, EtsObject::FromCoreType(obj1), EtsObject::FromCoreType(obj2)));
}

extern "C" EtsString *CompilerEtsTypeof(ObjectHeader *obj)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return EtsReferenceTypeof(executionCtx, EtsObject::FromCoreType(obj));
}

extern "C" uint8_t CompilerEtsIstrue(ObjectHeader *obj)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return static_cast<uint8_t>(EtsIstrue(executionCtx, EtsObject::FromCoreType(obj)));
}

extern "C" void CompilerEtsNullcheck(ObjectHeader *obj)
{
    if (UNLIKELY(obj == nullptr)) {
        ThrowNullPointerException();
    }
}

extern "C" EtsString *CompilerLongToStringDecimal(int64_t number, [[maybe_unused]] uint64_t unused)
{
    auto *cache = ManagedThread::GetCurrent()->GetLongToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return LongToStringCache::GetNoCache(number);
    }
    return LongToStringCache::FromCoreType(cache)->GetOrCache(EtsExecutionContext::GetCurrent(), number);
}

extern "C" EtsString *CompilerFloatToStringDecimal(uint32_t number, [[maybe_unused]] uint64_t unused)
{
    auto *cache = ManagedThread::GetCurrent()->GetFloatToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return FloatToStringCache::GetNoCache(bit_cast<float>(number));
    }
    return FloatToStringCache::FromCoreType(cache)->GetOrCache(EtsExecutionContext::GetCurrent(),
                                                               bit_cast<float>(number));
}

extern "C" EtsString *CompilerDoubleToStringDecimal(uint64_t number, [[maybe_unused]] uint64_t unused)
{
    auto *cache = ManagedThread::GetCurrent()->GetDoubleToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return DoubleToStringCache::GetNoCache(bit_cast<double>(number));
    }
    return DoubleToStringCache::FromCoreType(cache)->GetOrCache(EtsExecutionContext::GetCurrent(),
                                                                bit_cast<double>(number));
}

}  // namespace ark::ets::intrinsics
