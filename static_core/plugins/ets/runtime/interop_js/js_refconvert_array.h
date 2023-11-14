/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_ARRAY_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_ARRAY_H_

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/js_refconvert.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "runtime/mem/local_object_handle.h"

namespace panda::ets::interop::js {

// JSRefConvert adapter for builtin[] types
template <typename Conv>
class JSRefConvertBuiltinArray : public JSRefConvert {
public:
    explicit JSRefConvertBuiltinArray(Class *klass) : JSRefConvert(this), klass_(klass) {}

private:
    using ElemCpptype = typename Conv::cpptype;

    static ElemCpptype GetElem(coretypes::Array *arr, size_t idx)
    {
        if constexpr (Conv::IS_REFTYPE) {
            auto elem = EtsObject::FromCoreType(arr->Get<ObjectHeader *>(idx));
            return FromEtsObject<std::remove_pointer_t<ElemCpptype>>(elem);
        } else {
            return arr->Get<ElemCpptype>(idx);
        }
    }

    static void SetElem(coretypes::Array *arr, size_t idx, ElemCpptype value)
    {
        if constexpr (Conv::IS_REFTYPE) {
            arr->Set(idx, AsEtsObject(value)->GetCoreType());
        } else {
            arr->Set(idx, value);
        }
    }

public:
    napi_value WrapImpl(InteropCtx *ctx, EtsObject *obj)
    {
        auto env = ctx->GetJSEnv();

        auto ets_arr = static_cast<coretypes::Array *>(obj->GetCoreType());
        auto len = ets_arr->GetLength();

        NapiEscapableScope js_handle_scope(env);
        napi_value js_arr;
        NAPI_CHECK_FATAL(napi_create_array_with_length(env, len, &js_arr));

        for (size_t idx = 0; idx < len; ++idx) {
            ElemCpptype ets_elem = GetElem(ets_arr, idx);
            auto js_elem = Conv::WrapWithNullCheck(env, ets_elem);
            if (UNLIKELY(js_elem == nullptr)) {
                return nullptr;
            }
            napi_status rc = napi_set_element(env, js_arr, idx, js_elem);
            if (UNLIKELY(NapiThrownGeneric(rc))) {
                return nullptr;
            }
        }
        js_handle_scope.Escape(js_arr);
        return js_arr;
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value js_arr)
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = ctx->GetJSEnv();
        {
            bool is_array;
            NAPI_CHECK_FATAL(napi_is_array(env, js_arr, &is_array));
            if (UNLIKELY(!is_array)) {
                JSConvertTypeCheckFailed("array");
                return nullptr;
            }
        }

        uint32_t len;
        napi_status rc = napi_get_array_length(env, js_arr, &len);
        if (UNLIKELY(NapiThrownGeneric(rc))) {
            return nullptr;
        }

        // NOTE(vpukhov): elide handles for primitive arrays
        LocalObjectHandle<coretypes::Array> ets_arr(coro, coretypes::Array::Create(klass_, len));
        NapiScope js_handle_scope(env);

        for (size_t idx = 0; idx < len; ++idx) {
            napi_value js_elem;
            rc = napi_get_element(env, js_arr, idx, &js_elem);
            if (UNLIKELY(NapiThrownGeneric(rc))) {
                return nullptr;
            }
            auto res = Conv::UnwrapWithNullCheck(ctx, env, js_elem);
            if (UNLIKELY(!res)) {
                return nullptr;
            }
            SetElem(ets_arr.GetPtr(), idx, res.value());
        }

        return EtsObject::FromCoreType(ets_arr.GetPtr());
    }

private:
    Class *klass_ {};
};

template <ClassRoot CLASS_ROOT, typename Conv>
static inline void RegisterBuiltinArrayConvertor(JSRefConvertCache *cache, EtsClassLinkerExtension *ext)
{
    auto aklass = ext->GetClassRoot(CLASS_ROOT);
    cache->Insert(aklass, std::unique_ptr<JSRefConvert>(new JSRefConvertBuiltinArray<Conv>(aklass)));
}

