/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_EVENTS_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_EVENTS_H

#include "runtime/include/runtime.h"
#include "runtime/mem/refstorage/reference.h"

namespace ark {

/**
 * @brief The base class for coroutine events. Cannot be instantiated directly.
 *
 * These events are used to implement blocking and unblocking the coroutines that are waiting for something.
 * The lifetime of an event is intended to be managed manually or be bound to its owner lifetime (e.g. coroutine).
 */
class CAPABILITY("mutex") CoroutineEvent {
public:
    NO_COPY_SEMANTIC(CoroutineEvent);
    NO_MOVE_SEMANTIC(CoroutineEvent);

    enum class Type { NONE, GENERIC, COMPLETION, CHANNEL, IO };

    virtual ~CoroutineEvent()
    {
        if (IsLocked()) {
            Unlock();
        }
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(mutex_);
    };

    Type GetType()
    {
        return type_;
    }

    bool Happened() REQUIRES(this)
    {
        return happened_;
    }

    void SetHappened()
    {
        Lock();
        happened_ = true;
        Unlock();
    }

    void SetNotHappened()
    {
        Lock();
        happened_ = false;
        Unlock();
    }

    void Lock() ACQUIRE()
    {
        mutex_->Lock();
        locked_ = true;
    }

    void Unlock() RELEASE()
    {
        locked_ = false;
        mutex_->Unlock();
    }

protected:
    explicit CoroutineEvent(Type t) : type_(t)
    {
        mutex_ = Runtime::GetCurrent()->GetInternalAllocator()->New<os::memory::Mutex>();
    }

    bool IsLocked()
    {
        return locked_;
    }

private:
    Type type_ = Type::NONE;
    bool happened_ GUARDED_BY(this) = false;
    bool locked_ = false;

    os::memory::Mutex *mutex_;
};

/**
 * @brief The generic event: just some  event that can be awaited.
 *
 * The only thing that it can do: it can happen.
 */
class GenericEvent : public CoroutineEvent {
public:
    NO_COPY_SEMANTIC(GenericEvent);
    NO_MOVE_SEMANTIC(GenericEvent);

    explicit GenericEvent() : CoroutineEvent(Type::GENERIC) {}
    ~GenericEvent() override = default;

private:
};

/// @brief The coroutine completion event: happens when coroutine is done executing its bytecode.
class CompletionEvent : public CoroutineEvent {
public:
    NO_COPY_SEMANTIC(CompletionEvent);
    NO_MOVE_SEMANTIC(CompletionEvent);

    /**
     * @param promise A weak reference (from global storage) to the language-dependent promise object that will hold
     * the coroutine return value.
     */
    explicit CompletionEvent(mem::Reference *promise) : CoroutineEvent(Type::COMPLETION), promise_(promise) {}
    ~CompletionEvent() override = default;

    mem::Reference *GetPromise()
    {
        return promise_;
    }

private:
    mem::Reference *promise_ = nullptr;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_EVENTS_H */