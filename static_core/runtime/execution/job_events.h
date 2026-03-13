/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_EVENTS_H
#define PANDA_RUNTIME_EXECUTION_JOB_EVENTS_H

#include "libarkbase/os/mutex.h"
#include "runtime/mem/refstorage/reference.h"
#include <utility>

namespace ark {

using EventId = int32_t;

class JobManager;

/**
 * @brief The base class for job events. Cannot be instantiated directly.
 *
 * These events are used to implement blocking and unblocking the coroutines that are waiting for something.
 * The lifetime of an event is intended to be managed manually or be bound to its owner lifetime (e.g. job).
 */
class CAPABILITY("mutex") JobEvent {
public:
    NO_COPY_SEMANTIC(JobEvent);
    NO_MOVE_SEMANTIC(JobEvent);

    enum class Type { NONE, GENERIC, COMPLETION, CHANNEL, IO, TIMER, BLOCKING };

    virtual ~JobEvent()
    {
        ASSERT(!IsLocked());
    };

    Type GetType()
    {
        return type_;
    }

    bool Happened() REQUIRES(this)
    {
        return happened_;
    }

    /// Set event happened and unblock waiters
    void Happen();

    void SetNotHappened()
    {
        Lock();
        happened_ = false;
        Unlock();
    }

    void Lock() ACQUIRE()
    {
        mutex_.Lock();
        locked_ = true;
    }

    void Unlock() RELEASE()
    {
        locked_ = false;
        mutex_.Unlock();
    }

    EventId GetId() const
    {
        return eventId_;
    }

protected:
    static constexpr EventId DEFAULT_EVENT_ID = 0;

    explicit JobEvent(Type t, JobManager *jobManager, EventId id = DEFAULT_EVENT_ID)
        : jobManager_(jobManager), type_(t), eventId_(id)
    {
    }

    bool IsLocked()
    {
        return locked_;
    }

    void SetHappened()
    {
        Lock();
        happened_ = true;
        Unlock();
    }

protected:
    JobManager *jobManager_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Type type_ = Type::NONE;
    bool happened_ GUARDED_BY(this) = false;
    bool locked_ = false;
    EventId eventId_;

    os::memory::RecursiveMutex mutex_;
};

/**
 * @brief The generic event: just some  event that can be awaited.
 *
 * The only thing that it can do: it can happen.
 */
class GenericEvent : public JobEvent {
public:
    NO_COPY_SEMANTIC(GenericEvent);
    NO_MOVE_SEMANTIC(GenericEvent);

    explicit GenericEvent(JobManager *jobManager) : JobEvent(Type::GENERIC, jobManager) {}
    ~GenericEvent() override = default;

private:
};

/// @brief The job completion event: happens when job is done executing its bytecode.
class CompletionEvent : public JobEvent {
public:
    NO_COPY_SEMANTIC(CompletionEvent);
    NO_MOVE_SEMANTIC(CompletionEvent);

    /**
     * @param returnValueObject A weak reference (from global storage) to the language-dependent language object that
     * will hold the job return value.
     */
    explicit CompletionEvent(mem::Reference *returnValueObject, JobManager *jobManager)
        : JobEvent(Type::COMPLETION, jobManager), returnValueObject_(returnValueObject)
    {
    }
    ~CompletionEvent() override = default;

    mem::Reference *ReleaseReturnValueObject()
    {
        return std::exchange(returnValueObject_, nullptr);
    }

    void SetReturnValueObject(mem::Reference *returnValueObject)
    {
        ASSERT(returnValueObject_ == nullptr);
        returnValueObject_ = returnValueObject;
    }

private:
    mem::Reference *returnValueObject_ = nullptr;
};

class TimerEvent : public JobEvent {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    explicit TimerEvent(JobManager *jobManager, EventId id) : JobEvent(Type::TIMER, jobManager, id) {}

    NO_COPY_SEMANTIC(TimerEvent);
    NO_MOVE_SEMANTIC(TimerEvent);

    bool IsExpired() const
    {
        return expirationTime_ <= currentTime_;
    }

    void SetExpired()
    {
        currentTime_ = std::numeric_limits<uint64_t>::max();
    }

    void SetExpirationTime(uint64_t expirationTime)
    {
        expirationTime_ = expirationTime;
    }

    uint64_t GetExpirationTime() const
    {
        return expirationTime_;
    }

    void SetCurrentTime(uint64_t currentTime)
    {
        currentTime_ = currentTime;
    }

    uint64_t GetDelay() const
    {
        return expirationTime_ - currentTime_;
    }

private:
    /// the time is represented in microseconds
    std::atomic<uint64_t> currentTime_ = 0;
    uint64_t expirationTime_ = 0;
};

/**
 * @brief Blocking event used by synchronization primitives.
 *
 * This event is created as a temporary stack-local object when a job needs to block
 * waiting for a synchronization primitive (mutex lock, event fire, etc.).
 *
 * Usage pattern:
 * 1. Create BlockingEvent on stack (or obtain from pool)
 * 2. Add to waiters list of the synchronization primitive
 * 3. Call Wait() - this blocks the coroutine
 * 4. When the primitive signals, the event is automatically cleaned up
 *
 * Execution model differences:
 * - stackless: BlockingEvent::Wait blocks the worker thread itself. Other jobs
 *   cannot be executed on the same worker while the current job is blocked
 * - stackful: BlockingEvent::Wait suspends only the current coroutine/fiber, allowing
 *   other coroutines to run on the same worker thread
 */
class BlockingEvent : public JobEvent {
public:
    explicit BlockingEvent(JobManager *jobManager) : JobEvent(Type::BLOCKING, jobManager) {}
    ~BlockingEvent() override = default;

    NO_COPY_SEMANTIC(BlockingEvent);
    NO_MOVE_SEMANTIC(BlockingEvent);

    void Wait() RELEASE(this);
};

}  // namespace ark

#endif /* PANDA_RUNTIME_EXECUTION_JOB_EVENTS_H */
