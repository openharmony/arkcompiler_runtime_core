/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "common_components/heap/collector/utils.h"

namespace ark::common_vm {

void ArrayTaskDispatcher::Dispatch(Taskpool *pool, int nThread)
{
    aliveTask_ = nThread;
    for (int i = 0; i < nThread; ++i) {
        pool->PostTask(std::make_unique<Runner>(this));
    }
}

void ArrayTaskDispatcher::RunImpl()
{
    void *end = reinterpret_cast<void *>(array_ + bytes_);
    // Atomic with relaxed order reason: data race with index_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    size_t i = index_.fetch_add(1, std::memory_order_relaxed);
    auto iter = TaskAt(i);
    while (iter < end) {
        consumer_->Run(iter, std::min(TaskAt(i + 1), end));
        // Atomic with relaxed order reason: data race with index_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        i = index_.fetch_add(1, std::memory_order_relaxed);
        iter = TaskAt(i);
    }
}

bool ArrayTaskDispatcher::Runner::Run(uint32_t)
{
    manager_->RunImpl();
    return true;
}

ArrayTaskDispatcher::Runner::~Runner()
{
    ark::os::memory::LockHolder lock(manager_->mtx_);
    if ((--manager_->aliveTask_) == 0) {
        manager_->cv_.SignalAll();
    }
}

void ArrayTaskDispatcher::JoinAndWait()
{
    RunImpl();
    ark::os::memory::LockHolder lock(mtx_);
    while (aliveTask_ != 0) {
        cv_.Wait(&mtx_);
    }
}

}  // namespace ark::common_vm
