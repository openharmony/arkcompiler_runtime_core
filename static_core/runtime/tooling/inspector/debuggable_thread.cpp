/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

namespace panda::tooling::inspector {
DebuggableThread::DebuggableThread(
    ManagedThread *thread,
    std::function<void(ObjectRepository &, const std::vector<BreakpointId> &, ObjectHeader *)> &&pre_suspend,
    std::function<void(ObjectRepository &, const std::vector<BreakpointId> &, ObjectHeader *)> &&post_suspend,
    std::function<void()> &&pre_wait_suspension, std::function<void()> &&post_wait_suspension,
    std::function<void()> &&pre_resume, std::function<void()> &&post_resume)
    : thread_(thread),
      pre_suspend_(std::move(pre_suspend)),
      post_suspend_(std::move(post_suspend)),
      pre_wait_suspension_(std::move(pre_wait_suspension)),
      post_wait_suspension_(std::move(post_wait_suspension)),
      pre_resume_(std::move(pre_resume)),
      post_resume_(std::move(post_resume))
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
        request_done_.Wait(&mutex_);
    }

    ASSERT(suspended_);
    ASSERT(thread_->IsSuspended());

    return true;
}

void DebuggableThread::OnException(bool uncaught)
{
    os::memory::LockHolder lock(mutex_);
    state_.OnException(uncaught);
    while (state_.IsPaused()) {
        ObjectRepository object_repository;
        Suspend(object_repository, {}, thread_->GetException());
    }
}

void DebuggableThread::OnFramePop()
{
    os::memory::LockHolder lock(mutex_);
    state_.OnFramePop();
}

bool DebuggableThread::OnMethodEntry()
{
    os::memory::LockHolder lock(mutex_);
    return state_.OnMethodEntry();
}

void DebuggableThread::OnSingleStep(const PtLocation &location)
{
    os::memory::LockHolder lock(mutex_);
    state_.OnSingleStep(location);
    while (state_.IsPaused()) {
        ObjectRepository object_repository;
        auto hit_breakpoints = state_.GetBreakpointsByLocation(location);
        Suspend(object_repository, hit_breakpoints, {});
    }
}

std::vector<RemoteObject> DebuggableThread::OnConsoleCall(const PandaVector<TypedValue> &arguments)
{
    std::vector<RemoteObject> result;

    ObjectRepository object_repository;
    std::transform(arguments.begin(), arguments.end(), std::back_inserter(result),
                   [&object_repository](auto value) { return object_repository.CreateObject(value); });

    return result;
}

void DebuggableThread::Suspend(ObjectRepository &object_repository, const std::vector<BreakpointId> &hit_breakpoints,
                               ObjectHeader *exception)
{
    ASSERT(ManagedThread::GetCurrent() == thread_);

    ASSERT(!suspended_);
    ASSERT(!request_.has_value());

    pre_suspend_(object_repository, hit_breakpoints, exception);

    suspended_ = true;
    thread_->Suspend();

    post_suspend_(object_repository, hit_breakpoints, exception);

    while (suspended_) {
        mutex_.Unlock();

        pre_wait_suspension_();
        thread_->WaitSuspension();
        post_wait_suspension_();

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
            (*request_)(object_repository);

            request_.reset();
            thread_->Suspend();

            request_done_.Signal();
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

    pre_resume_();

    suspended_ = false;
    thread_->Resume();

    post_resume_();
}
}  // namespace panda::tooling::inspector
