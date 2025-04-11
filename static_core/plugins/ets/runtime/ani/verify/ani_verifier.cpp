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

#include "plugins/ets/runtime/ani/verify/ani_verifier.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets::ani::verify {

VRef *ANIVerifier::AddGloablVerifiedRef(ani_ref gref)
{
    auto vrefHolder = MakePandaUnique<VRef>(gref);
    auto *vref = vrefHolder.get();
    {
        os::memory::LockHolder<os::memory::Mutex> lockHolder(grefsMapMutex_);
        grefsMap_[vref] = std::move(vrefHolder);
    }
    return vref;
}

void ANIVerifier::DeleteDeleteGlobalRef(VRef *vgref)
{
    os::memory::LockHolder<os::memory::Mutex> lockHolder(grefsMapMutex_);
    auto it = grefsMap_.find(vgref);
    ASSERT(it != grefsMap_.cend());
    grefsMap_.erase(it);
}

bool ANIVerifier::IsValidGlobalVerifiedRef(VRef *vgref)
{
    os::memory::LockHolder<os::memory::Mutex> lockHolder(grefsMapMutex_);
    return grefsMap_.find(vgref) != grefsMap_.cend();
}

void ANIVerifier::Abort(const std::string_view message)
{
    PandaStringStream ss;
    ss << message << "\n";

    EtsCoroutine *coro = nullptr;
    Thread *thread = Thread::GetCurrent();
    if (thread != nullptr) {
        coro = EtsCoroutine::GetCurrent();
    }
    if (coro != nullptr) {
        ss << "  Called from:";
        StackWalker stack = StackWalker::Create(coro);
        for (; stack.HasFrame(); stack.NextFrame()) {
            Method *method = stack.GetMethod();
            ss << "\n    " << method->GetClass()->GetName() << "." << method->GetName().data << " at "
               << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
        }
    } else {
        ss << "  Called from:\n";
        ss << "    '[native]'";
    }

    if (abortHook_ != nullptr) {
        abortHook_(abortHookData_, ss.str());
    } else {
        LOG(FATAL, ANI) << ss.str();
    }
}

}  // namespace ark::ets::ani::verify
