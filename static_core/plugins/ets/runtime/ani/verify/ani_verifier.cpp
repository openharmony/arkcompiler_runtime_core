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

#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ani/verify/ani_verifier.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/mem/ets_reference.h"
#include "plugins/ets/runtime/ani/ani_type_check.h"
#include "runtime/include/cframe.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/include/mutator.h"

namespace ark::ets::ani::verify {

VRef *ANIVerifier::AddGlobalVerifiedRef(ani_ref gref)
{
    auto igrefHolder = MakePandaUnique<InternalRef>(gref);
    auto *igref = igrefHolder.get();
    auto vgref = InternalRef::CastToVRef(igref);
    {
        auto &grefs = GetGlobalData().grefsMap;
        os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().grefsMapMutex);
        grefs[vgref] = std::move(igrefHolder);
    }
    return vgref;
}

void ANIVerifier::DeleteGlobalVerifiedRef(VRef *vgref)
{
    auto &grefs = GetGlobalData().grefsMap;
    os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().grefsMapMutex);
    auto it = grefs.find(vgref);
    ASSERT(it != grefs.cend());
    grefs.erase(it);
}

bool ANIVerifier::IsValidGlobalVerifiedRef(VRef *vgref)
{
    auto &grefs = GetGlobalData().grefsMap;
    os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().grefsMapMutex);
    return grefs.find(vgref) != grefs.cend();
}

VWRef *ANIVerifier::AddVerifiedWeakRef(ani_wref wref)
{
    auto iwrefHolder = MakePandaUnique<InternalRef>(wref);
    auto *iref = iwrefHolder.get();
    auto *vwref = InternalRef::CastToVWRef(iref);
    {
        auto &wrefs = GetGlobalData().wrefsMap;
        os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().wrefsMapMutex);
        wrefs[vwref] = std::move(iwrefHolder);
    }
    return vwref;
}

void ANIVerifier::DeleteVerifiedWeakRef(VWRef *vwref)
{
    auto &wrefs = GetGlobalData().wrefsMap;
    os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().wrefsMapMutex);
    auto it = wrefs.find(vwref);
    ASSERT(it != wrefs.cend());
    wrefs.erase(it);
}

bool ANIVerifier::IsValidWeakRef(VWRef *vwref)
{
    if (vwref == nullptr) {
        return false;
    }
    auto &wrefs = GetGlobalData().wrefsMap;
    os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().wrefsMapMutex);
    return wrefs.find(vwref) != wrefs.cend();
}

impl::VMethod *ANIVerifier::AddMethod(EtsMethod *method)
{
    os::memory::WriteLockHolder lock(GetGlobalData().methodsMapLock);

    auto &etsMap = GetGlobalData().etsMethodsMap;
    auto it = etsMap.find(method);
    if (it != etsMap.end()) {
        return it->second;
    }

    auto vmethodHolder = MakePandaUnique<impl::VMethod>(method);
    impl::VMethod *vmethod = vmethodHolder.get();
    etsMap[method] = vmethod;

    GetGlobalData().methodsMap[vmethod] = std::move(vmethodHolder);
    return vmethod;
}

void ANIVerifier::DeleteMethod(impl::VMethod *vmethod)
{
    os::memory::WriteLockHolder lock(GetGlobalData().methodsMapLock);

    auto it = GetGlobalData().methodsMap.find(vmethod);
    if (it == GetGlobalData().methodsMap.cend()) {
        LOG(ERROR, RUNTIME) << "Attempted to delete non-existent Method: " << vmethod;
        return;
    }
    ASSERT(it != GetGlobalData().methodsMap.cend());
    EtsMethod *etsMethod = vmethod->GetEtsMethod();
    auto &etsMap = GetGlobalData().etsMethodsMap;
    auto etsIt = etsMap.find(etsMethod);
    if (etsIt != etsMap.end() && etsIt->second == vmethod) {
        etsMap.erase(etsIt);
    }
    GetGlobalData().methodsMap.erase(it);
}

bool ANIVerifier::IsValidMethod(impl::VMethod *vmethod)
{
    os::memory::ReadLockHolder lock(GetGlobalData().methodsMapLock);

    return GetGlobalData().methodsMap.find(vmethod) != GetGlobalData().methodsMap.cend();
}

impl::VField *ANIVerifier::AddField(EtsField *field)
{
    os::memory::WriteLockHolder lock(GetGlobalData().fieldsMapLock);

    auto &etsMap = GetGlobalData().etsFieldsMap;
    auto it = etsMap.find(field);
    if (it != etsMap.end()) {
        return it->second;
    }
    auto vfieldHolder = MakePandaUnique<impl::VField>(field);
    impl::VField *vfield = vfieldHolder.get();
    etsMap[field] = vfield;
    GetGlobalData().fieldsMap[vfield] = std::move(vfieldHolder);
    return vfield;
}

