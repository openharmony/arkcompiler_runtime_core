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

#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ani/verify/ani_verifier.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/mem/ets_reference.h"
#include "runtime/include/cframe.h"
#include "runtime/include/stack_walker-inl.h"

namespace ark::ets::ani::verify {

VRef *ANIVerifier::AddGlobalVerifiedRef(ani_ref gref)
{
    auto igrefHolder = MakePandaUnique<InternalRef>(gref);
    auto *igref = igrefHolder.get();
    auto vgref = InternalRef::CastToVRef(igref);
    {
        os::memory::LockHolder<os::memory::Mutex> lockHolder(grefsMapMutex_);
        grefsMap_[vgref] = std::move(igrefHolder);
    }
    return vgref;
}

void ANIVerifier::DeleteGlobalVerifiedRef(VRef *vgref)
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

impl::VMethod *ANIVerifier::AddMethod(EtsMethod *method)
{
    auto vmethodHolder = MakePandaUnique<impl::VMethod>(method);
    impl::VMethod *vmethod = vmethodHolder.get();
    {
        os::memory::WriteLockHolder lock(methodsSetLock_);

        methodsSet_[vmethod] = std::move(vmethodHolder);
    }
    return vmethod;
}

void ANIVerifier::DeleteMethod(impl::VMethod *vmethod)
{
    os::memory::WriteLockHolder lock(methodsSetLock_);

    auto it = methodsSet_.find(vmethod);
    ASSERT(it != methodsSet_.cend());
    methodsSet_.erase(it);
}

bool ANIVerifier::IsValidVerifiedMethod(impl::VMethod *vmethod)
{
    os::memory::ReadLockHolder lock(methodsSetLock_);

    return methodsSet_.find(vmethod) != methodsSet_.cend();
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

bool ANIVerifier::IsValidStackRef(VRef *vref)
{
    if (InternalRef::IsUndefinedStackRef(vref)) {
        return true;
    }

    if (!InternalRef::IsStackVRef(vref)) {
        return false;
    }

    auto stack = StackWalker::Create(ManagedThread::GetCurrent());
    if (!stack.HasFrame()) {
        return false;
    }
    size_t refIndex = 0;
    auto &cframe = stack.GetCFrame();
    bool notEqual = stack.IterateObjectsWithInfo([vref, &refIndex, &cframe, &stack](auto &regInfo, auto &stackReg) {
        if (EtsMethod::FromRuntimeMethod(stack.GetMethod())->IsFunction() && refIndex++ == 0) {
            [[maybe_unused]] auto stackEtsObj = EtsObject::FromCoreType(stackReg.GetReference());
            ASSERT(EtsReferenceStorage::IsUndefinedEtsObject(stackEtsObj));
            return true;
        }
        uintptr_t argRefAddr = reinterpret_cast<uintptr_t>(vref);
        uintptr_t stackRefAddr = reinterpret_cast<uintptr_t>(cframe.GetValuePtrFromSlot(regInfo.GetValue()));
        return (argRefAddr != stackRefAddr);
    });
    return !notEqual;
}

}  // namespace ark::ets::ani::verify
