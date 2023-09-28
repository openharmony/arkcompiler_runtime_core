/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/types/ets_void.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace panda::ets::intrinsics {

extern "C" EtsSharedMemory *SharedMemoryCreate(int32_t byte_length)
{
    return EtsSharedMemory::Create(byte_length);
}

extern "C" int8_t SharedMemoryAt(EtsSharedMemory *mem, int32_t index)
{
    return mem->GetElement(index);
}

extern "C" EtsVoid *SharedMemorySet(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    mem->SetElement(index, value);
    return EtsVoid::GetInstance();
}

extern "C" int32_t SharedMemoryGetByteLength(EtsSharedMemory *mem)
{
    return static_cast<int32_t>(mem->GetLength());
}

extern "C" int8_t SharedMemoryAddI8(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    auto add = [value](int8_t old_value) { return old_value + value; };
    auto result = mem->ReadModifyWriteI8(index, add);
    return result.first;
}

extern "C" int8_t SharedMemoryAndI8(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    auto bitwise_and = [value](int8_t old_value) {
        return static_cast<int8_t>(bit_cast<uint8_t>(old_value) & bit_cast<uint8_t>(value));
    };
    auto result = mem->ReadModifyWriteI8(index, bitwise_and);
    return result.first;
}

extern "C" int8_t SharedMemoryCompareExchangeI8(EtsSharedMemory *mem, int32_t index, int8_t expected_value,
                                                int8_t replacement_value)
{
    auto compare_exchange = [expected_value, replacement_value](int8_t old_value) {
        return old_value == expected_value ? replacement_value : old_value;
    };
    auto result = mem->ReadModifyWriteI8(index, compare_exchange);
    return result.first;
}

extern "C" int8_t SharedMemoryExchangeI8(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    auto exchange = [value]([[maybe_unused]] int8_t old_value) { return value; };
    auto result = mem->ReadModifyWriteI8(index, exchange);
    return result.first;
}

extern "C" int8_t SharedMemoryLoadI8(EtsSharedMemory *mem, int32_t index)
{
    auto load = [](int8_t value) { return value; };
    auto result = mem->ReadModifyWriteI8(index, load);
    return result.first;
}

extern "C" int8_t SharedMemoryOrI8(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    auto or_bitwise = [value](int8_t old_value) {
        return static_cast<int8_t>(bit_cast<uint8_t>(old_value) | bit_cast<uint8_t>(value));
    };
    auto result = mem->ReadModifyWriteI8(index, or_bitwise);
    return result.first;
}

extern "C" int8_t SharedMemoryStoreI8(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    auto store = [value]([[maybe_unused]] int8_t old_value) { return value; };
    auto result = mem->ReadModifyWriteI8(index, store);
    return result.second;
}

extern "C" int8_t SharedMemorySubI8(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    auto add = [value](int8_t old_value) { return old_value - value; };
    auto result = mem->ReadModifyWriteI8(index, add);
    return result.first;
}

extern "C" int8_t SharedMemoryXorI8(EtsSharedMemory *mem, int32_t index, int8_t value)
{
    auto xor_bitwise = [value](int8_t old_value) {
        return static_cast<int8_t>(bit_cast<uint8_t>(old_value) ^ bit_cast<uint8_t>(value));
    };
    auto result = mem->ReadModifyWriteI8(index, xor_bitwise);
    return result.first;
}

std::string PrintWaiters(EtsSharedMemory &mem)
{
    std::stringstream stream;
    auto cur_waiter = mem.GetHeadWaiter();
    while (cur_waiter != nullptr) {
        stream << reinterpret_cast<size_t>(cur_waiter) << " -> ";
        cur_waiter = cur_waiter->GetNext();
    }
    return stream.str();
}

extern "C" int32_t SharedMemoryWaitI32(EtsSharedMemory *mem, int32_t byte_offset, int32_t expected_value)
{
    auto result = mem->WaitI32(byte_offset, expected_value, std::nullopt);
    return static_cast<int32_t>(result);
}

extern "C" int32_t SharedMemoryTimedWaitI32(EtsSharedMemory *mem, int32_t byte_offset, int32_t expected_value,
                                            int64_t ms)
{
    ASSERT(ms >= 0);
    auto u_ms = static_cast<uint64_t>(ms);
    auto result = mem->WaitI32(byte_offset, expected_value, std::optional(u_ms));
    return static_cast<int32_t>(result);
}

extern "C" int32_t SharedMemoryNotify(EtsSharedMemory *mem, int32_t byte_offset)
{
    return mem->NotifyI32(byte_offset, std::nullopt);
}

extern "C" int32_t SharedMemoryBoundedNotify(EtsSharedMemory *mem, int32_t byte_offset, int32_t count)
{
    ASSERT(count >= 0);
    return mem->NotifyI32(byte_offset, std::optional(count));
}

}  // namespace panda::ets::intrinsics