void ANIVerifier::DeleteField(impl::VField *vfield)
{
    os::memory::WriteLockHolder lock(GetGlobalData().fieldsMapLock);

    auto it = GetGlobalData().fieldsMap.find(vfield);
    if (it == GetGlobalData().fieldsMap.cend()) {
        LOG(ERROR, RUNTIME) << "Attempted to delete non-existent Field: " << vfield;
        return;
    }
    ASSERT(it != GetGlobalData().fieldsMap.cend());
    EtsField *etsField = vfield->GetEtsField();
    auto &etsMap = GetGlobalData().etsFieldsMap;
    auto etsIt = etsMap.find(etsField);
    if (etsIt != etsMap.end() && etsIt->second == vfield) {
        etsMap.erase(etsIt);
    }

    GetGlobalData().fieldsMap.erase(it);
}

bool ANIVerifier::IsValidField(impl::VField *vfield)
{
    os::memory::ReadLockHolder lock(GetGlobalData().fieldsMapLock);

    return GetGlobalData().fieldsMap.find(vfield) != GetGlobalData().fieldsMap.cend();
}

VResolver *ANIVerifier::AddGlobalVerifiedResolver(ani_resolver resolver)
{
    auto vresolverHolder = MakePandaUnique<VResolver>(resolver);
    VResolver *vresolver = vresolverHolder.get();
    {
        auto &grefs = GetGlobalData().resolversMap;
        os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().resolverMapMutex);
        grefs[vresolver] = std::move(vresolverHolder);
    }
    return vresolver;
}

void ANIVerifier::DeleteGlobalResolver(VResolver *vresolver)
{
    auto &resolvers = GetGlobalData().resolversMap;
    os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().resolverMapMutex);
    auto it = resolvers.find(vresolver);
    ASSERT(it != resolvers.cend());
    resolvers.erase(it);
}

bool ANIVerifier::IsValidGlobalResolver(VResolver *vresolver)
{
    auto &resolvers = GetGlobalData().resolversMap;
    os::memory::LockHolder<os::memory::Mutex> lockHolder(GetGlobalData().resolverMapMutex);
    return resolvers.find(vresolver) != resolvers.cend();
}

void ANIVerifier::Report(const std::string_view message, bool isFatal)
{
    if (IsWorkaroundNoCrashIfInvalidUsage() || !isFatal) {
        ANIVerifier::Error(message);
    } else {
        ANIVerifier::Abort(message);
    }
}

namespace {
PandaStringStream BuildMessageWithStackTrace(const std::string_view message)
{
    PandaStringStream ss;
    ss << message << "\n";

    Mutator *mutator = Mutator::GetCurrent();
    EtsExecutionContext *executionCtx = nullptr;
    if (mutator != nullptr) {
        executionCtx = EtsExecutionContext::GetCurrent();
    }
    if (executionCtx != nullptr) {
        ss << "  Called from:";
        StackWalker stack = StackWalker::Create(executionCtx->GetMT());
        for (; stack.HasFrame(); stack.NextFrame()) {
            Method *method = stack.GetMethod();
            ss << "\n    " << method->GetClass()->GetName() << "." << method->GetName().data << " at "
               << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
        }
    } else {
        ss << "  Called from:\n";
        ss << "    '[native]'";
    }
    return ss;
}
}  // namespace

void ANIVerifier::Error(const std::string_view message)
{
    auto ss = BuildMessageWithStackTrace(message);
    if (errorHook_ != nullptr) {
        errorHook_(errorHookData_, ss.str());
    } else {
        LOG(ERROR, ANI) << ss.str();
    }
}

void ANIVerifier::Abort(const std::string_view message)
{
    auto ss = BuildMessageWithStackTrace(message);
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
        if (IsFunction(EtsMethod::FromRuntimeMethod(stack.GetMethod())) && refIndex++ == 0) {
            [[maybe_unused]] auto stackEtsObj = EtsObject::FromCoreType(stackReg.GetReference());
            ASSERT(EtsReferenceStorage::IsUndefinedEtsObject(stackEtsObj));
            return true;
        }
        auto argRefAddr = reinterpret_cast<uintptr_t>(vref);
        auto stackRefAddr = reinterpret_cast<uintptr_t>(cframe.GetValuePtrFromSlot(regInfo.GetValue()));
        return (argRefAddr != stackRefAddr);
    });
    return !notEqual;
}

}  // namespace ark::ets::ani::verify
