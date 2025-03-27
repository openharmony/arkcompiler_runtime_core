/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

namespace ark::ets::interop::js {

[[nodiscard]] JSValue *JSValue::AttachFinalizer(EtsCoroutine *coro, JSValue *jsValue)
{
    ASSERT(JSValue::IsFinalizableType(jsValue->GetType()));

    auto ctx = InteropCtx::Current(coro);
    if (ctx == nullptr) {
        ThrowNoInteropContextException();
        return nullptr;
    }

    LocalObjectHandle<JSValue> handle(coro, jsValue);

    JSValue *mirror = AllocUndefined(coro, ctx);
    if (UNLIKELY(mirror == nullptr)) {
        FinalizeETSWeak(ctx, handle.GetPtr());
        return nullptr;
    }
    mirror->type_ = handle->type_;
    mirror->data_ = handle->data_;

    if (UNLIKELY(!ctx->PushOntoFinalizationRegistry(coro, handle.GetPtr(), mirror))) {
        FinalizeETSWeak(ctx, handle.GetPtr());
        return nullptr;
    }
    return handle.GetPtr();
}

void JSValue::FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg)
{
    auto jsValue = JSValue::FromEtsObject(cbarg);
    ASSERT(JSValue::IsFinalizableType(jsValue->GetType()));

    auto type = jsValue->GetType();
    switch (type) {
        case napi_string:
            ctx->GetStringStor()->Release(jsValue->GetString());
            return;
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            NAPI_CHECK_FATAL(napi_delete_reference(ctx->GetJSEnv(), jsValue->GetNapiRef(ctx->GetJSEnv())));
            return;
        case napi_bigint:
            delete jsValue->GetBigInt();
            return;
        default:
            InteropCtx::Fatal("Finalizer called for non-finalizable type: " + std::to_string(type));
    }
    UNREACHABLE();
}

JSValue *JSValue::CreateByType(InteropCtx *ctx, napi_env env, napi_value nvalue, napi_valuetype jsType,
                               JSValue *jsvalue)
{
    switch (jsType) {
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
            auto cachedStr = ctx->GetStringStor()->Get(interop::js::GetString(env, nvalue));
            jsvalue->SetString(cachedStr);
            return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue);
        }
        case napi_bigint: {
            jsvalue->SetBigInt(interop::js::GetBigInt(env, nvalue));
            return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue);
        }
        case napi_symbol:
            [[fallthrough]];
        case napi_object:
            [[fallthrough]];
        case napi_function:
            [[fallthrough]];
        case napi_external: {
            jsvalue->SetRefValue(ctx, nvalue, jsType);
            return jsvalue;
        }
        default: {
            InteropCtx::Fatal("Unsupported JSValue.Type: " + std::to_string(jsType));
        }
    }
    UNREACHABLE();
}
JSValue *JSValue::Create(EtsCoroutine *coro, InteropCtx *ctx, napi_value nvalue)
{
    auto env = ctx->GetJSEnv();
    napi_valuetype jsType = GetValueType(env, nvalue);

    auto jsvalue = AllocUndefined(coro, ctx);
    if (UNLIKELY(jsvalue == nullptr)) {
        return nullptr;
    }

    return CreateByType(ctx, env, nvalue, jsType, jsvalue);
}

napi_value JSValue::GetNapiValue(napi_env env)
{
    napi_value jsValue {};

    auto jsType = GetType();
    switch (jsType) {
        case napi_undefined: {
            NAPI_ASSERT_OK(napi_get_undefined(env, &jsValue));
            return jsValue;
        }
        case napi_null: {
            NAPI_ASSERT_OK(napi_get_null(env, &jsValue));
            return jsValue;
        }
        case napi_boolean: {
            NAPI_ASSERT_OK(napi_get_boolean(env, GetBoolean(), &jsValue));
            return jsValue;
        }
        case napi_number: {
            NAPI_ASSERT_OK(napi_create_double(env, GetNumber(), &jsValue));
            return jsValue;
        }
        case napi_string: {
            std::string const *str = GetString().Data();
            NAPI_ASSERT_OK(napi_create_string_utf8(env, str->data(), str->size(), &jsValue));
            return jsValue;
        }
        case napi_bigint: {
            auto [words, signBit] = *GetBigInt();
            NAPI_ASSERT_OK(napi_create_bigint_words(env, signBit, words.size(), words.data(), &jsValue));
            return jsValue;
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
            InteropCtx::Fatal("Unsupported JSValue.Type: " + std::to_string(jsType));
        }
    }
    UNREACHABLE();
}

}  // namespace ark::ets::interop::js
