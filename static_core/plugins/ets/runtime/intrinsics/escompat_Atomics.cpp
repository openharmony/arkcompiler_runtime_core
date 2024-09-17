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

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include "libpandabase/macros.h"
#include "libpandabase/os/mutex.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_shared_memory.h"
#include "plugins/ets/runtime/types/ets_shared_memory-inl.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets::intrinsics {

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define SHARED_MEMORY_AT(type, postfix)                                          \
    extern "C" type SharedMemoryAt##postfix(EtsSharedMemory *mem, int32_t index) \
    {                                                                            \
        /* CC-OFFNXT(G.PRE.05) function gen */                                   \
        return mem->GetElement<type>(index);                                     \
    }

#define SHARED_MEMORY_SET(type, postfix)                                                      \
    extern "C" void SharedMemorySet##postfix(EtsSharedMemory *mem, int32_t index, type value) \
    {                                                                                         \
        mem->SetElement<type>(index, value);                                                  \
    }

#define SHARED_MEMORY_ADD(type, postfix)                                                      \
    extern "C" type SharedMemoryAdd##postfix(EtsSharedMemory *mem, int32_t index, type value) \
    {                                                                                         \
        auto add = [value](type oldValue) { return oldValue + value; };                       \
        auto result = mem->ReadModifyWrite<type>(index, add);                                 \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                \
        return result.first;                                                                  \
    }

