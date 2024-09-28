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

/// @brief The base class for sync primitives. Should not be used directly.
template <typename T>
class EtsSyncPrimitive : public EtsObject {
public:
    static T *FromCoreType(ObjectHeader *syncPrimitive)
    {
        return reinterpret_cast<T *>(syncPrimitive);
    }

    static T *FromEtsObject(EtsObject *syncPrimitive)
    {
        return reinterpret_cast<T *>(syncPrimitive);
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
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsSyncPrimitive, waitersList_)));
    }

    void SetWaitersList(EtsCoroutine *coro, EtsWaitersList *waitersList)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsSyncPrimitive, waitersList_),
                                  waitersList->GetCoreType());
    }

    /**
     * Blocks current coroutine. It can be used concurrently with other Suspend and Resume
     * @param awaitee - EtsWaitersList node that contains GenericEvent and probably other useful data
     * NOTE: `this` and all other raw ObjectHeaders may become invalid after this call due to GC
     */
    ALWAYS_INLINE void SuspendCoroutine(EtsWaitersList::Node *awaitee)
    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *coroManager = coro->GetCoroutineManager();
        auto &event = awaitee->GetEvent();
        // Need to lock event before PushBack
        // to avoid use-after-free in CoroutineEvent::Happen method
        event.Lock();
        GetWaitersList(coro)->PushBack(awaitee);
        coroManager->Await(&event);
    }

    /**
     * Unblocks suspended coroutine. It can be used concurrently with Suspend,
     * BUT not with other Resume (PopFront is not thread-safety)
     */
    ALWAYS_INLINE void ResumeCoroutine()
    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *awaitee = GetWaitersList(coro)->PopFront();
        awaitee->GetEvent().Happen();
    }

private:
    ObjectPointer<EtsWaitersList> waitersList_;

    friend class test::EtsSyncPrimitivesTest;
};

/// @brief Coroutine mutex. This allows to get exclusive access to the critical section.
class EtsMutex : public EtsSyncPrimitive<EtsMutex> {
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

private:
    std::atomic<uint32_t> waiters_;

    friend class test::EtsSyncPrimitivesTest;
};

/// @brief Coroutine one-shot event. This allows to block current coroutine until event is fired
class EtsEvent : public EtsSyncPrimitive<EtsEvent> {
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

    std::atomic<uint32_t> state_;

    friend class test::EtsSyncPrimitivesTest;
};

/// Coroutine conditional variable
class EtsCondVar : public EtsSyncPrimitive<EtsCondVar> {
public:
    /**
     * precondition: mutex is locked
     * 1. Unlocks mutex
     * 2. Blocks current coroutine
     * 3. Locks mutex
     * This method is thread-safe in relation to other methods of the CondVar
     */
    void Wait(EtsHandle<EtsMutex> &mutex);

    /**
     * precondition: mutex is locked
     * Unblocks ONE suspended coroutine associated with this CondVar, if it exists.
     * This method is thread-safe in relation to other methods of the CondVar
     */
    void NotifyOne([[maybe_unused]] EtsMutex *mutex);

    /**
     * precondition: mutex is locked
     * Unblocks ALL suspended coroutine associated with this CondVar.
     * This method is thread-safe in relation to other methods of the CondVar
     */
    void NotifyAll([[maybe_unused]] EtsMutex *mutex);

    static EtsCondVar *Create(EtsCoroutine *coro);

private:
    EtsInt waiters_;

    friend class test::EtsSyncPrimitivesTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MUTEX_H
