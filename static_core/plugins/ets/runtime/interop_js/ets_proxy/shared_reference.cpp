/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

bool SharedReference::InitRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject, uint32_t refIdx,
                              [[maybe_unused]] NapiXRefDirection refDirection)
{
    auto env = ctx->GetJSEnv();
    // Always save link to first reference
    SharedReference *ref = SharedReferenceStorage::GetCurrent()->GetItemByIndex(refIdx);
    // NOTE(ipetrov, #20833): Use special OHOS interfaces for cross references
    if (UNLIKELY(napi_ok != NapiWrap(env, jsObject, ref, CBDoNothing, &jsRef_, refDirection))) {
        return false;
    }
    // NOTE(ipetrov, 20833): Now we always keep the reference, special GC will remove such references
    // For future we can to provide a solution with correct finalization from finalizeCb in NapiWrap method
    NAPI_CHECK_FATAL(napi_reference_ref(env, jsRef_, nullptr));

    etsObject->SetInteropIndex(refIdx);
    SetETSObject(etsObject);
    ctx_ = ctx;
    return true;
}

bool SharedReference::InitETSObject(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject, uint32_t refIdx)
{
    flags_.SetBit<SharedReferenceFlags::Bit::ETS>();
    return InitRef(ctx, etsObject, jsObject, refIdx, NapiXRefDirection::NAPI_DIRECTION_DYNAMIC_TO_STATIC);
}

bool SharedReference::InitJSObject(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject, uint32_t refIdx)
{
    flags_.SetBit<SharedReferenceFlags::Bit::JS>();
    return InitRef(ctx, etsObject, jsObject, refIdx, NapiXRefDirection::NAPI_DIRECTION_STATIC_TO_DYNAMIC);
}

bool SharedReference::InitHybridObject(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject, uint32_t refIdx)
{
    flags_.SetBit<SharedReferenceFlags::Bit::ETS>();
    flags_.SetBit<SharedReferenceFlags::Bit::JS>();
    return InitRef(ctx, etsObject, jsObject, refIdx, NapiXRefDirection::NAPI_DIRECTION_HYBRID);
}

bool SharedReference::MarkIfNotMarked()
{
    return flags_.SetBit<SharedReferenceFlags::Bit::MARK>();
}

SharedReference::Iterator &SharedReference::Iterator::operator++()
{
    uint32_t index = ref_->flags_.GetNextIndex();
    if (index == 0U) {
        ref_ = nullptr;
    } else {
        ref_ = SharedReferenceStorage::GetCurrent()->GetItemByIndex(index);
    }
    return *this;
}

SharedReference::Iterator SharedReference::Iterator::operator++(int)  // NOLINT(cert-dcl21-cpp)
{
    auto result = *this;
    auto index = ref_->flags_.GetNextIndex();
    if (index == 0U) {
        ref_ = nullptr;
    } else {
        ref_ = SharedReferenceStorage::GetCurrent()->GetItemByIndex(index);
    }
    return result;
}

}  // namespace ark::ets::interop::js::ets_proxy