#define SHARED_MEMORY_AND_SIGNED(type, postfix)                                                   \
    extern "C" type SharedMemoryAnd##postfix(EtsSharedMemory *mem, int32_t index, type value)     \
    {                                                                                             \
        auto bitwiseAnd = [value](type oldValue) {                                                \
            /* CC-OFFNXT(G.PRE.05) function gen */                                                \
            return static_cast<type>((bit_cast<u##type>(oldValue)) & (bit_cast<u##type>(value))); \
        };                                                                                        \
        auto result = mem->ReadModifyWrite<type>(index, bitwiseAnd);                              \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                    \
        return result.first;                                                                      \
    }

#define SHARED_MEMORY_AND_UNSIGNED(type, postfix)                                                     \
    extern "C" type SharedMemoryAnd##postfix(EtsSharedMemory *mem, int32_t index, type value)         \
    {                                                                                                 \
        auto bitwiseAnd = [value](type oldValue) { return static_cast<type>((oldValue) & (value)); }; \
        auto result = mem->ReadModifyWrite<type>(index, bitwiseAnd);                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                        \
        return result.first;                                                                          \
    }

#define SHARED_MEMORY_COMPARE_EXCHANGE(type, postfix)                                                             \
    extern "C" type SharedMemoryCompareExchange##postfix(EtsSharedMemory *mem, int32_t index, type expectedValue, \
                                                         type replacementValue)                                   \
    {                                                                                                             \
        auto compareExchange = [expectedValue, replacementValue](type oldValue) {                                 \
            /* CC-OFFNXT(G.PRE.05) function gen */                                                                \
            return oldValue == expectedValue ? replacementValue : oldValue;                                       \
        };                                                                                                        \
        auto result = mem->ReadModifyWrite<type>(index, compareExchange);                                         \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                    \
        return result.first;                                                                                      \
    }

#define SHARED_MEMORY_EXCHANGE(type, postfix)                                                      \
    extern "C" type SharedMemoryExchange##postfix(EtsSharedMemory *mem, int32_t index, type value) \
    {                                                                                              \
        auto exchange = [value]([[maybe_unused]] type oldValue) { return value; };                 \
        auto result = mem->ReadModifyWrite<type>(index, exchange);                                 \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                     \
        return result.first;                                                                       \
    }

#define SHARED_MEMORY_LOAD(type, postfix)                                          \
    extern "C" type SharedMemoryLoad##postfix(EtsSharedMemory *mem, int32_t index) \
    {                                                                              \
        auto load = [](type value) { return value; };                              \
        auto result = mem->ReadModifyWrite<type>(index, load);                     \
        /* CC-OFFNXT(G.PRE.05) function gen */                                     \
        return result.first;                                                       \
    }

// CC-OFFNXT(G.PRE.05) function gen
#define SHARED_MEMORY_OR_SIGNED(type, postfix)                                                    \
    extern "C" type SharedMemoryOr##postfix(EtsSharedMemory *mem, int32_t index, type value)      \
    {                                                                                             \
        auto orBitwise = [value](type oldValue) {                                                 \
            /* CC-OFFNXT(G.PRE.05) function gen */                                                \
            return static_cast<type>((bit_cast<u##type>(oldValue)) | (bit_cast<u##type>(value))); \
        };                                                                                        \
        auto result = mem->ReadModifyWrite<type>(index, orBitwise);                               \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                    \
        return result.first;                                                                      \
    }

#define SHARED_MEMORY_OR_UNSIGNED(type, postfix)                                                     \
    extern "C" type SharedMemoryOr##postfix(EtsSharedMemory *mem, int32_t index, type value)         \
    {                                                                                                \
        auto orBitwise = [value](type oldValue) { return static_cast<type>((oldValue) | (value)); }; \
        auto result = mem->ReadModifyWrite<type>(index, orBitwise);                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                       \
        return result.first;                                                                         \
    }

#define SHARED_MEMORY_STORE(type, postfix)                                                      \
    extern "C" type SharedMemoryStore##postfix(EtsSharedMemory *mem, int32_t index, type value) \
    {                                                                                           \
        auto store = [value]([[maybe_unused]] type oldValue) { return value; };                 \
        auto result = mem->ReadModifyWrite<type>(index, store);                                 \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                  \
        return result.second;                                                                   \
    }

#define SHARED_MEMORY_SUB(type, postfix)                                                      \
    extern "C" type SharedMemorySub##postfix(EtsSharedMemory *mem, int32_t index, type value) \
    {                                                                                         \
        auto add = [value](type oldValue) { return oldValue - value; };                       \
        auto result = mem->ReadModifyWrite<type>(index, add);                                 \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                \
        return result.first;                                                                  \
    }

#define SHARED_MEMORY_XOR_SIGNED(type, postfix)                                                   \
    extern "C" type SharedMemoryXor##postfix(EtsSharedMemory *mem, int32_t index, type value)     \
    {                                                                                             \
        auto xorBitwise = [value](type oldValue) {                                                \
            /* CC-OFFNXT(G.PRE.05) function gen */                                                \
            return static_cast<type>((bit_cast<u##type>(oldValue)) ^ (bit_cast<u##type>(value))); \
        };                                                                                        \
        auto result = mem->ReadModifyWrite<type>(index, xorBitwise);                              \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                    \
        return result.first;                                                                      \
    }

#define SHARED_MEMORY_XOR_UNSIGNED(type, postfix)                                                     \
    extern "C" type SharedMemoryXor##postfix(EtsSharedMemory *mem, int32_t index, type value)         \
    {                                                                                                 \
        auto xorBitwise = [value](type oldValue) { return static_cast<type>((oldValue) ^ (value)); }; \
        auto result = mem->ReadModifyWrite<type>(index, xorBitwise);                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                        \
        return result.first;                                                                          \
    }

#define FOR_ALL_TYPES(sharedMemoryMethod)                                                                     \
    SHARED_MEMORY_##sharedMemoryMethod(int8_t, I8) SHARED_MEMORY_##sharedMemoryMethod(int16_t, I16)           \
        SHARED_MEMORY_##sharedMemoryMethod(int32_t, I32) SHARED_MEMORY_##sharedMemoryMethod(int64_t, I64)     \
            SHARED_MEMORY_##sharedMemoryMethod(uint8_t, U8) SHARED_MEMORY_##sharedMemoryMethod(uint16_t, U16) \
                SHARED_MEMORY_##sharedMemoryMethod(uint32_t, U32) SHARED_MEMORY_##sharedMemoryMethod(uint64_t, U64)

#define FOR_SIGNED_TYPES(sharedMemoryMethod)                                                        \
    SHARED_MEMORY_##sharedMemoryMethod(int8_t, I8) SHARED_MEMORY_##sharedMemoryMethod(int16_t, I16) \
        SHARED_MEMORY_##sharedMemoryMethod(int32_t, I32) SHARED_MEMORY_##sharedMemoryMethod(int64_t, I64)

#define FOR_UNSIGNED_TYPES(sharedMemoryMethod)                                                        \
    SHARED_MEMORY_##sharedMemoryMethod(uint8_t, U8) SHARED_MEMORY_##sharedMemoryMethod(uint16_t, U16) \
        SHARED_MEMORY_##sharedMemoryMethod(uint32_t, U32) SHARED_MEMORY_##sharedMemoryMethod(uint64_t, U64)

// NOLINTEND(cppcoreguidelines-macro-usage)

FOR_ALL_TYPES(AT)
FOR_ALL_TYPES(SET)
FOR_ALL_TYPES(ADD)
FOR_ALL_TYPES(COMPARE_EXCHANGE)
FOR_ALL_TYPES(EXCHANGE)
FOR_ALL_TYPES(LOAD)
FOR_ALL_TYPES(STORE)
FOR_ALL_TYPES(SUB)
FOR_SIGNED_TYPES(OR_SIGNED)
FOR_UNSIGNED_TYPES(OR_UNSIGNED)
FOR_SIGNED_TYPES(AND_SIGNED)
FOR_UNSIGNED_TYPES(AND_UNSIGNED)
FOR_SIGNED_TYPES(XOR_SIGNED)
FOR_UNSIGNED_TYPES(XOR_UNSIGNED)

extern "C" EtsSharedMemory *SharedMemoryCreate(int32_t byteLength)
{
    return EtsSharedMemory::Create(byteLength);
}

extern "C" int32_t SharedMemoryGetByteLength(EtsSharedMemory *mem)
{
    return static_cast<int32_t>(mem->GetLength());
}

std::string PrintWaiters(EtsSharedMemory &mem)
{
    std::stringstream stream;
    auto curWaiter = mem.GetHeadWaiter();
    while (curWaiter != nullptr) {
        stream << reinterpret_cast<size_t>(curWaiter) << " -> ";
        curWaiter = curWaiter->GetNext();
    }
    return stream.str();
}

extern "C" int32_t SharedMemoryWaitI32(EtsSharedMemory *mem, int32_t byteOffset, int32_t expectedValue)
{
    auto result = mem->WaitI32(byteOffset, expectedValue, std::nullopt);
    return static_cast<int32_t>(result);
}

extern "C" int32_t SharedMemoryWaitI64(EtsSharedMemory *mem, int32_t byteOffset, int64_t expectedValue)
{
    auto result = mem->WaitI64(byteOffset, expectedValue, std::nullopt);
    return static_cast<int32_t>(result);
}

extern "C" int32_t SharedMemoryTimedWaitI32(EtsSharedMemory *mem, int32_t byteOffset, int32_t expectedValue, int64_t ms)
{
    ASSERT(ms >= 0);
    auto uMs = static_cast<uint64_t>(ms);
    auto result = mem->WaitI32(byteOffset, expectedValue, std::optional(uMs));
    return static_cast<int32_t>(result);
}

extern "C" int32_t SharedMemoryTimedWaitI64(EtsSharedMemory *mem, int32_t byteOffset, int64_t expectedValue, int64_t ms)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    EtsHandle<EtsSharedMemory> hmem(currentCoro, mem);

    ASSERT(ms >= 0);
    auto uMs = static_cast<uint64_t>(ms);
    ScopedNativeCodeThread n(currentCoro);

    auto result = hmem->WaitI64(byteOffset, expectedValue, std::optional(uMs));
    return static_cast<int32_t>(result);
}

extern "C" int32_t SharedMemoryNotify(EtsSharedMemory *mem, int32_t byteOffset)
{
    return mem->NotifyI32(byteOffset, std::nullopt);
}

extern "C" int32_t SharedMemoryBoundedNotify(EtsSharedMemory *mem, int32_t byteOffset, int32_t count)
{
    ASSERT(count >= 0);
    return mem->NotifyI32(byteOffset, std::optional(count));
}

#undef SHARED_MEMORY_AT
#undef SHARED_MEMORY_SET
#undef SHARED_MEMORY_ADD
#undef SHARED_MEMORY_AND_SIGNED
#undef SHARED_MEMORY_AND_UNSIGNED
#undef SHARED_MEMORY_COMPARE_EXCHANGE
#undef SHARED_MEMORY_EXCHANGE
#undef SHARED_MEMORY_LOAD
#undef SHARED_MEMORY_OR_SIGNED
#undef SHARED_MEMORY_OR_UNSIGNED
#undef SHARED_MEMORY_STORE
#undef SHARED_MEMORY_SUB
#undef SHARED_MEMORY_XOR_SIGNED
#undef SHARED_MEMORY_XOR_UNSIGNED
#undef FOR_ALL_TYPES
#undef FOR_SIGNED_TYPES
#undef FOR_UNSIGNED_TYPES

}  // namespace ark::ets::intrinsics
