/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MUTEX_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MUTEX_H

#include "libpandabase/mem/object_pointer.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_waiters_list.h"

#include <cstdint>

namespace ark::ets {

template <typename T>
class EtsHandle;

namespace test {
class EtsSyncPrimitivesTest;
}  // namespace test

/// @brief Coroutine mutex. This allows to get exclusive access to the critical section.
class EtsMutex : public EtsObject {
public:
    template <typename T>
    class LockHolder {
    public:
        explicit LockHolder(EtsHandle<T> &hLock) : hLock_(hLock)
        {
            hLock_->Lock();
        }

        NO_COPY_SEMANTIC(LockHolder);
        NO_MOVE_SEMANTIC(LockHolder);

        ~LockHolder()
        {
            hLock_->Unlock();
        }

    private:
        EtsHandle<T> &hLock_;
    };

    /**
     * Method gives exclusive access to critical section is case of successful mutex lock.
     * Otherwise, blocks current coroutine until other coroutine will unlock the mutex.
     */
    void Lock();

    /**
     * Calling the method means exiting the critical section.
     * If there are blocked coroutines, unblocks one of them.
     */
    void Unlock();

    /// This method should be used to make sure that mutex is locked by current coroutine.
    bool IsHeld();

    static EtsMutex *Create(EtsCoroutine *coro);

    static EtsMutex *FromCoreType(ObjectHeader *mutex)
    {
        return reinterpret_cast<EtsMutex *>(mutex);
    }

    static EtsMutex *FromEtsObject(EtsObject *mutex)
    {
        return reinterpret_cast<EtsMutex *>(mutex);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsWaitersList *GetWaitersList(EtsCoroutine *coro)
    {
        return EtsWaitersList::FromCoreType(
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsMutex, waitersList_)));
    }

    void SetWaitersList(EtsCoroutine *coro, EtsWaitersList *waitersList)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsMutex, waitersList_), waitersList->GetCoreType());
    }

private:
    ObjectPointer<EtsWaitersList> waitersList_;
    std::atomic<uint32_t> waiters_;

    friend class test::EtsSyncPrimitivesTest;
};

/// @brief Coroutine one-shot event. This allows to block current coroutine until event is fired
class EtsEvent : public EtsObject {
public:
    /**
     * Blocks current coroutine until another coroutine will fire the same event.
     * It can be used concurrently with other wait/fire.
     * It has no effect in case fire has already been caused, but guarantees happens-before.
     */
    void Wait();

    /**
     * Unblocks all coroutines that are waiting the same event.
     * Can be used concurrently with other wait/fire but multiply calls have no effect.
     */
    void Fire();

    static EtsEvent *Create(EtsCoroutine *coro);

    static EtsEvent *FromCoreType(ObjectHeader *event)
    {
        return reinterpret_cast<EtsEvent *>(event);
    }

    static EtsEvent *FromEtsObject(EtsObject *event)
    {
        return reinterpret_cast<EtsEvent *>(event);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsWaitersList *GetWaitersList(EtsCoroutine *coro)
    {
        return EtsWaitersList::FromCoreType(
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsEvent, waitersList_)));
    }

    void SetWaitersList(EtsCoroutine *coro, EtsWaitersList *waitersList)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsEvent, waitersList_), waitersList->GetCoreType());
    }

private:
    static constexpr uint32_t STATE_BIT = 1U;
    static constexpr uint32_t FIRE_STATE = 1U;
    static constexpr uint32_t ONE_WAITER = 2U;

    static bool IsFireState(uint32_t state)
    {
        return (state & STATE_BIT) == FIRE_STATE;
    }

    static uint32_t GetNumberOfWaiters(uint32_t state)
    {
        return state >> STATE_BIT;
    }

    ObjectPointer<EtsWaitersList> waitersList_;
    std::atomic<uint32_t> state_;

    friend class test::EtsSyncPrimitivesTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MUTEX_H
