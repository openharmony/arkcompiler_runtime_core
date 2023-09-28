/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

namespace panda::ets {

EtsCoroutine::EtsCoroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, PandaString name,
                           CoroutineContext *context, std::optional<EntrypointInfo> &&ep_info)
    : Coroutine(id, allocator, vm, panda::panda_file::SourceLang::ETS, std::move(name), context, std::move(ep_info))
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
    auto ets_napi_env = PandaEtsNapiEnv::Create(this, allocator);
    if (!ets_napi_env) {
        LOG(FATAL, RUNTIME) << "Cannot create PandaEtsNapiEnv: " << ets_napi_env.Error();
    }
    ets_napi_env_ = std::move(ets_napi_env.Value());
    // Main EtsCoroutine is Initialized before class linker and promise_class_ptr_ will be set after creating the class
    if (HasManagedEntrypoint()) {
        promise_class_ptr_ = GetPandaVM()->GetClassLinker()->GetPromiseClass()->GetRuntimeClass();
    }
    ASSERT(promise_class_ptr_ != nullptr || !HasManagedEntrypoint());

    Coroutine::Initialize();
}

void EtsCoroutine::FreeInternalMemory()
{
    ets_napi_env_->FreeInternalMemory();
    ManagedThread::FreeInternalMemory();
}

void EtsCoroutine::RequestCompletion(Value return_value)
{
    auto promise_ref = GetCompletionEvent()->GetPromise();
    auto promise = reinterpret_cast<EtsPromise *>(GetVM()->GetGlobalObjectStorage()->Get(promise_ref));
    if (promise != nullptr) {
        [[maybe_unused]] EtsHandleScope scope(this);
        EtsHandle<EtsPromise> hpromise(this, promise);
        EtsObject *ret_object = nullptr;
        if (!HasPendingException()) {
            panda_file::Type return_type = GetReturnType();
            ret_object = GetReturnValueAsObject(return_type, return_value);
            if (ret_object != nullptr) {
                if (return_type.IsVoid()) {
                    LOG(DEBUG, COROUTINES) << "Coroutine " << GetName() << " has completed";
                } else if (return_type.IsPrimitive()) {
                    LOG(DEBUG, COROUTINES) << "Coroutine " << GetName() << " has completed with return value 0x"
                                           << std::hex << return_value.GetAs<uint64_t>();
                } else {
                    LOG(DEBUG, COROUTINES)
                        << "Coroutine " << GetName() << " has completed with return value = ObjectPtr<"
                        << return_value.GetAs<ObjectHeader *>() << ">";
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
            intrinsics::EtsPromiseResolve(hpromise.GetPtr(), ret_object);
        }
    } else {
        LOG(DEBUG, COROUTINES)
            << "Coroutine " << GetName()
            << " has completed, but the associated promise has been already collected by the GC. Exception thrown: "
            << HasPendingException();
    }
    Coroutine::RequestCompletion(return_value);
}

panda_file::Type EtsCoroutine::GetReturnType()
{
    Method *entrypoint = GetManagedEntrypoint();
    ASSERT(entrypoint != nullptr);
    return entrypoint->GetReturnType();
}

EtsObject *EtsCoroutine::GetReturnValueAsObject(panda_file::Type return_type, Value return_value)
{
    switch (return_type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            LOG(FATAL, COROUTINES) << "Return type 'void' is not supported yet in 'launch' instruction";
            break;
        case panda_file::Type::TypeId::U1:
            return EtsBoxPrimitive<EtsBoolean>::Create(this, return_value.GetAs<EtsBoolean>());
        case panda_file::Type::TypeId::I8:
            return EtsBoxPrimitive<EtsByte>::Create(this, return_value.GetAs<EtsByte>());
        case panda_file::Type::TypeId::I16:
            return EtsBoxPrimitive<EtsShort>::Create(this, return_value.GetAs<EtsShort>());
        case panda_file::Type::TypeId::U16:
            return EtsBoxPrimitive<EtsChar>::Create(this, return_value.GetAs<EtsChar>());
        case panda_file::Type::TypeId::I32:
            return EtsBoxPrimitive<EtsInt>::Create(this, return_value.GetAs<EtsInt>());
        case panda_file::Type::TypeId::F32:
            return EtsBoxPrimitive<EtsFloat>::Create(this, return_value.GetAs<EtsFloat>());
        case panda_file::Type::TypeId::F64:
            return EtsBoxPrimitive<EtsDouble>::Create(this, return_value.GetAs<EtsDouble>());
        case panda_file::Type::TypeId::I64:
            return EtsBoxPrimitive<EtsLong>::Create(this, return_value.GetAs<EtsLong>());
        case panda_file::Type::TypeId::REFERENCE:
            return EtsObject::FromCoreType(return_value.GetAs<ObjectHeader *>());
        default:
            LOG(FATAL, COROUTINES) << "Unsupported return type: " << return_type;
            break;
    }
    return nullptr;
}
}  // namespace panda::ets
