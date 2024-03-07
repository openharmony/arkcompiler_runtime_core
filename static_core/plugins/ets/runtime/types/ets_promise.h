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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_PROMISE_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_PROMISE_H_

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "runtime/coroutines/coroutine_events.h"

namespace ark::ets {

class EtsCoroutine;

namespace test {
class EtsPromiseTest;
}  // namespace test

class EtsPromise : public ObjectHeader {
public:
    // temp
    static constexpr EtsInt STATE_PENDING = 0;
    static constexpr EtsInt STATE_RESOLVED = 1;
    static constexpr EtsInt STATE_REJECTED = 2;

    EtsPromise() = delete;
    ~EtsPromise() = delete;

    NO_COPY_SEMANTIC(EtsPromise);
    NO_MOVE_SEMANTIC(EtsPromise);

    PANDA_PUBLIC_API static EtsPromise *Create(EtsCoroutine *etsCoroutine = EtsCoroutine::GetCurrent());

    static EtsPromise *FromCoreType(ObjectHeader *promise)
    {
        return reinterpret_cast<EtsPromise *>(promise);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsPromise *FromEtsObject(EtsObject *promise)
    {
        return reinterpret_cast<EtsPromise *>(promise);
    }

    EtsObjectArray *GetThenQueue(EtsCoroutine *coro)
    {
        return EtsObjectArray::FromCoreType(
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, thenQueue_)));
    }

    EtsInt GetThenQueueSize()
    {
        return thenQueueSize_;
    }

    EtsObjectArray *GetCatchQueue(EtsCoroutine *coro)
    {
        return EtsObjectArray::FromCoreType(
            ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, catchQueue_)));
    }

    EtsInt GetCatchQueueSize()
    {
        return catchQueueSize_;
    }

    void ClearQueues(EtsCoroutine *coro)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, thenQueue_), nullptr);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, catchQueue_), nullptr);
        thenQueueSize_ = 0;
        catchQueueSize_ = 0;
    }

    CoroutineEvent *GetEventPtr()
    {
        return reinterpret_cast<CoroutineEvent *>(event_);
    }

    void SetEventPtr(CoroutineEvent *e)
    {
        event_ = reinterpret_cast<EtsLong>(e);
    }

    bool IsResolved() const
    {
        return (state_ == STATE_RESOLVED);
    }

    bool IsRejected() const
    {
        return (state_ == STATE_REJECTED);
    }

    bool IsPending() const
    {
        return (state_ == STATE_PENDING);
    }

    bool IsProxy()
    {
        return GetLinkedPromise(EtsCoroutine::GetCurrent()) != nullptr;
    }

    EtsObject *GetLinkedPromise(EtsCoroutine *coro)
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, linkedPromise_));
        return EtsObject::FromCoreType(obj);
    }

    void SetLinkedPromise(EtsCoroutine *coro, EtsObject *p)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, linkedPromise_), p->GetCoreType());
    }

    void Resolve(EtsCoroutine *coro, EtsObject *value)
    {
        os::memory::LockHolder lk(*this);
        ASSERT(state_ == STATE_PENDING);
        auto coreValue = (value == nullptr) ? nullptr : value->GetCoreType();
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, value_), coreValue);
        state_ = STATE_RESOLVED;
        SetEventPtr(nullptr);
    }

    void Reject(EtsCoroutine *coro, EtsObject *error)
    {
        os::memory::LockHolder lk(*this);
        ASSERT(state_ == STATE_PENDING);
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromise, value_), error->GetCoreType());
        state_ = STATE_REJECTED;
        SetEventPtr(nullptr);
    }

    EtsObject *GetValue(EtsCoroutine *coro)
    {
        return EtsObject::FromCoreType(ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromise, value_)));
    }

    uint32_t GetState() const
    {
        return state_;
    }

    static size_t ValueOffset()
    {
        return MEMBER_OFFSET(EtsPromise, value_);
    }

    void Lock()
    {
        auto tid = EtsCoroutine::GetCurrent()->GetCoroutineId();
        ASSERT((tid & MarkWord::LIGHT_LOCK_THREADID_MASK) != 0);
        auto mwExpected = AtomicGetMark();
        switch (mwExpected.GetState()) {
            case MarkWord::STATE_GC:
                LOG(FATAL, COROUTINES) << "Cannot lock a GC-collected Promise object!";
                break;
            case MarkWord::STATE_LIGHT_LOCKED:
            case MarkWord::STATE_HEAVY_LOCKED:
                // should wait until unlock...
                // NOTE(konstanting, #I67QXC): check tid and decide what to do in case when locked by current tid
                mwExpected = mwExpected.DecodeFromUnlocked();
                break;
            case MarkWord::STATE_HASHED:
                // NOTE(konstanting, #I67QXC): should save the hash
                LOG(FATAL, COROUTINES)
                    << "Cannot lock a hashed Promise object! This functionality is not implemented yet.";
                break;
            case MarkWord::STATE_UNLOCKED:
            default:
                break;
        }
        auto newMw = mwExpected.DecodeFromLightLock(tid, 0);
#ifndef NDEBUG
        size_t cycles = 0;
#endif /* NDEBUG */
        while (!AtomicSetMark<false>(mwExpected, newMw)) {
            mwExpected = mwExpected.DecodeFromUnlocked();
            newMw = mwExpected.DecodeFromLightLock(tid, 0);
#ifndef NDEBUG
            ++cycles;
#endif /* NDEBUG */
        }
#ifndef NDEBUG
        LOG(DEBUG, COROUTINES) << "Promise::Lock(): took " << cycles << " cycles";
#endif /* NDEBUG */
    }

    void Unlock()
    {
        // precondition: is locked AND if the event pointer is set then the event is locked
        // NOTE(konstanting, #I67QXC): find a way to add an ASSERT for (the event is locked)
        // NOTE(konstanting, #I67QXC): should load the hash once we support the hashed state in Lock()
        auto mw = AtomicGetMark();
        ASSERT(mw.GetState() == MarkWord::STATE_LIGHT_LOCKED);
        auto newMw = mw.DecodeFromUnlocked();
        while (!AtomicSetMark<false>(mw, newMw)) {
            ASSERT(mw.GetState() == MarkWord::STATE_LIGHT_LOCKED);
            newMw = mw.DecodeFromUnlocked();
        }
    }

private:
    ObjectPointer<EtsObject> value_;  // the completion value of the Promise
    ObjectPointer<EtsObjectArray>
        thenQueue_;  // the queue of 'then' calbacks which will be called when the Promise gets resolved
    ObjectPointer<EtsObjectArray>
        catchQueue_;  // the queue of 'catch' callback which will be called when the Promise gets rejected
    ObjectPointer<EtsObject> linkedPromise_;  // linked JS promise as JSValue (if exists)
    EtsLong event_;
    EtsInt thenQueueSize_;
    EtsInt catchQueueSize_;
    uint32_t state_;  // the Promise's state

    friend class test::EtsPromiseTest;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_PROMISE_H_