// JSRefConvert adapter for reference[] types
class JSRefConvertReftypeArray : public JSRefConvert {
public:
    explicit JSRefConvertReftypeArray(Class *klass) : JSRefConvert(this), klass_(klass) {}

    napi_value WrapImpl(InteropCtx *ctx, EtsObject *obj)
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = ctx->GetJSEnv();

        LocalObjectHandle<coretypes::Array> ets_arr(coro, obj->GetCoreType());
        auto len = ets_arr->GetLength();

        NapiEscapableScope js_handle_scope(env);
        napi_value js_arr;
        NAPI_CHECK_FATAL(napi_create_array_with_length(env, len, &js_arr));

        for (size_t idx = 0; idx < len; ++idx) {
            EtsObject *ets_elem = EtsObject::FromCoreType(ets_arr->Get<ObjectHeader *>(idx));
            napi_value js_elem;
            if (LIKELY(ets_elem != nullptr)) {
                JSRefConvert *elem_conv = GetElemConvertor(ctx, ets_elem->GetClass());
                if (UNLIKELY(elem_conv == nullptr)) {
                    return nullptr;
                }
                js_elem = elem_conv->Wrap(ctx, ets_elem);
                if (UNLIKELY(js_elem == nullptr)) {
                    return nullptr;
                }
            } else {
                js_elem = GetNull(env);
            }
            napi_status rc = napi_set_element(env, js_arr, idx, js_elem);
            if (UNLIKELY(NapiThrownGeneric(rc))) {
                return nullptr;
            }
        }
        js_handle_scope.Escape(js_arr);
        return js_arr;
    }

    EtsObject *UnwrapImpl(InteropCtx *ctx, napi_value js_arr)
    {
        auto coro = EtsCoroutine::GetCurrent();
        auto env = ctx->GetJSEnv();
        {
            bool is_array;
            NAPI_CHECK_FATAL(napi_is_array(env, js_arr, &is_array));
            if (UNLIKELY(!is_array)) {
                JSConvertTypeCheckFailed("array");
                return nullptr;
            }
        }

        uint32_t len;
        napi_status rc = napi_get_array_length(env, js_arr, &len);
        if (UNLIKELY(NapiThrownGeneric(rc))) {
            return nullptr;
        }

        LocalObjectHandle<coretypes::Array> ets_arr(coro, coretypes::Array::Create(klass_, len));
        NapiScope js_handle_scope(env);

        for (size_t idx = 0; idx < len; ++idx) {
            napi_value js_elem;
            rc = napi_get_element(env, js_arr, idx, &js_elem);
            if (UNLIKELY(NapiThrownGeneric(rc))) {
                return nullptr;
            }
            if (LIKELY(!IsNullOrUndefined(env, js_elem))) {
                if (UNLIKELY(base_elem_conv_ == nullptr)) {
                    base_elem_conv_ = JSRefConvertResolve(ctx, klass_->GetComponentType());
                    if (UNLIKELY(base_elem_conv_ == nullptr)) {
                        return nullptr;
                    }
                }
                EtsObject *ets_elem = base_elem_conv_->Unwrap(ctx, js_elem);
                if (UNLIKELY(ets_elem == nullptr)) {
                    return nullptr;
                }
                ets_arr->Set(idx, ets_elem->GetCoreType());
            }
        }

        return EtsObject::FromCoreType(ets_arr.GetPtr());
    }

private:
    static constexpr auto ELEM_SIZE = ClassHelper::OBJECT_POINTER_SIZE;

    JSRefConvert *GetElemConvertor(InteropCtx *ctx, EtsClass *elem_ets_klass)
    {
        Class *elem_klass = elem_ets_klass->GetRuntimeClass();
        if (elem_klass != klass_->GetComponentType()) {
            return JSRefConvertResolve(ctx, elem_klass);
        }
        if (LIKELY(base_elem_conv_ != nullptr)) {
            return base_elem_conv_;
        }
        return base_elem_conv_ = JSRefConvertResolve(ctx, klass_->GetComponentType());
    }

    Class *klass_ {};
    JSRefConvert *base_elem_conv_ {};
};

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JS_REFCONVERT_ARRAY_H_
