/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "verification/jobs/service.h"

#include "runtime/mem/allocator_adapter.h"

namespace ark::verifier {

void VerifierService::Init() {}

void VerifierService::Destroy()
{
    ark::os::memory::LockHolder lck {lock_};
    if (shutdown_) {
        return;
    }
    shutdown_ = true;

    for (auto &it : processors_) {
        auto *loadContextData = &it.second;
        // Wait for ongoing verifications to finish
        while (loadContextData->totalProcessors < loadContextData->queue.size()) {
            condVar_.Wait(&lock_);
        }
        for (auto *processor : loadContextData->queue) {
            allocator_->Delete<TaskProcessor>(processor);
        }
    }
}

TaskProcessor *VerifierService::GetProcessor(ClassLinkerContext *ctx)
{
    ark::os::memory::LockHolder lck {lock_};
    if (shutdown_) {
        return nullptr;
    }
    if (processors_.count(ctx) == 0) {
        processors_.emplace(ctx, ctx);
    }
    LoadContextData *loadContextData = &processors_.at(ctx);
    if (loadContextData->queue.empty()) {
        loadContextData->queue.push_back(allocator_->New<TaskProcessor>(this, ctx));
        loadContextData->totalProcessors++;
    }
    // NOTE(gogabr): should we use a queue or stack discipline?
    auto res = loadContextData->queue.front();
    loadContextData->queue.pop_front();
    return res;
}

void VerifierService::ReleaseProcessor(TaskProcessor *processor)
{
    ark::os::memory::LockHolder lck {lock_};
    ASSERT(processors_.count(processor->GetLoadContext()) > 0);
    processors_.at(processor->GetLoadContext()).queue.push_back(processor);
    condVar_.Signal();
}

}  // namespace ark::verifier
