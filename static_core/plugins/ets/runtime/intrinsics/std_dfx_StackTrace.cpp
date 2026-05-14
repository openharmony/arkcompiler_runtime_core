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

#include <optional>
#include "ets_handle.h"
#include "include/mem/panda_containers.h"
#include "runtime/runtime_helpers.h"
#include "types/ets_method.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "include/panda_vm.h"
#include "mem/rendezvous.h"

#include "runtime/include/stack_walker.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"

#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"

namespace ark::ets::intrinsics {

static std::optional<std::string> CreateStackTraceElementString(StackWalker *stack)
{
    helpers::StackTraceElementInfo info;
    if (!GetStackTraceElementBasicInfo(stack, &info)) {
        LOG(ERROR, STDLIB) << "Found method from stack is nullptr.";
        return std::nullopt;
    }

    std::ostringstream oss;
    oss << "\tat " << info.className << "." << info.methodName << " (" << info.sourceFile << ":" << info.lineNumber
        << ":0)\n";

    return oss.str();
}

static std::string BuildMainThreadStackTrace(ManagedThread *mainThread)
{
    std::string result;
    if (mainThread == nullptr) {
        return result;
    }
    for (auto stack = StackWalker::Create(mainThread); stack.HasFrame(); stack.NextFrame()) {
        std::optional<std::string> backTrace = CreateStackTraceElementString(&stack);
        if (backTrace.has_value()) {
            result.append(backTrace.value());
        }
    }
    return result;
}

extern "C" EtsString *StackTraceGetMainThreadStackTrace()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    JobManager *jobManager = executionCtx->GetPandaVM()->GetJobManager();
    if (jobManager == nullptr) {
        return EtsString::CreateFromMUtf8("");
    }

    auto *mt = jobManager->GetMainThread();
    if (mt == nullptr) {
        return EtsString::CreateFromMUtf8("");
    }

    std::optional<ScopedSuspendAllThreadsRunning> ssat;
    if (jobManager->GetMainThread() != executionCtx->GetMT()) {
        auto *rendezvous = executionCtx->GetPandaVM()->GetRendezvous();
        ssat.emplace(rendezvous);
    }

    std::string trace = BuildMainThreadStackTrace(mt);
    return EtsString::CreateFromMUtf8(trace.c_str());
}

}  // namespace ark::ets::intrinsics
