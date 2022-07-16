/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "runtime/include/locks.h"
#include "libpandabase/utils/logger.h"
#include "include/thread.h"

#include <memory>

namespace panda {

static bool is_initialized = false;

MutatorLock *Locks::mutator_lock = nullptr;
os::memory::Mutex *Locks::custom_tls_lock = nullptr;
os::memory::Mutex *Locks::user_suspension_lock = nullptr;

void Locks::Initialize()
{
    if (!is_initialized) {
        Locks::mutator_lock = new MutatorLock();
        Locks::custom_tls_lock = new os::memory::Mutex();
        Locks::user_suspension_lock = new os::memory::Mutex();
        is_initialized = true;
    }
}

#ifndef NDEBUG

void MutatorLock::ReadLock()
{
    ASSERT(!HasLock());
    os::memory::RWLock::ReadLock();
    LOG(DEBUG, RUNTIME) << "MutatorLock::ReadLock";
    Thread::GetCurrent()->SetLockState(RDLOCK);
}

void MutatorLock::WriteLock()
{
    ASSERT(!HasLock());
    os::memory::RWLock::WriteLock();
    LOG(DEBUG, RUNTIME) << "MutatorLock::WriteLock";
    Thread::GetCurrent()->SetLockState(WRLOCK);
}

bool MutatorLock::TryReadLock()
{
    bool ret = os::memory::RWLock::TryReadLock();
    LOG(DEBUG, RUNTIME) << "MutatorLock::TryReadLock";
    if (ret) {
        Thread::GetCurrent()->SetLockState(RDLOCK);
    }
    return ret;
}

bool MutatorLock::TryWriteLock()
{
    bool ret = os::memory::RWLock::TryWriteLock();
    LOG(DEBUG, RUNTIME) << "MutatorLock::TryWriteLock";
    if (ret) {
        Thread::GetCurrent()->SetLockState(WRLOCK);
    }
    return ret;
}

void MutatorLock::Unlock()
{
    ASSERT(HasLock());
    os::memory::RWLock::Unlock();
    LOG(DEBUG, RUNTIME) << "MutatorLock::Unlock";
    Thread::GetCurrent()->SetLockState(UNLOCKED);
}

MutatorLock::MutatorLockState MutatorLock::GetState()
{
    return Thread::GetCurrent()->GetLockState();
}

bool MutatorLock::HasLock()
{
    auto state = Thread::GetCurrent()->GetLockState();
    return state == RDLOCK || state == WRLOCK;
}
#endif  // !NDEBUG

}  // namespace panda
