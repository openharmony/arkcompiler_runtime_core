/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "intrinsics.h"
#include "libpandabase/os/mutex.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::intrinsics {

using TimerId = int32_t;

class TimerInfo {
public:
    TimerInfo(mem::Reference *callback, int delay, bool isPeriodic, TimerId id)
        : callback_(callback), delay_(delay), isPeriodic_(isPeriodic), event_(Coroutine::GetCurrent()->GetManager(), id)
    {
    }

    NO_COPY_SEMANTIC(TimerInfo);
    NO_MOVE_SEMANTIC(TimerInfo);

    mem::Reference *GetCallback() const
    {
        return callback_;
    }

    int GetDelay() const
    {
        return delay_;
    }

    bool IsPeriodic() const
    {
        return isPeriodic_;
    }

    TimerEvent *GetEvent()
    {
        return &event_;
    }

    bool IsCancelled() const;

private:
    mem::Reference *callback_;
    const int delay_;
    const bool isPeriodic_;

    TimerEvent event_;
};

class TimerTable {
public:
    void RegisterTimer(TimerEvent *event)
    {
        os::memory::LockHolder lh(lock_);
        timers_.insert({event->GetId(), event});
    }

    void DisarmTimer(TimerId timerId)
    {
        os::memory::LockHolder lh(lock_);
        auto timerIter = timers_.find(timerId);
        if (timerIter != timers_.end()) {
            auto *timerEvent = timerIter->second;
            timerEvent->SetExpired();
            timers_.erase(timerIter);
            os::memory::LockHolder lk(*timerEvent);
            timerEvent->Happen();
        }
    }

    void RemoveTimer(TimerId timerId)
    {
        os::memory::LockHolder lh(lock_);
        auto timerIter = timers_.find(timerId);
        if (timerIter != timers_.end()) {
            timers_.erase(timerIter);
        }
    }

    bool HasTimer(TimerId timerId)
    {
        os::memory::LockHolder lh(lock_);
        auto timerIter = timers_.find(timerId);
        return timerIter != timers_.end();
    }

private:
    os::memory::Mutex lock_;
    std::unordered_map<TimerId, TimerEvent *> timers_;
};

static TimerTable g_timerTable = {};
static std::atomic<TimerId> nextTimerId_ = 1;

static void InvokeCallback(mem::Reference *callback)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ScopedManagedCodeThread managedScope(coro);
    auto *lambda = coro->GetVM()->GetGlobalObjectStorage()->Get(callback);
    LambdaUtils::InvokeVoid(coro, EtsObject::FromCoreType(lambda));
}

bool TimerInfo::IsCancelled() const
{
    return !g_timerTable.HasTimer(event_.GetId());
}

TimerId StdCoreRegisterTimer(EtsObject *callback, int32_t delay, uint8_t periodic)
{
    // Atomic with relaxed order reason: sync is not needed here
    auto timerId = nextTimerId_.fetch_add(1, std::memory_order_relaxed);
    auto *coro = Coroutine::GetCurrent();
    auto callbackRef =
        coro->GetVM()->GetGlobalObjectStorage()->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    auto *timerInfo =
        Runtime::GetCurrent()->GetInternalAllocator()->New<TimerInfo>(callbackRef, delay, periodic, timerId);
    g_timerTable.RegisterTimer(timerInfo->GetEvent());

    auto timerEntrypoint = [](void *param) {
        auto *timer = static_cast<TimerInfo *>(param);
        auto *timerCoro = Coroutine::GetCurrent();
        while (true) {
            auto *timerEvent = timer->GetEvent();
            if (timer->GetDelay() != 0) {
                auto *coroMan = timerCoro->GetManager();
                timerEvent->SetNotHappened();
                timerEvent->Lock();
                timerEvent->SetExpirationTime(coroMan->GetCurrentTime() + timer->GetDelay());
                coroMan->Await(timerEvent);
            }
            if (timer->IsCancelled()) {
                break;
            }

            InvokeCallback(timer->GetCallback());

            if (!timer->IsPeriodic()) {
                break;
            }
            timerCoro->GetManager()->Schedule();
        }

        g_timerTable.RemoveTimer(timer->GetEvent()->GetId());
        timerCoro->GetVM()->GetGlobalObjectStorage()->Remove(timer->GetCallback());
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(timer);
    };

    auto coroName = "Timer callback: " + ark::ToPandaString(timerId);
    auto wGroup = ark::CoroutineWorkerGroup::GenerateExactWorkerId(coro->GetWorker()->GetId());
    coro->GetManager()->LaunchNative(timerEntrypoint, timerInfo, std::move(coroName), wGroup,
                                     ark::ets::EtsCoroutine::TIMER_CALLBACK, true);
    return timerId;
}

void StdCoreClearTimer(TimerId timerId)
{
    g_timerTable.DisarmTimer(timerId);
}

}  // namespace ark::ets::intrinsics
