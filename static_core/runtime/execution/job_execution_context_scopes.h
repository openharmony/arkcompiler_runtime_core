/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_EXECUTION_JOB_EXECUTION_CONTEXT_SCOPES_H
#define PANDA_RUNTIME_EXECUTION_JOB_EXECUTION_CONTEXT_SCOPES_H

#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_manager.h"

namespace ark {

class ScopedExecutionContextNativeCall {
public:
    explicit ScopedExecutionContextNativeCall(JobExecutionContext *execCtx) : saved_(execCtx)
    {
        saved_->GetManager()->OnNativeCallEnter(saved_);
    }

    ~ScopedExecutionContextNativeCall()
    {
        saved_->GetManager()->OnNativeCallExit(saved_);
    }

private:
    NO_COPY_SEMANTIC(ScopedExecutionContextNativeCall);
    NO_MOVE_SEMANTIC(ScopedExecutionContextNativeCall);

    JobExecutionContext *saved_ = nullptr;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_EXECUTION_JOB_EXECUTION_CONTEXT_SCOPES_H */