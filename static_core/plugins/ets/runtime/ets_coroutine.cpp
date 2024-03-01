/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/panda_vm.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "intrinsics.h"

namespace ark::ets {

EtsCoroutine::EtsCoroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, PandaString name,
                           CoroutineContext *context, std::optional<EntrypointInfo> &&epInfo)
    : Coroutine(id, allocator, vm, ark::panda_file::SourceLang::ETS, std::move(name), context, std::move(epInfo))
{
    ASSERT(vm != nullptr);
}

PandaEtsVM *EtsCoroutine::GetPandaVM() const
{
    return static_cast<PandaEtsVM *>(GetVM());
}

CoroutineManager *EtsCoroutine::GetCoroutineManager() const
{
    return GetPandaVM()->GetCoroutineManager();
}

void EtsCoroutine::Initialize()
{
    auto allocator = GetVM()->GetHeapManager()->GetInternalAllocator();
    auto etsNapiEnv = PandaEtsNapiEnv::Create(this, allocator);
    if (!etsNapiEnv) {
        LOG(FATAL, RUNTIME) << "Cannot create PandaEtsNapiEnv: " << etsNapiEnv.Error();
    }
    etsNapiEnv_ = std::move(etsNapiEnv.Value());
    // Main EtsCoroutine is Initialized before class linker and promise_class_ptr_ will be set after creating the class
    if (HasManagedEntrypoint()) {
        promiseClassPtr_ = GetPandaVM()->GetClassLinker()->GetPromiseClass()->GetRuntimeClass();
        undefinedObj_ = GetPandaVM()->GetUndefinedObject();
        // NOTE (electronick, #15938): Refactor the managed class-related pseudo TLS fields
        // initialization in MT ManagedThread ctor and EtsCoroutine::Initialize
        auto *linkExt = GetPandaVM()->GetClassLinker()->GetEtsClassLinkerExtension();
        SetStringClassPtr(linkExt->GetClassRoot(ClassRoot::STRING));
        SetArrayU16ClassPtr(linkExt->GetClassRoot(ClassRoot::ARRAY_U16));
    }
    ASSERT(promiseClassPtr_ != nullptr || !HasManagedEntrypoint());

    Coroutine::Initialize();
}

void EtsCoroutine::FreeInternalMemory()
{
    etsNapiEnv_->FreeInternalMemory();
    ManagedThread::FreeInternalMemory();
}

void EtsCoroutine::RequestCompletion(Value returnValue)
{
    auto promiseRef = GetCompletionEvent()->GetPromise();
    auto promise = reinterpret_cast<EtsPromise *>(GetVM()->GetGlobalObjectStorage()->Get(promiseRef));
    if (promise != nullptr) {
        [[maybe_unused]] EtsHandleScope scope(this);
        EtsHandle<EtsPromise> hpromise(this, promise);
        EtsObject *retObject = nullptr;
        if (!HasPendingException()) {
            panda_file::Type returnType = GetReturnType();
            retObject = GetReturnValueAsObject(returnType, returnValue);
            if (retObject != nullptr) {
                if (returnType.IsVoid()) {
                    LOG(DEBUG, COROUTINES) << "Coroutine " << GetName() << " has completed";
                } else if (returnType.IsPrimitive()) {
                    LOG(DEBUG, COROUTINES) << "Coroutine " << GetName() << " has completed with return value 0x"
                                           << std::hex << returnValue.GetAs<uint64_t>();
                } else {
                    LOG(DEBUG, COROUTINES)
                        << "Coroutine " << GetName() << " has completed with return value = ObjectPtr<"
                        << returnValue.GetAs<ObjectHeader *>() << ">";
                }
            }
        }
        if (HasPendingException()) {
            // An exception may occur while boxin primitive return value in GetReturnValueAsObject
            auto *exc = GetException();
            ClearException();
            LOG(INFO, COROUTINES) << "Coroutine " << GetName()
                                  << " completed with an exception: " << exc->ClassAddr<Class>()->GetName();
            intrinsics::EtsPromiseReject(hpromise.GetPtr(), EtsObject::FromCoreType(exc));
        } else {
            intrinsics::EtsPromiseResolve(hpromise.GetPtr(), retObject);
        }
    } else {
        LOG(DEBUG, COROUTINES)
            << "Coroutine " << GetName()
            << " has completed, but the associated promise has been already collected by the GC. Exception thrown: "
            << HasPendingException();
    }
    Coroutine::RequestCompletion(returnValue);
}

panda_file::Type EtsCoroutine::GetReturnType()
{
    Method *entrypoint = GetManagedEntrypoint();
    ASSERT(entrypoint != nullptr);
    return entrypoint->GetReturnType();
}

EtsObject *EtsCoroutine::GetReturnValueAsObject(panda_file::Type returnType, Value returnValue)
{
    switch (returnType.GetId()) {
        case panda_file::Type::TypeId::VOID:
            LOG(FATAL, COROUTINES) << "Return type 'void' is not supported yet in 'launch' instruction";
            break;
        case panda_file::Type::TypeId::U1:
            return EtsBoxPrimitive<EtsBoolean>::Create(this, returnValue.GetAs<EtsBoolean>());
        case panda_file::Type::TypeId::I8:
            return EtsBoxPrimitive<EtsByte>::Create(this, returnValue.GetAs<EtsByte>());
        case panda_file::Type::TypeId::I16:
            return EtsBoxPrimitive<EtsShort>::Create(this, returnValue.GetAs<EtsShort>());
        case panda_file::Type::TypeId::U16:
            return EtsBoxPrimitive<EtsChar>::Create(this, returnValue.GetAs<EtsChar>());
        case panda_file::Type::TypeId::I32:
            return EtsBoxPrimitive<EtsInt>::Create(this, returnValue.GetAs<EtsInt>());
        case panda_file::Type::TypeId::F32:
            return EtsBoxPrimitive<EtsFloat>::Create(this, returnValue.GetAs<EtsFloat>());
        case panda_file::Type::TypeId::F64:
            return EtsBoxPrimitive<EtsDouble>::Create(this, returnValue.GetAs<EtsDouble>());
        case panda_file::Type::TypeId::I64:
            return EtsBoxPrimitive<EtsLong>::Create(this, returnValue.GetAs<EtsLong>());
        case panda_file::Type::TypeId::REFERENCE:
            return EtsObject::FromCoreType(returnValue.GetAs<ObjectHeader *>());
        default:
            LOG(FATAL, COROUTINES) << "Unsupported return type: " << returnType;
            break;
    }
    return nullptr;
}
}  // namespace ark::ets
