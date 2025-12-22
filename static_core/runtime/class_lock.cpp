/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "class_lock.h"
#include "runtime/include/thread_scopes.h"

namespace ark {

ClassLock::ClassLock(const ObjectHeader *obj)
{
    cls_ = Class::FromClassObject(obj);
    auto *clsLinkerCtx = cls_->GetLoadContext();
    clsMtx_ = clsLinkerCtx->GetClassMutexHandler(cls_);

    ScopedChangeThreadStatus s(ManagedThread::GetCurrent(), ThreadStatus::IS_BLOCKED);
    clsMtx_->Lock();
}

bool ClassLock::Wait([[maybe_unused]] bool ignoreInterruption)
{
    ScopedChangeThreadStatus s(ManagedThread::GetCurrent(), ThreadStatus::IS_WAITING);
    clsMtx_->Wait();
    return true;
}

void ClassLock::Notify()
{
    clsMtx_->Signal();
}

void ClassLock::NotifyAll()
{
    clsMtx_->SignalAll();
}

ClassLock::~ClassLock()
{
    clsMtx_->Unlock();
}
}  // namespace ark
