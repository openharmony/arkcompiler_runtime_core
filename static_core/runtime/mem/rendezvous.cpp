/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "runtime/mem/rendezvous.h"

#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark {

Rendezvous::Rendezvous(PandaVM *pandaVm) : mutatorLock_(pandaVm->GetMutatorLock()) {}

void Rendezvous::SafepointBegin()
{
    auto mutator = Mutator::GetCurrent();
    ASSERT(mutator != nullptr);
    ASSERT(!mutatorLock_->HasLock());
    LOG(DEBUG, GC) << "Rendezvous: SafepointBegin";
    // Suspend all mutators
    mutator->GetVM()->GetMutatorManager()->SuspendAllMutators();
    // Acquire write MutatorLock
    mutatorLock_->WriteLock();
}

void Rendezvous::SafepointEnd()
{
    auto mutator = Mutator::GetCurrent();
    ASSERT(mutator != nullptr);
    ASSERT(mutatorLock_->HasLock());
    LOG(DEBUG, GC) << "Rendezvous: SafepointEnd";
    // Release write MutatorLock
    mutatorLock_->Unlock();
    // Resume all mutators
    mutator->GetVM()->GetMutatorManager()->ResumeAllMutators();
    LOG(DEBUG, GC) << "Rendezvous: SafepointEnd exit";
}

ScopedStopTheWorld::ScopedStopTheWorld() : rendezvous_(PandaVM::GetCurrent()->GetRendezvous())
{
    ASSERT(rendezvous_ != nullptr);
    rendezvous_->SafepointBegin();
}

ScopedSuspendAllThreadsRunning::ScopedSuspendAllThreadsRunning(Rendezvous *rendezvous) : rendezvous_(rendezvous)
{
    ASSERT(rendezvous_ != nullptr);
    ASSERT(rendezvous_->GetMutatorLock()->HasLock());
    rendezvous_->GetMutatorLock()->Unlock();
    rendezvous_->SafepointBegin();
}

ScopedSuspendAllThreadsRunning::~ScopedSuspendAllThreadsRunning()
{
    rendezvous_->SafepointEnd();
    rendezvous_->GetMutatorLock()->ReadLock();
}

}  // namespace ark
