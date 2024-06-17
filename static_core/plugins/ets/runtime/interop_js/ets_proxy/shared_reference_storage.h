/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_H_

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/mem/items_pool.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/mark_word.h"

namespace ark::ets::interop::js {
class InteropCtx;
}  // namespace ark::ets::interop::js

namespace ark::ets::interop::js::ets_proxy {

namespace testing {
class SharedReferenceStorage1GTest;
}  // namespace testing

using SharedReferencePool = ItemsPool<SharedReference, SharedReference::MAX_MARK_BITS>;

class SharedReferenceStorage : private SharedReferencePool {
public:
    PANDA_PUBLIC_API static std::unique_ptr<SharedReferenceStorage> Create();
    ~SharedReferenceStorage() = default;

    static bool HasReference(EtsObject *etsObject)
    {
        return SharedReference::HasReference(etsObject);
    }

    PANDA_PUBLIC_API SharedReference *CreateETSObjectRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject);
    SharedReference *CreateJSObjectRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject);
    SharedReference *CreateHybridObjectRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject);

    PANDA_PUBLIC_API SharedReference *GetReference(napi_env env, napi_value jsObject);
    PANDA_PUBLIC_API SharedReference *GetReference(EtsObject *etsObject);

    SharedReference *GetNextAlloc() const
    {
        return SharedReferencePool::GetNextAlloc();
    }

    static constexpr size_t MAX_POOL_SIZE = (sizeof(void *) > 4) ? 1_GB : 64_MB;

private:
    SharedReferenceStorage(void *data, size_t size) : SharedReferencePool(data, size) {}
    NO_COPY_SEMANTIC(SharedReferenceStorage);
    NO_MOVE_SEMANTIC(SharedReferenceStorage);

    template <SharedReference::InitFn REF_INIT>
    PANDA_PUBLIC_API inline SharedReference *CreateReference(InteropCtx *ctx, EtsObject *etsObject,
                                                             napi_value jsObject);

    PANDA_PUBLIC_API SharedReference *GetReference(void *data);
    PANDA_PUBLIC_API void RemoveReference(SharedReference *sharedRef);

    PANDA_PUBLIC_API bool CheckAlive(void *data);
    friend class SharedReference;
    friend class testing::SharedReferenceStorage1GTest;
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_H_
