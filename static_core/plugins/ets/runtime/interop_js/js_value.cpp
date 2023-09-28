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

#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/mem/local_object_handle.h"

namespace panda::ets::interop::js {

[[nodiscard]] JSValue *JSValue::AttachFinalizer(EtsCoroutine *coro, JSValue *js_value)
{
    ASSERT(JSValue::IsFinalizableType(js_value->GetType()));

    auto ctx = InteropCtx::Current(coro);

    LocalObjectHandle<JSValue> handle(coro, js_value);

    JSValue *mirror = AllocUndefined(coro, ctx);
    if (UNLIKELY(mirror == nullptr)) {
        FinalizeETSWeak(ctx, handle.GetPtr());
        return nullptr;
    }
    mirror->type_ = handle->type_;
    mirror->data_ = handle->data_;

    if (UNLIKELY(!ctx->PushOntoFinalizationQueue(coro, handle.GetPtr(), mirror))) {
        FinalizeETSWeak(ctx, handle.GetPtr());
        return nullptr;
    }
    return handle.GetPtr();
}

void JSValue::FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg)
{
    auto js_value = JSValue::FromEtsObject(cbarg);
    ASSERT(JSValue::IsFinalizableType(js_value->GetType()));

    auto type = js_value->GetType();
    switch (type) {
        case napi_string:
            ctx->GetStringStor()->Release(js_value->GetString());
            return;
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            NAPI_CHECK_FATAL(napi_delete_reference(ctx->GetJSEnv(), js_value->GetNapiRef()));
            return;
        default:
            InteropCtx::Fatal("Finalizer called for non-finalizable type: " + std::to_string(type));
    }
    UNREACHABLE();
}

JSValue *JSValue::Create(EtsCoroutine *coro, InteropCtx *ctx, napi_value nvalue)
{
    auto env = ctx->GetJSEnv();
    napi_valuetype js_type = GetValueType(env, nvalue);

    auto jsvalue = AllocUndefined(coro, ctx);
    if (UNLIKELY(jsvalue == nullptr)) {
        return nullptr;
    }

    switch (js_type) {
        case napi_undefined: {
            jsvalue->SetUndefined();
            return jsvalue;
        }
        case napi_null: {
            jsvalue->SetNull();
            return jsvalue;
        }
        case napi_boolean: {
            bool v;
            NAPI_ASSERT_OK(napi_get_value_bool(env, nvalue, &v));
            jsvalue->SetBoolean(v);
            return jsvalue;
        }
        case napi_number: {
            double v;
            NAPI_ASSERT_OK(napi_get_value_double(env, nvalue, &v));
            jsvalue->SetNumber(v);
            return jsvalue;
        }
        case napi_string: {
            auto cached_str = ctx->GetStringStor()->Get(interop::js::GetString(env, nvalue));
            jsvalue->SetString(cached_str);
            return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue);
        }
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            [[fallthrough]];
        case napi_external: {
            jsvalue->SetRefValue(env, nvalue, js_type);
            return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue);
        }
        default: {
            InteropCtx::Fatal("Unsupported JSValue.Type: " + std::to_string(js_type));
        }
    }
    UNREACHABLE();
}

napi_value JSValue::GetNapiValue(napi_env env)
{
    napi_value js_value {};

    auto js_type = GetType();
    switch (js_type) {
        case napi_undefined: {
            NAPI_ASSERT_OK(napi_get_undefined(env, &js_value));
            return js_value;
        }
        case napi_null: {
            NAPI_ASSERT_OK(napi_get_null(env, &js_value));
            return js_value;
        }
        case napi_boolean: {
            NAPI_ASSERT_OK(napi_get_boolean(env, GetBoolean(), &js_value));
            return js_value;
        }
        case napi_number: {
            NAPI_ASSERT_OK(napi_create_double(env, GetNumber(), &js_value));
            return js_value;
        }
        case napi_string: {
            std::string const *str = GetString().Data();
            NAPI_ASSERT_OK(napi_create_string_utf8(env, str->data(), str->size(), &js_value));
            return js_value;
        }
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            [[fallthrough]];
        case napi_external: {
            return GetRefValue(env);
        }
        default: {
            InteropCtx::Fatal("Unsupported JSValue.Type: " + std::to_string(js_type));
        }
    }
    UNREACHABLE();
}

}  // namespace panda::ets::interop::js
