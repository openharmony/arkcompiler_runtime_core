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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SHARED_MEMORY_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SHARED_MEMORY_INL_H

#include "runtime/include/thread_scopes.h"
#include "plugins/ets/runtime/types/ets_shared_memory.h"

namespace ark::ets {

// CC-OFFNXT(G.PRE.02, G.PRE.06) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE(methodName, type, obj, ...)        \
    do {                                                                       \
        if constexpr (std::is_same_v<type, int8_t>) {                          \
            reinterpret_cast<EtsByteArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, int16_t>) {                  \
            reinterpret_cast<EtsShortArray *>(obj)->methodName(__VA_ARGS__);   \
        } else if constexpr (std::is_same_v<type, int32_t>) {                  \
            reinterpret_cast<EtsIntArray *>(obj)->methodName(__VA_ARGS__);     \
        } else if constexpr (std::is_same_v<type, int64_t>) {                  \
            reinterpret_cast<EtsLongArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint8_t>) {                  \
            reinterpret_cast<EtsBooleanArray *>(obj)->methodName(__VA_ARGS__); \
        } else if constexpr (std::is_same_v<type, uint16_t>) {                 \
            reinterpret_cast<EtsCharArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint32_t>) {                 \
            reinterpret_cast<EtsUintArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint64_t>) {                 \
            reinterpret_cast<EtsUlongArray *>(obj)->methodName(__VA_ARGS__);   \
        } else {                                                               \
            UNREACHABLE();                                                     \
        }                                                                      \
    } while (false)

// CC-OFFNXT(G.PRE.02, G.PRE.06) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EXECUTE_METHOD_DEPENDS_ON_TYPE(varToStore, methodName, type, obj, ...)              \
    do {                                                                                    \
        if constexpr (std::is_same_v<type, int8_t>) {                                       \
            varToStore = reinterpret_cast<EtsByteArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, int16_t>) {                               \
            varToStore = reinterpret_cast<EtsShortArray *>(obj)->methodName(__VA_ARGS__);   \
        } else if constexpr (std::is_same_v<type, int32_t>) {                               \
            varToStore = reinterpret_cast<EtsIntArray *>(obj)->methodName(__VA_ARGS__);     \
        } else if constexpr (std::is_same_v<type, int64_t>) {                               \
            varToStore = reinterpret_cast<EtsLongArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint8_t>) {                               \
            varToStore = reinterpret_cast<EtsBooleanArray *>(obj)->methodName(__VA_ARGS__); \
        } else if constexpr (std::is_same_v<type, uint16_t>) {                              \
            varToStore = reinterpret_cast<EtsCharArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint32_t>) {                              \
            varToStore = reinterpret_cast<EtsUintArray *>(obj)->methodName(__VA_ARGS__);    \
        } else if constexpr (std::is_same_v<type, uint64_t>) {                              \
            varToStore = reinterpret_cast<EtsUlongArray *>(obj)->methodName(__VA_ARGS__);   \
        } else {                                                                            \
            UNREACHABLE();                                                                  \
        }                                                                                   \
    } while (false)

template <typename T>
T EtsSharedMemory::GetElement(uint32_t index)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, Get, T, obj, index);
    return val;
}

template <typename T>
void EtsSharedMemory::SetElement(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE(SetVolatile, T, obj, index, element);
}

template <typename T>
T EtsSharedMemory::GetVolatileElement(uint32_t index)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetVolatile, T, obj, index);
    return val;
}

template <typename T>
void EtsSharedMemory::SetVolatileElement(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE(SetVolatile, T, obj, index, element);
}

template <typename T>
std::pair<bool, T> EtsSharedMemory::CompareAndExchangeElement(uint32_t index, T oldElement, T newElement, bool strong)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    std::pair<bool, T> val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, CompareAndExchange, T, obj, index, oldElement, newElement, strong);
    return val;
}
template <typename T>
T EtsSharedMemory::ExchangeElement(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, Exchange, T, obj, index, element);
    return val;
}

template <typename T>
T EtsSharedMemory::GetAndAdd(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndAdd, T, obj, index, element);
    return val;
}

template <typename T>
T EtsSharedMemory::GetAndSub(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndSub, T, obj, index, element);
    return val;
}

template <typename T>
T EtsSharedMemory::GetAndBitwiseOr(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndBitwiseXor, T, obj, index, element);
    return val;
}

template <typename T>
T EtsSharedMemory::GetAndBitwiseAnd(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndBitwiseAnd, T, obj, index, element);
    return val;
}

template <typename T>
T EtsSharedMemory::GetAndBitwiseXor(uint32_t index, T element)
{
    ASSERT_PRINT(index < GetLength(), "SharedMemory index out of bounds");
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *obj = ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_));
    // CC-OFFNXT(G.EXP.09): var is used as macros parameter
    T val;
    EXECUTE_METHOD_DEPENDS_ON_TYPE(val, GetAndBitwiseXor, T, obj, index, element);
    return val;
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

#undef EXECUTE_VOID_METHOD_DEPENDS_ON_TYPE
#undef EXECUTE_METHOD_DEPENDS_ON_TYPE

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SHARED_MEMORY_INL_H
