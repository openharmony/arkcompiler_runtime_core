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

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "runtime/mem/local_object_handle.h"

#include <node_api.h>

namespace panda::ets::interop::js::ets_proxy {

static void CBDoNothing([[maybe_unused]] napi_env env, [[maybe_unused]] void *data, [[maybe_unused]] void *hint) {}

bool SharedReference::InitETSObject(InteropCtx *ctx, EtsObject *ets_object, napi_value js_object, uint32_t ref_idx)
{
    SetFlags(HasETSObject::Encode(true) | HasJSObject::Encode(false));

    auto env = ctx->GetJSEnv();
    if (UNLIKELY(napi_ok != NapiWrap(env, js_object, this, FinalizeJSWeak, nullptr, &js_ref_))) {
        return false;
    }

    ets_object->SetHash(ref_idx);
    ets_ref_ = ctx->Refstor()->Add(ets_object->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    if (UNLIKELY(ets_ref_ == nullptr)) {
        INTEROP_LOG(ERROR) << "REFERENCE STORAGE OVERFLOW";
        ctx->ThrowJSError(env, "ets refstor overflow");
        return false;
    }
    return true;
}

bool SharedReference::InitJSObject(InteropCtx *ctx, EtsObject *ets_object, napi_value js_object, uint32_t ref_idx)
{
    SetFlags(HasETSObject::Encode(false) | HasJSObject::Encode(true));

    auto coro = EtsCoroutine::GetCurrent();
    auto env = ctx->GetJSEnv();
    if (UNLIKELY(napi_ok != NapiWrap(env, js_object, this, CBDoNothing, nullptr, &js_ref_))) {
        return false;
    }
    NAPI_CHECK_FATAL(napi_reference_ref(env, js_ref_, nullptr));

    LocalObjectHandle<EtsObject> handle(coro, ets_object);  // object may have no strong refs, so create one
    handle->SetHash(ref_idx);
    // TODO(vpukhov): reuse weakref from finalizationqueue
    ets_ref_ = ctx->Refstor()->Add(ets_object->GetCoreType(), mem::Reference::ObjectType::WEAK);

    auto box_long = EtsBoxPrimitive<EtsLong>::Create(EtsCoroutine::GetCurrent(), ToUintPtr(this));
    if (UNLIKELY(box_long == nullptr ||
                 !ctx->PushOntoFinalizationQueue(EtsCoroutine::GetCurrent(), handle.GetPtr(), box_long))) {
        NAPI_CHECK_FATAL(napi_delete_reference(env, js_ref_));
        return false;
    }
    return true;
}

// TODO(vpukhov): Circular interop references
//                Present solution is dummy and consists of two strong refs
bool SharedReference::InitHybridObject(InteropCtx *ctx, EtsObject *ets_object, napi_value js_object, uint32_t ref_idx)
{
    SetFlags(HasETSObject::Encode(true) | HasJSObject::Encode(true));

    auto env = ctx->GetJSEnv();
    if (UNLIKELY(napi_ok != NapiWrap(env, js_object, this, CBDoNothing, nullptr, &js_ref_))) {
        return false;
    }
    NAPI_CHECK_FATAL(napi_reference_ref(env, js_ref_, nullptr));

    ets_object->SetHash(ref_idx);
    ets_ref_ = ctx->Refstor()->Add(ets_object->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    if (UNLIKELY(ets_ref_ == nullptr)) {
        INTEROP_LOG(ERROR) << "REFERENCE STORAGE OVERFLOW";
        ctx->ThrowJSError(env, "ets refstor overflow");
        NAPI_CHECK_FATAL(napi_delete_reference(env, js_ref_));
        return false;
    }
    return true;
}

/*static*/
void SharedReference::FinalizeJSWeak([[maybe_unused]] napi_env env, void *data, [[maybe_unused]] void *hint)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    InteropCtx *ctx = InteropCtx::Current(coro);

    auto ref = reinterpret_cast<SharedReference *>(data);
    ASSERT(ref->ets_ref_ != nullptr);

    ctx->Refstor()->Remove(ref->ets_ref_);
    ctx->GetSharedRefStorage()->RemoveReference(ref);
}

/*static*/
void SharedReference::FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg)
{
    ASSERT(cbarg->GetClass()->GetRuntimeClass() == ctx->GetBoxLongClass());
    auto box_long = FromEtsObject<EtsBoxPrimitive<EtsLong>>(cbarg);

    auto shared_ref = ToNativePtr<SharedReference>(static_cast<uintptr_t>(box_long->GetValue()));

    NAPI_CHECK_FATAL(napi_delete_reference(ctx->GetJSEnv(), shared_ref->js_ref_));
    ctx->GetSharedRefStorage()->RemoveReference(shared_ref);
}

}  // namespace panda::ets::interop::js::ets_proxy
