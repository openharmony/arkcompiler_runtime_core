/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <optional>
#include "libpandabase/macros.h"
#include "libpandabase/utils/bit_utils.h"
#include "mem/vm_handle.h"
#include "runtime/include/runtime.h"
#include "runtime/include/object_header.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/mem/panda_containers.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_shared_memory.h"
#include "plugins/ets/runtime/types/ets_shared_memory-inl.h"

namespace ark::ets {

bool EtsSharedMemory::Waiter::Wait(std::optional<uint64_t> timeout)
{
    auto mutex = &EtsCoroutine::GetCurrent()->GetPandaVM()->GetAtomicsMutex();
    if (timeout.has_value()) {
        auto ms = timeout.value();
        return cv_.TimedWait(mutex, ms);
    }

    cv_.Wait(mutex);
    return false;
}

void EtsSharedMemory::Waiter::SignalAll()
{
    cv_.SignalAll();
}

EtsSharedMemory *EtsSharedMemory::Create(size_t length)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);

    auto cls = currentCoro->GetPandaVM()->GetClassLinker()->GetSharedMemoryClass();
    // Note: This object must be non-movable since the 'waiter_' pointer is shared between different threads
    EtsHandle<EtsSharedMemory> hmem(currentCoro, EtsSharedMemory::FromEtsObject(EtsObject::CreateNonMovable(cls)));
    auto *arrayPtr = EtsByteArray::Create(length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ObjectAccessor::SetObject(currentCoro, hmem.GetPtr(), MEMBER_OFFSET(EtsSharedMemory, array_),
                              arrayPtr->GetCoreType());
    hmem->SetHeadWaiter(nullptr);
    return hmem.GetPtr();
}

void EtsSharedMemory::LinkWaiter(Waiter &waiter)
{
    waiter.SetPrev(nullptr);
    waiter.SetNext(GetHeadWaiter());
    SetHeadWaiter(&waiter);
}

void EtsSharedMemory::UnlinkWaiter(Waiter &waiter)
{
    auto prev = waiter.GetPrev();
    auto next = waiter.GetNext();

    if (prev != nullptr) {
        prev->SetNext(next);
    }
    if (next != nullptr) {
        next->SetPrev(prev);
    }
    if (GetHeadWaiter() == &waiter) {
        SetHeadWaiter(nullptr);
    }
}

size_t EtsSharedMemory::GetLength()
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    auto *arrayPtr = reinterpret_cast<EtsByteArray *>(
        ObjectAccessor::GetObject(currentCoro, this, MEMBER_OFFSET(EtsSharedMemory, array_)));
    return arrayPtr->GetLength();
}

namespace {

std::string PrintWaiters(EtsHandle<EtsSharedMemory> &buffer)
{
    std::stringstream stream;
    auto curWaiter = buffer->GetHeadWaiter();
    while (curWaiter != nullptr) {
        stream << reinterpret_cast<size_t>(curWaiter) << " -> ";
        curWaiter = curWaiter->GetNext();
    }
    return stream.str();
}

bool IsLittleEndian()
{
    int32_t x = 1;
    return *reinterpret_cast<int8_t *>(&x) == static_cast<int8_t>(1);
}

template <typename IntegerType, typename UIntegerType>
IntegerType AssembleFromBytes(EtsSharedMemory &mem, uint32_t index, uint32_t (*getByteIndex)(uint32_t, uint32_t))
{
    UIntegerType value = 0;
    for (uint32_t i = 0; i < sizeof(IntegerType); i++) {
        auto curByteIndex = getByteIndex(index, i);
        auto curByte = bit_cast<UIntegerType>(static_cast<IntegerType>(mem.GetElement<int8_t>(curByteIndex)));
        value |= curByte << (8U * i);
    }
    return bit_cast<IntegerType>(value);
}

uint32_t LittleEndianGetByteIndex(uint32_t index, uint32_t i)
{
    return index + i;
}

template <typename IntegerType>
uint32_t BigEndianGetByteIndex(uint32_t index, uint32_t i)
{
    return index + sizeof(IntegerType) - 1 - i;
}

template <typename IntegerType, typename UIntegerType>
IntegerType AssembleFromBytes(EtsSharedMemory &mem, uint32_t index)
{
    return IsLittleEndian()
               ? AssembleFromBytes<IntegerType, UIntegerType>(mem, index, LittleEndianGetByteIndex)
               : AssembleFromBytes<IntegerType, UIntegerType>(mem, index, BigEndianGetByteIndex<IntegerType>);
}

template <typename IntegerType, typename UIntegerType>
EtsSharedMemory::WaitResult Wait(EtsSharedMemory *mem, uint32_t byteOffset, IntegerType expectedValue,
                                 std::optional<uint64_t> timeout)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsSharedMemory> thisHandle(coroutine, mem);

