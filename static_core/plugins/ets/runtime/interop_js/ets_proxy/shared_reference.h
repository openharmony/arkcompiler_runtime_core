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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_H_

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/ets_object_reference.h"

#include <node_api.h>

namespace panda::mem {
class Reference;
}  // namespace panda::mem

namespace panda::ets::interop::js {
class InteropCtx;
// Forward declarations to avoid cyclic deps.
inline mem::GlobalObjectStorage *RefstorFromInteropCtx(InteropCtx *ctx);
}  // namespace panda::ets::interop::js

namespace panda::ets::interop::js::ets_proxy {

class SharedReference {
private:
    using FlagsStart = BitField<uint8_t, 0, 0>;

public:
    static constexpr size_t MAX_MARK_BITS = MarkWord::MarkWordRepresentation::HASH_SIZE;

    // Actual state of shared object is contained in ETS
    bool InitETSObject(InteropCtx *ctx, EtsObject *ets_object, napi_value js_object, uint32_t ref_idx);

    // Actual state of shared object is contained in JS
    bool InitJSObject(InteropCtx *ctx, EtsObject *ets_object, napi_value js_object, uint32_t ref_idx);

    // State of object is shared between ETS and JS
    bool InitHybridObject(InteropCtx *ctx, EtsObject *ets_object, napi_value js_object, uint32_t ref_idx);

    using InitFn = decltype(&SharedReference::InitHybridObject);

    EtsObject *GetEtsObject(InteropCtx *ctx) const
    {
        ASSERT_MANAGED_CODE();
        ASSERT(ets_ref_ != nullptr);
        return EtsObject::FromCoreType(RefstorFromInteropCtx(ctx)->Get(ets_ref_));
    }

    napi_value GetJsObject(napi_env env) const
    {
        napi_value js_value;
        NAPI_CHECK_FATAL(napi_get_reference_value(env, js_ref_, &js_value));
        return js_value;
    }

    static void *ExtractMaybeReference(napi_env env, napi_value js_object)
    {
        void *data;
        if (UNLIKELY(napi_unwrap(env, js_object, &data) != napi_ok)) {
            return nullptr;
        }
        return data;
    }

    static bool HasReference(EtsObject *ets_object)
    {
        return ets_object->IsHashed();
    }

    static uint32_t ExtractMaybeIndex(EtsObject *ets_object)
    {
        ASSERT(HasReference(ets_object));
        return ets_object->GetHash();
    }

    using FlagsType = FlagsStart::ValueType;

    template <typename F>
    typename F::ValueType GetField() const
    {
        return F::Get(flags_);
    }

    template <typename F>
    void SetField(typename F::ValueType value)
    {
        F::Set(value, &flags_);
    }

    void SetFlags(FlagsType flags)
    {
        flags_ = flags;
    }

    using HasETSObject = FlagsStart::NextFlag;
    using HasJSObject = FlagsStart::NextFlag;

    static void FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg);

private:
    friend class SharedReferenceSanity;
    static void FinalizeJSWeak(napi_env env, void *data, void *hint);

    /* Possible values:
     *                 ets_proxy:  {instance,       proxy}
     *      extensible ets_proxy:  {proxy-base,     extender-proxy}
     *                  js_proxy:  {proxy,          instance}
     *      extensible  js_proxy:  {extender-proxy, proxy-base}
     */
    mem::Reference *ets_ref_ {};
    napi_ref js_ref_ {};

    FlagsType flags_ {};
};

}  // namespace panda::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_H_
