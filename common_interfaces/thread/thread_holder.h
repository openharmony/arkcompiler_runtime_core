/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_THREAD_THREAD_HOLDER_H
#define COMMON_INTERFACES_THREAD_THREAD_HOLDER_H

#include <condition_variable>
#include <mutex>
#include <unordered_set>

#include "base/common.h"
#include "heap/heap_visitor.h"
#include "thread/mutator_base.h"
#include "thread/thread_state.h"


namespace panda::ecmascript {
class JSThread;
}

namespace ark {
class Coroutine;
}

namespace panda {
class BaseThread;
class ThreadHolderManager;

/**
 * ThreadHolder does two things:
 * 1. Represent the current ThreadState(RUNNING for VM, or NATIVE for NativeCode) for all the execution BaseThread
 *    registered to this ThreadHolder, they will share the same ThreadState. And be responsible for transfer
 *    ThreadState, and is the control unit for ThreadHolderManager::SuspendAll()/ResumeAll().
 * 2. Maintain all the execution BaseThread registered to the current ThreadHolder.
 *
 * ThreadHolder is a package of execution BaseThreads which must run in the same OS Thread and so could
 * share ThreadState.
 */
class ThreadHolder {
public:
    using JSThread = panda::ecmascript::JSThread;
    using Coroutine = ark::Coroutine;
    using MutatorBase = panda::MutatorBase;

    ThreadHolder(MutatorBase *mutatorBase) : mutatorBase_(mutatorBase)
    {
        SetCurrent(this);
    }

    static ThreadHolder *GetCurrent();
    static void SetCurrent(ThreadHolder *holder);

    // This is a temporary impl so we need pass vm from JSThread, or nullptr otherwise
    static ThreadHolder *CreateAndRegisterNewThreadHolder(void *vm);
    static void DestroyThreadHolder(ThreadHolder *holder);

    // Transfer to Running no matter in Running or Native.
    inline void TransferToRunning();

    // Transfer to Native no matter in Running or Native.
    inline void TransferToNative();

    // If current in Native, transfer to Running and return true;
    // If current in Running, do nothing and return false.
    inline bool TransferToRunningIfInNative();

    // If current in Running, transfer to Native and return true;
    // If current in Native, do nothing and return false.
    inline bool TransferToNativeIfInRunning();

    void CheckSafepointIfSuspended()
    {
        if (UNLIKELY_CC(HasSuspendRequest())) {
            WaitSuspension();
        }
    }

    void WaitSuspension();

    bool HasSuspendRequest() const
    {
        return mutatorBase_->HasAnySuspensionRequest();
    }

    bool IsInRunningState() const
    {
        return !mutatorBase_->InSaferegion();
    }

    // Thread must be binded mutator before to allocate. Otherwise it cannot allocate heap object in this thread.
    // One thread only allow to bind one muatator. If try bind sencond mutator, will be fatal.
    void BindMutator();
    // One thread only allow to bind one muatator. So it must be unbinded mutator before bind another one.
    void UnbindMutator();
    // unify JSThread* and Coroutine*
    // When register a thread, it must be initialized, i.e. it's safe to visit GC-Root.
    void RegisterJSThread(JSThread *jsThread);
    void UnregisterJSThread(JSThread *jsThread);
    void RegisterCoroutine(Coroutine *coroutine);
    void UnregisterCoroutine(Coroutine *coroutine);
    void VisitAllThreads(CommonRootVisitor visitor);

    JSThread* GetJSThread() const
    {
        return jsThread_;
    }

    void *GetMutator() const
    {
        return mutatorBase_->mutator_;
    }

    GCPhase GetMutatorPhase() const
    {
        return mutatorBase_->GetMutatorPhase();
    }

    // Return if thread has already binded mutator.
    class TryBindMutatorScope {
    public:
        TryBindMutatorScope(ThreadHolder *holder);
        ~TryBindMutatorScope();

    private:
        ThreadHolder *holder_ {nullptr};
    };

private:
    ~ThreadHolder()
    {
        SetCurrent(nullptr);
    }

    // Return false if thread has already binded mutator. Otherwise bind a mutator.
    bool TryBindMutator();

    MutatorBase *mutatorBase_ {nullptr};

    // Access jsThreads/coroutines(iterate/insert/remove) must happen in RunningState from the currentThreadHolder, or
    // in SuspendAll from others, because daemon thread may iterate if in NativeState.
    // And if we use locks to make that thread safe, it would cause a AB-BA dead lock.
    JSThread *jsThread_ {nullptr};
    std::unordered_set<Coroutine *> coroutines_ {};

    NO_COPY_SEMANTIC_CC(ThreadHolder);
    NO_MOVE_SEMANTIC_CC(ThreadHolder);
    friend JSThread;
    friend ThreadHolderManager;
};
}  // namespace panda
#endif  // COMMON_INTERFACES_THREAD_THREAD_HOLDER_H
