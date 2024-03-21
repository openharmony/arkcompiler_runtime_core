/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SHARED_MEMORY_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SHARED_MEMORY_INL_H

#include "runtime/include/thread_scopes.h"
#include "plugins/ets/runtime/types/ets_shared_memory.h"

namespace ark::ets {

template <typename T>
T EtsSharedMemory::GetElement(uint32_t index)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    if constexpr (std::is_same_v<T, int8_t>) {
        return reinterpret_cast<EtsByteArray *>(obj)->Get(index);
    } else if constexpr (std::is_same_v<T, int16_t>) {
        return reinterpret_cast<EtsShortArray *>(obj)->Get(index);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return reinterpret_cast<EtsIntArray *>(obj)->Get(index);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        return reinterpret_cast<EtsLongArray *>(obj)->Get(index);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        return reinterpret_cast<EtsBooleanArray *>(obj)->Get(index);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        return reinterpret_cast<EtsCharArray *>(obj)->Get(index);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return reinterpret_cast<EtsUintArray *>(obj)->Get(index);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        return reinterpret_cast<EtsUlongArray *>(obj)->Get(index);
    } else {
        UNREACHABLE();
    }
}

template <typename T>
void EtsSharedMemory::SetElement(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    if constexpr (std::is_same_v<T, int8_t>) {
        reinterpret_cast<EtsByteArray *>(obj)->Set(index, element);
    } else if constexpr (std::is_same_v<T, int16_t>) {
        reinterpret_cast<EtsShortArray *>(obj)->Set(index, element);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        reinterpret_cast<EtsIntArray *>(obj)->Set(index, element);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        reinterpret_cast<EtsLongArray *>(obj)->Set(index, element);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        reinterpret_cast<EtsBooleanArray *>(obj)->Set(index, element);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        reinterpret_cast<EtsCharArray *>(obj)->Set(index, element);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        reinterpret_cast<EtsUintArray *>(obj)->Set(index, element);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        reinterpret_cast<EtsUlongArray *>(obj)->Set(index, element);
    } else {
        UNREACHABLE();
    }
}

template <typename T, typename F>
std::pair<T, T> EtsSharedMemory::ReadModifyWrite(int32_t index, const F &f)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsSharedMemory> thisHandle(coroutine, this);

    // NOTE(egor-porsev): add LIKELY(std::try_lock) path to prevent ScopedNativeCodeThread creation if no blocking
    // occurs
    ScopedNativeCodeThread n(coroutine);
    os::memory::LockHolder lock(coroutine->GetPandaVM()->GetAtomicsMutex());
    ScopedManagedCodeThread m(coroutine);

    auto oldValue = thisHandle->GetElement<T>(index);
    auto newValue = f(oldValue);
    thisHandle->SetElement<T>(index, newValue);

    return std::pair(oldValue, newValue);
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SHARED_MEMORY_INL_H
