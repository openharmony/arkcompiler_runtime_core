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

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_arraybuffer-inl.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets::intrinsics {

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define SHARED_MEMORY_AT(inType, realType, postfix)                                                             \
    extern "C" EtsDouble ArrayBufferAt##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset) \
    {                                                                                                           \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                  \
        return mem->GetElement<realType>(index, byteOffset);                                                    \
    }

#define SHARED_MEMORY_SET(inType, realType, postfix)                                                        \
    extern "C" void ArrayBufferSet##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                            inType value)                                                   \
    {                                                                                                       \
        auto newValue = static_cast<realType>(value);                                                       \
        mem->SetElement<realType>(index, byteOffset, newValue);                                             \
    }

#define SHARED_MEMORY_ADD(inType, realType, postfix)                                                           \
    extern "C" EtsLong ArrayBufferAdd##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                               inType value)                                                   \
    {                                                                                                          \
        auto newValue = static_cast<realType>(value);                                                          \
        auto result = mem->GetAndAdd<realType>(index, byteOffset, newValue);                                   \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                 \
        return result;                                                                                         \
    }

#define SHARED_MEMORY_AND_SIGNED(inType, postfix)                                                              \
    extern "C" EtsLong ArrayBufferAnd##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                               inType value)                                                   \
    {                                                                                                          \
        auto result = mem->GetAndBitwiseAnd<inType>(index, byteOffset, value);                                 \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                 \
        return result;                                                                                         \
    }

#define SHARED_MEMORY_AND_UNSIGNED(inType, realType, postfix)                                                  \
    extern "C" EtsLong ArrayBufferAnd##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                               inType value)                                                   \
    {                                                                                                          \
        auto newValue = static_cast<realType>(value);                                                          \
        auto result = mem->GetAndBitwiseAnd<realType>(index, byteOffset, newValue);                            \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                 \
        return result;                                                                                         \
    }

#define SHARED_MEMORY_COMPARE_EXCHANGE(inType, realType, postfix)                                                      \
    extern "C" EtsLong ArrayBufferCompareExchange##postfix(                                                            \
        EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, inType expectedValue, inType replacementValue) \
    {                                                                                                                  \
        auto newExpectedValue = static_cast<realType>(expectedValue);                                                  \
        auto newReplacementValue = static_cast<realType>(replacementValue);                                            \
        auto result =                                                                                                  \
            mem->CompareAndExchangeElement<realType>(index, byteOffset, newExpectedValue, newReplacementValue, true);  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                         \
        return result.second;                                                                                          \
    }

#define SHARED_MEMORY_EXCHANGE(inType, realType, postfix)                                                           \
    extern "C" EtsLong ArrayBufferExchange##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                                    inType value)                                                   \
    {                                                                                                               \
        auto newValue = static_cast<realType>(value);                                                               \
        auto result = mem->ExchangeElement<realType>(index, byteOffset, newValue);                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                      \
        return result;                                                                                              \
    }

#define SHARED_MEMORY_LOAD(inType, realType, postfix)                                                           \
    extern "C" EtsLong ArrayBufferLoad##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset) \
    {                                                                                                           \
        auto result = mem->GetVolatileElement<realType>(index, byteOffset);                                     \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                  \
        return result;                                                                                          \
    }

// CC-OFFNXT(G.PRE.05) function gen
#define SHARED_MEMORY_OR_SIGNED(inType, postfix)                                                              \
    extern "C" EtsLong ArrayBufferOr##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                              inType value)                                                   \
    {                                                                                                         \
        auto result = mem->GetAndBitwiseOr<inType>(index, byteOffset, value);                                 \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                \
        return result;                                                                                        \
    }

#define SHARED_MEMORY_OR_UNSIGNED(inType, realType, postfix)                                                  \
    extern "C" EtsLong ArrayBufferOr##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                              inType value)                                                   \
    {                                                                                                         \
        auto newValue = static_cast<realType>(value);                                                         \
        auto result = mem->GetAndBitwiseOr<realType>(index, byteOffset, newValue);                            \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                \
        return result;                                                                                        \
    }

// CC-OFFNXT(G.PRE.06) code generation
#define SHARED_MEMORY_STORE(inType, realType, postfix)                                                           \
    extern "C" EtsLong ArrayBufferStore##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                                 inType value)                                                   \
    {                                                                                                            \
        auto newValue = static_cast<realType>(value);                                                            \
        mem->SetVolatileElement<realType>(index, byteOffset, newValue);                                          \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                   \
        return newValue;                                                                                         \
    }

#define SHARED_MEMORY_SUB(inType, realType, postfix)                                                           \
    extern "C" EtsLong ArrayBufferSub##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                               inType value)                                                   \
    {                                                                                                          \
        auto newValue = static_cast<realType>(value);                                                          \
        auto result = mem->GetAndSub<realType>(index, byteOffset, newValue);                                   \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                 \
        return result;                                                                                         \
    }

#define SHARED_MEMORY_XOR_SIGNED(inType, postfix)                                                              \
    extern "C" EtsLong ArrayBufferXor##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                               inType value)                                                   \
    {                                                                                                          \
        auto result = mem->GetAndBitwiseXor<inType>(index, byteOffset, value);                                 \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                 \
        return result;                                                                                         \
    }

#define SHARED_MEMORY_XOR_UNSIGNED(inType, realType, postfix)                                                  \
    extern "C" EtsLong ArrayBufferXor##postfix(EtsEscompatArrayBuffer *mem, int32_t index, int32_t byteOffset, \
                                               inType value)                                                   \
    {                                                                                                          \
        auto newValue = static_cast<realType>(value);                                                          \
        auto result = mem->GetAndBitwiseXor<realType>(index, byteOffset, newValue);                            \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                 \
        return result;                                                                                         \
    }

#define FOR_ALL_TYPES(ArrayBufferMethod)                                                                           \
    SHARED_MEMORY_##ArrayBufferMethod(int8_t, int8_t, I8) SHARED_MEMORY_##ArrayBufferMethod(int16_t, int16_t, I16) \
        SHARED_MEMORY_##ArrayBufferMethod(int32_t, int32_t, I32)                                                   \
            SHARED_MEMORY_##ArrayBufferMethod(int64_t, int64_t, I64)                                               \
                SHARED_MEMORY_##ArrayBufferMethod(int8_t, uint8_t, U8)                                             \
                    SHARED_MEMORY_##ArrayBufferMethod(int16_t, uint16_t, U16)                                      \
                        SHARED_MEMORY_##ArrayBufferMethod(int32_t, uint32_t, U32)                                  \
                            SHARED_MEMORY_##ArrayBufferMethod(int64_t, uint64_t, U64)

#define FOR_SIGNED_TYPES(ArrayBufferMethod)                                                       \
    SHARED_MEMORY_##ArrayBufferMethod(int8_t, I8) SHARED_MEMORY_##ArrayBufferMethod(int16_t, I16) \
        SHARED_MEMORY_##ArrayBufferMethod(int32_t, I32) SHARED_MEMORY_##ArrayBufferMethod(int64_t, I64)

#define FOR_UNSIGNED_TYPES(ArrayBufferMethod)                                                                        \
    SHARED_MEMORY_##ArrayBufferMethod(int8_t, uint8_t, U8) SHARED_MEMORY_##ArrayBufferMethod(int16_t, uint16_t, U16) \
        SHARED_MEMORY_##ArrayBufferMethod(int32_t, uint32_t, U32)                                                    \
            SHARED_MEMORY_##ArrayBufferMethod(int64_t, uint64_t, U64)

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
