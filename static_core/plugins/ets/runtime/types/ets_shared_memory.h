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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_SHAREDMEM_H
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_SHAREDMEM_H

#include <atomic>
#include <cstdint>
#include <optional>
#include "include/mem/panda_containers.h"
#include "libpandabase/macros.h"
#include "libpandabase/mem/space.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/mem/object_pointer.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets {

class EtsSharedMemory : public ObjectHeader {
public:
    EtsSharedMemory() = delete;
    ~EtsSharedMemory() = delete;

    NO_COPY_SEMANTIC(EtsSharedMemory);
    NO_MOVE_SEMANTIC(EtsSharedMemory);

    class Waiter;

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsSharedMemory *FromEtsObject(EtsObject *sharedMem)
    {
        return reinterpret_cast<EtsSharedMemory *>(sharedMem);
    }

    static EtsSharedMemory *Create(size_t length);

    void Initialize(size_t length);

    size_t GetLength();

    Waiter *GetHeadWaiter()
    {
        return reinterpret_cast<Waiter *>(waiter_);
    }

    void SetHeadWaiter(Waiter *waiter)
    {
        waiter_ = reinterpret_cast<EtsLong>(waiter);
    }

    void LinkWaiter(Waiter &waiter);
    void UnlinkWaiter(Waiter &waiter);

    template <typename T>
    T GetElement(uint32_t index);
    template <typename T>
    void SetElement(uint32_t index, T element);

    template <typename T, typename F>
    std::pair<T, T> ReadModifyWrite(int32_t index, const F &f);

    enum class WaitResult { OK = 0, NOT_EQUAL = 1, TIMED_OUT = 2 };

    WaitResult WaitI32(uint32_t offset, int32_t expectedValue, std::optional<uint64_t> timeout);

    WaitResult WaitI64(uint32_t offset, int64_t expectedValue, std::optional<uint64_t> timeout);

    int32_t NotifyI32(uint32_t offset, std::optional<uint32_t> count);

    class Waiter {
    public:
        explicit Waiter(uint32_t offset) : offset_(offset) {}

        bool Wait(std::optional<uint64_t> timeout);

        void SignalAll();

        bool IsNotified()
        {
            // Atomic with seq_cst order reason: strong synchronization
            return notified_.load(std::memory_order_seq_cst);
        }

        void SetNotified()
        {
            // Atomic with seq_cst order reason: strong synchronization
            notified_.store(true, std::memory_order_seq_cst);
        }

        uint32_t GetOffset()
        {
            return offset_;
        }

        Waiter *GetNext()
        {
            return next_;
        }

        void SetNext(Waiter *next)
        {
            next_ = next;
        }

        Waiter *GetPrev()
        {
            return prev_;
        }

        void SetPrev(Waiter *prev)
        {
            prev_ = prev;
        }

    private:
        os::memory::ConditionVariable cv_ {os::memory::ConditionVariable()};
        std::atomic<bool> notified_ {std::atomic<bool>(false)};
        uint32_t offset_;

        Waiter *prev_ {nullptr};
        Waiter *next_ {nullptr};
    };

private:
    ObjectPointer<EtsByteArray> array_;
    EtsLong waiter_;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_SHAREDMEM_H