    ScopedNativeCodeThread n(coroutine);
    os::memory::LockHolder lock(coroutine->GetPandaVM()->GetAtomicsMutex());
    ScopedManagedCodeThread m(coroutine);

    auto witnessedValue = AssembleFromBytes<IntegerType, UIntegerType>(*(thisHandle.GetPtr()), byteOffset);
    LOG(DEBUG, ATOMICS) << "Wait: witnesseed_value=" << witnessedValue << ", expected_value=" << expectedValue;
    if (witnessedValue == expectedValue) {
        // Only stack allocations

        // 1. Add waiter
        auto waiter = EtsSharedMemory::Waiter(byteOffset);
        thisHandle->LinkWaiter(waiter);
        LOG(DEBUG, ATOMICS) << "Wait: added waiter: " << reinterpret_cast<size_t>(&waiter)
                            << ", list: " << PrintWaiters(thisHandle);

        // 2. Wait
        bool timedOut = false;
        while (!waiter.IsNotified() && !timedOut) {
            ScopedNativeCodeThread nCv(coroutine);

            timedOut = waiter.Wait(timeout);
            LOG(DEBUG, ATOMICS) << "Wait: woke up, waiter: " << reinterpret_cast<size_t>(&waiter);
        }

        // 3. Remove waiter
        thisHandle->UnlinkWaiter(waiter);
        LOG(DEBUG, ATOMICS) << "Wait: removed waiter: " << reinterpret_cast<size_t>(&waiter)
                            << ", list: " << PrintWaiters(thisHandle);
        return timedOut ? EtsSharedMemory::WaitResult::TIMED_OUT : EtsSharedMemory::WaitResult::OK;
    }

    LOG(DEBUG, ATOMICS) << "Wait: not-equal, returning";
    return EtsSharedMemory::WaitResult::NOT_EQUAL;
}

}  // namespace

EtsSharedMemory::WaitResult EtsSharedMemory::WaitI32(uint32_t byteOffset, int32_t expectedValue,
                                                     std::optional<uint64_t> timeout)
{
    return Wait<int32_t, uint32_t>(this, byteOffset, expectedValue, timeout);
}

EtsSharedMemory::WaitResult EtsSharedMemory::WaitI64(uint32_t byteOffset, int64_t expectedValue,
                                                     std::optional<uint64_t> timeout)
{
    return Wait<int64_t, uint64_t>(this, byteOffset, expectedValue, timeout);
}

int32_t EtsSharedMemory::NotifyI32(uint32_t byteOffset, std::optional<uint32_t> count)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsSharedMemory> thisHandle(coroutine, this);

    ScopedNativeCodeThread n(coroutine);
    os::memory::LockHolder lock(coroutine->GetPandaVM()->GetAtomicsMutex());
    ScopedManagedCodeThread m(coroutine);

    auto waiter = thisHandle->GetHeadWaiter();
    uint32_t notifiedCount = 0;
    LOG(DEBUG, ATOMICS) << "Notify: started, head waiter: " << reinterpret_cast<size_t>(waiter);
    while (waiter != nullptr && (!count.has_value() || notifiedCount < count.value())) {
        LOG(DEBUG, ATOMICS) << "Notify: vising waiter: " << reinterpret_cast<size_t>(waiter)
                            << ", list: " << PrintWaiters(thisHandle);
        if (waiter->GetOffset() == byteOffset) {
            ScopedNativeCodeThread nCv(coroutine);

            waiter->SetNotified();
            LOG(DEBUG, ATOMICS) << "Notify: notifying waiter " << reinterpret_cast<size_t>(waiter)
                                << ", list: " << PrintWaiters(thisHandle);
            waiter->SignalAll();
            notifiedCount += 1;
        }
        waiter = waiter->GetNext();
    }
    LOG(DEBUG, ATOMICS) << "Notify: notified " << notifiedCount << " waiters";

    return static_cast<int32_t>(notifiedCount);
}

}  // namespace ark::ets