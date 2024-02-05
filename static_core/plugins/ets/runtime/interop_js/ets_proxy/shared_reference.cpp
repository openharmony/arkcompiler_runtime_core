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

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "runtime/mem/local_object_handle.h"

#include <node_api.h>

namespace ark::ets::interop::js::ets_proxy {

static void CBDoNothing([[maybe_unused]] napi_env env, [[maybe_unused]] void *data, [[maybe_unused]] void *hint) {}

bool SharedReference::InitETSObject(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject, uint32_t refIdx)
{
    SetFlags(HasETSObject::Encode(true) | HasJSObject::Encode(false));

    auto env = ctx->GetJSEnv();
    if (UNLIKELY(napi_ok != NapiWrap(env, jsObject, this, FinalizeJSWeak, nullptr, &jsRef_))) {
        return false;
    }

    etsObject->SetInteropHash(refIdx);
    etsRef_ = ctx->Refstor()->Add(etsObject->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    if (UNLIKELY(etsRef_ == nullptr)) {
        INTEROP_LOG(ERROR) << "REFERENCE STORAGE OVERFLOW";
        ctx->ThrowJSError(env, "ets refstor overflow");
        return false;
    }
    return true;
}

bool SharedReference::InitJSObject(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject, uint32_t refIdx)
{
    SetFlags(HasETSObject::Encode(false) | HasJSObject::Encode(true));

    auto coro = EtsCoroutine::GetCurrent();
    auto env = ctx->GetJSEnv();
    if (UNLIKELY(napi_ok != NapiWrap(env, jsObject, this, CBDoNothing, nullptr, &jsRef_))) {
        return false;
    }
    NAPI_CHECK_FATAL(napi_reference_ref(env, jsRef_, nullptr));

    LocalObjectHandle<EtsObject> handle(coro, etsObject);  // object may have no strong refs, so create one
    handle->SetInteropHash(refIdx);
    // NOTE(vpukhov): reuse weakref from finalizationRegistry
    etsRef_ = ctx->Refstor()->Add(etsObject->GetCoreType(), mem::Reference::ObjectType::WEAK);

    auto boxLong = EtsBoxPrimitive<EtsLong>::Create(EtsCoroutine::GetCurrent(), ToUintPtr(this));
    if (UNLIKELY(boxLong == nullptr ||
                 !ctx->PushOntoFinalizationRegistry(EtsCoroutine::GetCurrent(), handle.GetPtr(), boxLong))) {
        NAPI_CHECK_FATAL(napi_delete_reference(env, jsRef_));
        return false;
    }
    return true;
}

// NOTE(vpukhov): Circular interop references
//                Present solution is dummy and consists of two strong refs
bool SharedReference::InitHybridObject(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject, uint32_t refIdx)
{
    SetFlags(HasETSObject::Encode(true) | HasJSObject::Encode(true));

    auto env = ctx->GetJSEnv();
    if (UNLIKELY(napi_ok != NapiWrap(env, jsObject, this, CBDoNothing, nullptr, &jsRef_))) {
        return false;
    }
    NAPI_CHECK_FATAL(napi_reference_ref(env, jsRef_, nullptr));

    etsObject->SetInteropHash(refIdx);
    etsRef_ = ctx->Refstor()->Add(etsObject->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    if (UNLIKELY(etsRef_ == nullptr)) {
        INTEROP_LOG(ERROR) << "REFERENCE STORAGE OVERFLOW";
        ctx->ThrowJSError(env, "ets refstor overflow");
        NAPI_CHECK_FATAL(napi_delete_reference(env, jsRef_));
        return false;
    }
    return true;
}

/*static*/
void SharedReference::FinalizeJSWeak([[maybe_unused]] napi_env env, void *data, [[maybe_unused]] void *hint)
{
    if (UNLIKELY(Runtime::GetCurrent() == nullptr)) {
        // Runtime was destroyed, no need to cleanup
        return;
    }
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    InteropCtx *ctx = InteropCtx::Current(coro);
    ScopedManagedCodeThread scope(coro);

    auto ref = reinterpret_cast<SharedReference *>(data);
    ASSERT(ref->etsRef_ != nullptr);

    ref->GetEtsObject(ctx)->DropInteropHash();
    ctx->Refstor()->Remove(ref->etsRef_);
    ctx->GetSharedRefStorage()->RemoveReference(ref);
}

/*static*/
void SharedReference::FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg)
{
    ASSERT(cbarg->GetClass()->GetRuntimeClass() == ctx->GetBoxLongClass());
    auto boxLong = FromEtsObject<EtsBoxPrimitive<EtsLong>>(cbarg);

    auto sharedRef = ToNativePtr<SharedReference>(static_cast<uintptr_t>(boxLong->GetValue()));

    NAPI_CHECK_FATAL(napi_delete_reference(ctx->GetJSEnv(), sharedRef->jsRef_));
    ctx->GetSharedRefStorage()->RemoveReference(sharedRef);
}

}  // namespace ark::ets::interop::js::ets_proxy
