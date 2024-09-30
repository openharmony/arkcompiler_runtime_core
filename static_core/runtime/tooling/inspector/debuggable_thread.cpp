/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "runtime/tooling/inspector/debuggable_thread.h"

namespace ark::tooling::inspector {
DebuggableThread::DebuggableThread(ManagedThread *thread, SuspensionCallbacks &&callbacks)
    : thread_(thread), callbacks_(std::move(callbacks))
{
    ASSERT(thread);
}

void DebuggableThread::Reset()
{
    os::memory::LockHolder lock(mutex_);
    state_.Reset();
}

void DebuggableThread::BreakOnStart()
{
    os::memory::LockHolder lock(mutex_);
    state_.BreakOnStart();
}

void DebuggableThread::Continue()
{
    os::memory::LockHolder lock(mutex_);
    state_.Continue();
    Resume();
}

void DebuggableThread::ContinueTo(std::unordered_set<PtLocation, HashLocation> locations)
{
    os::memory::LockHolder lock(mutex_);
    state_.ContinueTo(std::move(locations));
    Resume();
}

void DebuggableThread::StepInto(std::unordered_set<PtLocation, HashLocation> locations)
{
    os::memory::LockHolder lock(mutex_);
    state_.StepInto(std::move(locations));
    Resume();
}

void DebuggableThread::StepOver(std::unordered_set<PtLocation, HashLocation> locations)
{
    os::memory::LockHolder lock(mutex_);
    state_.StepOver(std::move(locations));
    Resume();
}

void DebuggableThread::StepOut()
{
    os::memory::LockHolder lock(mutex_);
    state_.StepOut();
    Resume();
}

bool DebuggableThread::IsPaused()
{
    os::memory::LockHolder lock(mutex_);
    return suspended_;
}

void DebuggableThread::Touch()
{
    os::memory::LockHolder lock(mutex_);
    Resume();
}

void DebuggableThread::Pause()
{
    os::memory::LockHolder lock(mutex_);
    state_.Pause();
}

void DebuggableThread::SetBreakpointsActive(bool active)
{
    os::memory::LockHolder lock(mutex_);
    state_.SetBreakpointsActive(active);
}

BreakpointId DebuggableThread::SetBreakpoint(const std::vector<PtLocation> &locations)
{
    os::memory::LockHolder lock(mutex_);
    return state_.SetBreakpoint(locations);
}

void DebuggableThread::RemoveBreakpoint(BreakpointId id)
{
    os::memory::LockHolder lock(mutex_);
    state_.RemoveBreakpoint(id);
}

void DebuggableThread::SetPauseOnExceptions(PauseOnExceptionsState state)
{
    os::memory::LockHolder lock(mutex_);
    state_.SetPauseOnExceptions(state);
}

bool DebuggableThread::RequestToObjectRepository(std::function<void(ObjectRepository &)> request)
{
    os::memory::LockHolder lock(mutex_);

    ASSERT(!request_.has_value());

    if (!suspended_) {
        return false;
    }

    ASSERT(thread_->IsSuspended());

    request_ = std::move(request);
    thread_->Resume();

    while (request_) {
        requestDone_.Wait(&mutex_);
    }

    ASSERT(suspended_);
    ASSERT(thread_->IsSuspended());

    return true;
}

std::pair<std::optional<RemoteObject>, std::optional<RemoteObject>> DebuggableThread::Evaluate(
    const std::string &bcFragment)
{
    std::optional<RemoteObject> optResult;
    std::optional<RemoteObject> optException;

    RequestToObjectRepository([this, &bcFragment, &optResult, &optException](ObjectRepository &objectRepo) {
        ASSERT(thread_->GetCurrentFrame());
        auto *ctx = thread_->GetCurrentFrame()->GetMethod()->GetClass()->GetLoadContext();

        auto *method = LoadEvaluationPatch(ctx, bcFragment);
        if (method == nullptr) {
            return;
        }

        auto [result, exception] = InvokeEvaluationMethod(method);

        if (exception != nullptr) {
            optException.emplace(objectRepo.CreateObject(TypedValue::Reference(exception)));
        }

        auto resultTypeId = method->GetReturnType().GetId();
        optResult.emplace(objectRepo.CreateObject(result, resultTypeId));
    });

    return std::make_pair(optResult, optException);
}

std::pair<Value, ObjectHeader *> DebuggableThread::InvokeEvaluationMethod(Method *method)
{
    ASSERT(method);
    ASSERT(!thread_->HasPendingException());

    // Some debugger functionality must be disabled in evaluation mode.
    evaluationMode_ = true;
    // Evaluation patch should not accept any arguments.
    Value result = method->Invoke(thread_, nullptr);
    evaluationMode_ = false;

    ObjectHeader *newException = nullptr;
    if (thread_->HasPendingException()) {
        newException = thread_->GetException();
        LOG(WARNING, DEBUGGER) << "Evaluation has completed with a exception "
                               << newException->ClassAddr<Class>()->GetName();
        // Restore the previous exception.
        thread_->SetException(nullptr);
    }

    return std::make_pair(result, newException);
}

void DebuggableThread::OnException(bool uncaught)
{
    if (evaluationMode_) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    state_.OnException(uncaught);
    while (state_.IsPaused()) {
        ObjectRepository objectRepository;
        Suspend(objectRepository, {}, thread_->GetException());
    }
}

void DebuggableThread::OnFramePop()
{
    if (evaluationMode_) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    state_.OnFramePop();
}

bool DebuggableThread::OnMethodEntry()
{
    if (evaluationMode_) {
        return false;
    }
    os::memory::LockHolder lock(mutex_);
    return state_.OnMethodEntry();
}

void DebuggableThread::OnSingleStep(const PtLocation &location)
{
    if (evaluationMode_) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    state_.OnSingleStep(location);
    while (state_.IsPaused()) {
        ObjectRepository objectRepository;
        auto hitBreakpoints = state_.GetBreakpointsByLocation(location);
        Suspend(objectRepository, hitBreakpoints, {});
    }
}

std::vector<RemoteObject> DebuggableThread::OnConsoleCall(const PandaVector<TypedValue> &arguments)
{
    std::vector<RemoteObject> result;

    ObjectRepository objectRepository;
    std::transform(arguments.begin(), arguments.end(), std::back_inserter(result),
                   [&objectRepository](auto value) { return objectRepository.CreateObject(value); });

    return result;
}

void DebuggableThread::Suspend(ObjectRepository &objectRepository, const std::vector<BreakpointId> &hitBreakpoints,
                               ObjectHeader *exception)
{
    ASSERT(ManagedThread::GetCurrent() == thread_);

    ASSERT(!suspended_);
    ASSERT(!request_.has_value());

    callbacks_.preSuspend(objectRepository, hitBreakpoints, exception);

    suspended_ = true;
    thread_->Suspend();

    callbacks_.postSuspend(objectRepository, hitBreakpoints, exception);

    while (suspended_) {
        mutex_.Unlock();

        callbacks_.preWaitSuspension();
        thread_->WaitSuspension();
        callbacks_.postWaitSuspension();

        mutex_.Lock();

        // We have three levels of suspension:
        // - state_.IsPaused() - tells if the thread is paused on breakpoint or step;
        // - suspended_ - tells if the thread generally sleeps (but could execute object repository requests);
        // - thread_->IsSuspended() - tells if the thread is actually sleeping
        //
        // If we are here, then the latter is false (thread waked up). The following variants are possible:
        // - not paused and not suspended - e.g. continue / stepping action was performed;
        // - not paused and suspended - invalid;
        // - paused and not suspended - touch was performed, resume and sleep back
        //     (used to notify about the state on `runIfWaitingForDebugger`);
        // - paused and suspended - object repository request was made.

        ASSERT(suspended_ == request_.has_value());

        if (request_) {
            (*request_)(objectRepository);

            request_.reset();
            thread_->Suspend();

            requestDone_.Signal();
        }
    }
}

void DebuggableThread::Resume()
{
    ASSERT(!request_.has_value());

    if (!suspended_) {
        return;
    }

    ASSERT(thread_->IsSuspended());

    callbacks_.preResume();

    suspended_ = false;
    thread_->Resume();

    callbacks_.postResume();
}
}  // namespace ark::tooling::inspector
