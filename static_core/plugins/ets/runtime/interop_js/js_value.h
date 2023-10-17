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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_H_

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/include/coretypes/class.h"
#include <node_api.h>

namespace panda::ets::interop::js {

namespace testing {
class JSValueOffsets;
}  // namespace testing

struct JSValueMemberOffsets;

class JSValue : private EtsObject {
public:
    static JSValue *FromEtsObject(EtsObject *ets_object)
    {
        ASSERT(ets_object->GetClass() == EtsClass::FromRuntimeClass(InteropCtx::Current()->GetJSValueClass()));
        return static_cast<JSValue *>(ets_object);
    }

    static JSValue *FromCoreType(ObjectHeader *object)
    {
        return FromEtsObject(EtsObject::FromCoreType(object));
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    ObjectHeader *GetCoreType() const
    {
        return EtsObject::GetCoreType();
    }

    napi_valuetype GetType() const
    {
        return bit_cast<napi_valuetype>(type_);
    }

    static JSValue *Create(EtsCoroutine *coro, InteropCtx *ctx, napi_value nvalue);

    static JSValue *CreateUndefined(EtsCoroutine *coro, InteropCtx *ctx)
    {
        return AllocUndefined(coro, ctx);
    }

    static JSValue *CreateNull(EtsCoroutine *coro, InteropCtx *ctx)
    {
        auto jsvalue = AllocUndefined(coro, ctx);
        if (UNLIKELY(jsvalue == nullptr)) {
            return nullptr;
        }
        jsvalue->SetNull();
        return jsvalue;
    }

    static JSValue *CreateBoolean(EtsCoroutine *coro, InteropCtx *ctx, bool value)
    {
        auto jsvalue = AllocUndefined(coro, ctx);
        if (UNLIKELY(jsvalue == nullptr)) {
            return nullptr;
        }
        jsvalue->SetBoolean(value);
        return jsvalue;
    }

    static JSValue *CreateNumber(EtsCoroutine *coro, InteropCtx *ctx, double value)
    {
        auto jsvalue = AllocUndefined(coro, ctx);
        if (UNLIKELY(jsvalue == nullptr)) {
            return nullptr;
        }
        jsvalue->SetNumber(value);
        return jsvalue;
    }

    static JSValue *CreateString(EtsCoroutine *coro, InteropCtx *ctx, std::string &&value)
    {
        auto jsvalue = AllocUndefined(coro, ctx);
        if (UNLIKELY(jsvalue == nullptr)) {
            return nullptr;
        }
        jsvalue->SetString(ctx->GetStringStor()->Get(std::move(value)));
        return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue);
    }

    static JSValue *CreateRefValue(EtsCoroutine *coro, InteropCtx *ctx, napi_value value, napi_valuetype type)
    {
        auto jsvalue = AllocUndefined(coro, ctx);
        if (UNLIKELY(jsvalue == nullptr)) {
            return nullptr;
        }
        jsvalue->SetRefValue(ctx->GetJSEnv(), value, type);
        return JSValue::AttachFinalizer(EtsCoroutine::GetCurrent(), jsvalue);
    }

    napi_value GetNapiValue(napi_env env);

    bool GetBoolean() const
    {
        ASSERT(GetType() == napi_boolean);
        return GetData<bool>();
    }

    double GetNumber() const
    {
        ASSERT(GetType() == napi_number);
        return GetData<double>();
    }

    JSValueStringStorage::CachedEntry GetString() const
    {
        ASSERT(GetType() == napi_string);
        return JSValueStringStorage::CachedEntry(GetData<std::string const *>());
    }

    napi_value GetRefValue(napi_env env)
    {
        napi_value value;
        NAPI_ASSERT_OK(napi_get_reference_value(env, GetNapiRef(), &value));
        return value;
    }

    // Specific for experimental ts2ets_tstype
    static JSValue *CreateTSTypeDerived([[maybe_unused]] Class *klass, [[maybe_unused]] napi_env env,
                                        [[maybe_unused]] napi_value js_val)
    {
        UNREACHABLE();
    }

    static void FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg);

    static constexpr uint32_t GetTypeOffset()
    {
        return MEMBER_OFFSET(JSValue, type_);
    }

    JSValue() = delete;

private:
    static constexpr bool IsRefType(napi_valuetype type)
    {
        return type == napi_object || type == napi_function || type == napi_symbol;
    }

    static constexpr bool IsFinalizableType(napi_valuetype type)
    {
        return type == napi_string || IsRefType(type);
    }

    void SetType(napi_valuetype type)
    {
        type_ = bit_cast<decltype(type_)>(type);
    }

    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T>> SetData(T val)
    {
        static_assert(sizeof(data_) >= sizeof(T));
        memcpy(&data_, &val, sizeof(T));
    }

    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T> && std::is_trivially_constructible_v<T>, T> GetData() const
    {
        static_assert(sizeof(data_) >= sizeof(T));
        T val;
        memcpy(&val, &data_, sizeof(T));
        return val;
    }

    static JSValue *AllocUndefined(EtsCoroutine *coro, InteropCtx *ctx)
    {
        JSValue *js_value;
        {
            auto obj = ObjectHeader::Create(coro, ctx->GetJSValueClass());
            if (UNLIKELY(!obj)) {
                return nullptr;
            }
            js_value = FromCoreType(obj);
        }
        static_assert(napi_undefined == 0);  // zero-initialized
        ASSERT(js_value->GetType() == napi_undefined);
        return js_value;
    }

    // Returns moved js_value
    [[nodiscard]] static JSValue *AttachFinalizer(EtsCoroutine *coro, JSValue *js_value);

    void SetNapiRef(napi_ref ref, napi_valuetype type)
    {
        ASSERT(IsRefType(type));
        SetType(type);
        SetData(ref);
    }

    napi_ref GetNapiRef() const
    {
        ASSERT(IsRefType(GetType()));
        return GetData<napi_ref>();
    }

    void SetUndefined()
    {
        SetType(napi_undefined);
    }

    void SetNull()
    {
        SetType(napi_null);
    }

    void SetBoolean(bool value)
    {
        SetType(napi_boolean);
        SetData(value);
    }

    void SetNumber(double value)
    {
        SetType(napi_number);
        SetData(value);
    }

    void SetString(JSValueStringStorage::CachedEntry value)
    {
        SetType(napi_string);
        SetData(value);
    }

    void SetRefValue(napi_env env, napi_value js_value, napi_valuetype type)
    {
        ASSERT(GetValueType(env, js_value) == type);
        napi_ref js_ref;
        NAPI_ASSERT_OK(napi_create_reference(env, js_value, 1, &js_ref));
        SetNapiRef(js_ref, type);
    }

    FIELD_UNUSED uint32_t type_;
    FIELD_UNUSED uint32_t padding_;
    FIELD_UNUSED uint64_t data_;

    friend class testing::JSValueOffsets;
};

static_assert(JSValue::GetTypeOffset() == sizeof(ObjectHeader));

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_H_
