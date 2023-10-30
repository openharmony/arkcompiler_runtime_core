/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"

#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace panda::ets::interop::js::ets_proxy {

class SharedReferenceSanity {
public:
    static inline void Kill(SharedReference *ref)
    {
        ASSERT(ref->js_ref_ != nullptr);
        ref->js_ref_ = nullptr;
    }

    static inline bool CheckAlive(SharedReference *ref)
    {
        return ref->js_ref_ != nullptr;
    }
};

/*static*/
std::unique_ptr<SharedReferenceStorage> SharedReferenceStorage::Create()
{
    size_t real_size = SharedReferenceStorage::MAX_POOL_SIZE;

    void *data = os::mem::MapRWAnonymousRaw(real_size);
    if (data == nullptr) {
        INTEROP_LOG(FATAL) << "Cannot allocate MemPool";
        return nullptr;
    }
    return std::unique_ptr<SharedReferenceStorage>(new SharedReferenceStorage(data, real_size));
}

SharedReference *SharedReferenceStorage::GetReference(EtsObject *ets_object)
{
    ASSERT(HasReference(ets_object));
    return GetItemByIndex(SharedReference::ExtractMaybeIndex(ets_object));
}

SharedReference *SharedReferenceStorage::GetReference(napi_env env, napi_value js_object)
{
    void *data = SharedReference::ExtractMaybeReference(env, js_object);
    if (UNLIKELY(data == nullptr)) {
        return nullptr;
    }
    return GetReference(data);
}

SharedReference *SharedReferenceStorage::GetReference(void *data)
{
    auto *shared_ref = reinterpret_cast<SharedReference *>(data);
    if (UNLIKELY(!IsValidItem(shared_ref))) {
        // We don't own that object
        return nullptr;
    }
    ASSERT(SharedReferenceSanity::CheckAlive(shared_ref));
    return shared_ref;
}

template <SharedReference::InitFn REF_INIT>
inline SharedReference *SharedReferenceStorage::CreateReference(InteropCtx *ctx, EtsObject *ets_object,
                                                                napi_value js_object)
{
    ASSERT(!SharedReference::HasReference(ets_object));
    SharedReference *shared_ref = AllocItem();
    if (UNLIKELY(shared_ref == nullptr)) {
        ctx->ThrowJSError(ctx->GetJSEnv(), "no free space for SharedReference");
        return nullptr;
    }
    if (UNLIKELY(!(shared_ref->*REF_INIT)(ctx, ets_object, js_object, GetIndexByItem(shared_ref)))) {
        auto coro = EtsCoroutine::GetCurrent();
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }
    return shared_ref;
}

SharedReference *SharedReferenceStorage::CreateETSObjectRef(InteropCtx *ctx, EtsObject *ets_object,
                                                            napi_value js_object)
{
    return CreateReference<&SharedReference::InitETSObject>(ctx, ets_object, js_object);
}

SharedReference *SharedReferenceStorage::CreateJSObjectRef(InteropCtx *ctx, EtsObject *ets_object, napi_value js_object)
{
    return CreateReference<&SharedReference::InitJSObject>(ctx, ets_object, js_object);
}

SharedReference *SharedReferenceStorage::CreateHybridObjectRef(InteropCtx *ctx, EtsObject *ets_object,
                                                               napi_value js_object)
{
    return CreateReference<&SharedReference::InitHybridObject>(ctx, ets_object, js_object);
}

void SharedReferenceStorage::RemoveReference(SharedReference *shared_ref)
{
    FreeItem(shared_ref);
    SharedReferenceSanity::Kill(shared_ref);
}

bool SharedReferenceStorage::CheckAlive(void *data)
{
    auto *shared_ref = reinterpret_cast<SharedReference *>(data);
    return IsValidItem(shared_ref) && SharedReferenceSanity::CheckAlive(shared_ref);
}

}  // namespace panda::ets::interop::js::ets_proxy
