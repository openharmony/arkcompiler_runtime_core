/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_INL_H
#define PANDA_RUNTIME_EXECUTION_JOB_INL_H

#include <variant>
#include "runtime/execution/job.h"

namespace ark {

inline bool Job::HasEntrypoint() const
{
    return !std::holds_alternative<std::monostate>(entrypoint_);
}

inline bool Job::HasManagedEntrypoint() const
{
    return std::holds_alternative<ManagedEntrypointInfo>(entrypoint_);
}

inline Method *Job::GetManagedEntrypoint() const
{
    ASSERT(HasManagedEntrypoint());
    return std::get<ManagedEntrypointInfo>(entrypoint_).entrypoint;
}

inline CompletionEvent *Job::GetCompletionEvent() const
{
    ASSERT(HasManagedEntrypoint());
    return std::get<ManagedEntrypointInfo>(entrypoint_).completionEvent;
}

inline PandaVector<Value> &Job::GetManagedEntrypointArguments()
{
    ASSERT(HasManagedEntrypoint());
    return std::get<ManagedEntrypointInfo>(entrypoint_).arguments;
}

inline const PandaVector<Value> &Job::GetManagedEntrypointArguments() const
{
    ASSERT(HasManagedEntrypoint());
    return std::get<ManagedEntrypointInfo>(entrypoint_).arguments;
}

inline bool Job::HasNativeEntrypoint() const
{
    return std::holds_alternative<NativeEntrypointInfo>(entrypoint_);
}

inline Job::NativeEntrypointInfo::NativeEntrypointFunc Job::GetNativeEntrypoint() const
{
    ASSERT(HasNativeEntrypoint());
    return std::get<NativeEntrypointInfo>(entrypoint_).entrypoint;
}

inline void *Job::GetNativeEntrypointParam() const
{
    ASSERT(HasNativeEntrypoint());
    return std::get<NativeEntrypointInfo>(entrypoint_).param;
}

inline void Job::ReplaceEntrypoint(EntrypointInfo &&epInfo)
{
    entrypoint_ = std::move(epInfo);
}

inline const PandaString &Job::GetName() const
{
    return name_;
}

inline Job::Id Job::GetId() const
{
    return id_;
}

inline JobPriority Job::GetPriority() const
{
    return priority_;
}

inline Job::Type Job::GetType() const
{
    return type_;
}

inline bool Job::HasAbortFlag() const
{
    return abortFlag_;
}

inline AffinityMask Job::GetAffinityMask() const
{
    return affinityMask_;
}

inline void Job::SetAffinityMask(const AffinityMask &mask)
{
    affinityMask_ = mask;
}

inline bool Job::IsMigrationAllowed() const
{
    return affinityMask_.NumAllowedWorkers() > 1;
}

inline Job::Status Job::GetStatus() const
{
    return status_;
}

inline void Job::SetStatus(Status newStatus)
{
    status_ = newStatus;
}

inline JobExecutionContext *Job::GetExecutionContext() const
{
    return executionCtx_;
}

inline void Job::SetExecutionContext(JobExecutionContext *executionCtx)
{
    ASSERT(executionCtx_ == nullptr || executionCtx == nullptr);
    executionCtx_ = executionCtx;
}

}  // namespace ark

#endif
