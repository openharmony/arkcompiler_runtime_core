/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_ETS_RUNTIME_ETS_CALLBACK_H
#define PANDA_RUNTIME_ETS_RUNTIME_ETS_CALLBACK_H

#include "macros.h"
#include "mem/refstorage/reference.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/include/callback_queue.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark::ets {
class EtsCallback final : public Callback {
public:
    static PandaUniquePtr<Callback> Create(EtsCoroutine *coro, const EtsObject *callback)
    {
        auto *refStorage = coro->GetPandaVM()->GetGlobalObjectStorage();
        auto *callbackRef = refStorage->Add(callback->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        ASSERT(callbackRef != nullptr);
        auto *etsCallback = Runtime::GetCurrent()->GetInternalAllocator()->New<EtsCallback>(callbackRef);
        return PandaUniquePtr<Callback>(etsCallback);
    }

    void Run() override
    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *refStorage = coro->GetPandaVM()->GetGlobalObjectStorage();
        auto *callback = EtsObject::FromCoreType(refStorage->Get(callbackRef_));

        [[maybe_unused]] EtsHandleScope scope(coro);
        EtsHandle<EtsObject> exception(coro, EtsObject::FromCoreType(coro->GetException()));

        LambdaUtils::InvokeVoid(coro, callback);

        if (exception.GetPtr() != nullptr) {
            coro->SetException(exception.GetPtr()->GetCoreType());
        }
    }

    ~EtsCallback() override
    {
        auto *refStorage = EtsCoroutine::GetCurrent()->GetPandaVM()->GetGlobalObjectStorage();
        refStorage->Remove(callbackRef_);
    }

    NO_COPY_SEMANTIC(EtsCallback);
    DEFAULT_MOVE_SEMANTIC(EtsCallback);

private:
    explicit EtsCallback(mem::Reference *callbackRef) : callbackRef_(callbackRef) {}

    mem::Reference *callbackRef_;

    friend class ark::mem::Allocator;
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_RUNTIME_ETS_CALLBACK_H
