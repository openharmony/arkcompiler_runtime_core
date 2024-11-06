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

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"

#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js::ets_proxy {

class SharedReferenceSanity {
public:
    static inline void Kill(SharedReference *ref)
    {
        ASSERT(ref->jsRef_ != nullptr);
        ref->jsRef_ = nullptr;
    }

    static inline bool CheckAlive(SharedReference *ref)
    {
        return ref->jsRef_ != nullptr;
    }
};

/*static*/
std::unique_ptr<SharedReferenceStorage> SharedReferenceStorage::Create()
{
    size_t realSize = SharedReferenceStorage::MAX_POOL_SIZE;

    void *data = os::mem::MapRWAnonymousRaw(realSize);
    if (data == nullptr) {
        INTEROP_LOG(FATAL) << "Cannot allocate MemPool";
        return nullptr;
    }
    // CC-OFFNXT(G.RES.09) private constructor
    return std::unique_ptr<SharedReferenceStorage>(new SharedReferenceStorage(data, realSize));
}

SharedReference *SharedReferenceStorage::GetReference(EtsObject *etsObject)
{
    ASSERT(HasReference(etsObject));
    return GetItemByIndex(SharedReference::ExtractMaybeIndex(etsObject));
}

SharedReference *SharedReferenceStorage::GetReference(napi_env env, napi_value jsObject)
{
    void *data = SharedReference::ExtractMaybeReference(env, jsObject);
    if (UNLIKELY(data == nullptr)) {
        return nullptr;
    }
    return GetReference(data);
}

SharedReference *SharedReferenceStorage::GetReference(void *data)
{
    auto *sharedRef = reinterpret_cast<SharedReference *>(data);
    if (UNLIKELY(!IsValidItem(sharedRef))) {
        // We don't own that object
        return nullptr;
    }
    ASSERT(SharedReferenceSanity::CheckAlive(sharedRef));
    return sharedRef;
}

template <SharedReference::InitFn REF_INIT>
inline SharedReference *SharedReferenceStorage::CreateReference(InteropCtx *ctx, EtsObject *etsObject,
                                                                napi_value jsObject)
{
    ASSERT(!SharedReference::HasReference(etsObject));
    SharedReference *sharedRef = AllocItem();
    if (UNLIKELY(sharedRef == nullptr)) {
        ctx->ThrowJSError(ctx->GetJSEnv(), "no free space for SharedReference");
        return nullptr;
    }
    if (UNLIKELY(!(sharedRef->*REF_INIT)(ctx, etsObject, jsObject, GetIndexByItem(sharedRef)))) {
        auto coro = EtsCoroutine::GetCurrent();
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }
    return sharedRef;
}

SharedReference *SharedReferenceStorage::CreateETSObjectRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject)
{
    return CreateReference<&SharedReference::InitETSObject>(ctx, etsObject, jsObject);
}

SharedReference *SharedReferenceStorage::CreateJSObjectRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject)
{
    return CreateReference<&SharedReference::InitJSObject>(ctx, etsObject, jsObject);
}

SharedReference *SharedReferenceStorage::CreateHybridObjectRef(InteropCtx *ctx, EtsObject *etsObject,
                                                               napi_value jsObject)
{
    return CreateReference<&SharedReference::InitHybridObject>(ctx, etsObject, jsObject);
}

void SharedReferenceStorage::RemoveReference(SharedReference *sharedRef)
{
    FreeItem(sharedRef);
    SharedReferenceSanity::Kill(sharedRef);
}

bool SharedReferenceStorage::CheckAlive(void *data)
{
    auto *sharedRef = reinterpret_cast<SharedReference *>(data);
    return IsValidItem(sharedRef) && SharedReferenceSanity::CheckAlive(sharedRef);
}

}  // namespace ark::ets::interop::js::ets_proxy
