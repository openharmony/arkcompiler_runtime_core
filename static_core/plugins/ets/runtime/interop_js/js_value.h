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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_H_

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/include/coretypes/class.h"
#include "utils/small_vector.h"
#include <node_api.h>

namespace ark::ets::interop::js {

namespace testing {
class JSValueOffsets;
class ESValueOffsets;
}  // namespace testing

struct JSValueMemberOffsets;

class JSValue : private EtsObject {
public:
    static JSValue *FromEtsObject(EtsObject *etsObject)
    {
        ASSERT(etsObject->GetClass() == EtsClass::FromRuntimeClass(InteropCtx::Current()->GetJSValueClass()));
        return static_cast<JSValue *>(etsObject);
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
        jsvalue->SetRefValue(ctx, value, type);
        return jsvalue;
    }

    // prefer JSConvertJSValue::WrapWithNullCheck
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

    std::pair<SmallVector<uint64_t, 4U>, int> *GetBigInt() const
    {
        ASSERT(GetType() == napi_bigint);
        return GetData<std::pair<SmallVector<uint64_t, 4U>, int> *>();
    }

    JSValueStringStorage::CachedEntry GetString() const
    {
        ASSERT(GetType() == napi_string);
        return JSValueStringStorage::CachedEntry(GetData<std::string const *>());
    }

    napi_value GetRefValue(napi_env env)
    {
        napi_value value;
        auto napiRef = GetNapiRef(env);
        NAPI_ASSERT_OK(napi_get_reference_value(env, napiRef, &value));
        return value;
    }

    static void FinalizeETSWeak(InteropCtx *ctx, EtsObject *cbarg);

    static constexpr uint32_t GetTypeOffset()
    {
        return MEMBER_OFFSET(JSValue, type_);
    }

    JSValue() = delete;

private:
    static JSValue *CreateByType(InteropCtx *ctx, napi_env env, napi_value nvalue, napi_valuetype jsType,
                                 JSValue *jsvalue);

    static constexpr bool IsRefType(napi_valuetype type)
    {
        return type == napi_object || type == napi_function || type == napi_symbol;
    }

    static constexpr bool IsFinalizableType(napi_valuetype type)
    {
        return type == napi_string || type == napi_bigint || IsRefType(type);
    }

    void SetType(napi_valuetype type)
    {
        type_ = bit_cast<decltype(type_)>(type);
    }

    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T>> SetData(T val)
    {
        static_assert(sizeof(data_) >= sizeof(T));
        std::copy_n(reinterpret_cast<uint8_t *>(&val), sizeof(T), reinterpret_cast<uint8_t *>(&data_));
    }

    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T> && std::is_trivially_constructible_v<T>, T> GetData() const
    {
        static_assert(sizeof(data_) >= sizeof(T));
        T val;
        std::copy_n(reinterpret_cast<const uint8_t *>(&data_), sizeof(T), reinterpret_cast<uint8_t *>(&val));
        return val;
    }

    static JSValue *AllocUndefined(EtsCoroutine *coro, InteropCtx *ctx)
    {
        JSValue *jsValue;
        {
            auto obj = ObjectHeader::Create(coro, ctx->GetJSValueClass());
            if (UNLIKELY(!obj)) {
                return nullptr;
            }
            jsValue = FromCoreType(obj);
        }
        static_assert(napi_undefined == 0);  // zero-initialized
        ASSERT(jsValue->GetType() == napi_undefined);
        return jsValue;
    }

    // Returns moved jsValue
    [[nodiscard]] static JSValue *AttachFinalizer(EtsCoroutine *coro, JSValue *jsValue);

    napi_ref GetNapiRef(napi_env env) const
    {
        ASSERT(IsRefType(GetType()));

        ets_proxy::SharedReference *sharedRef = GetData<ets_proxy::SharedReference *>();

        // Interop ctx check:
        // check if the ctx is the same as the one that created the reference
        if (sharedRef->GetCtx()->GetJSEnv() != env) {
            // NOTE(MockMockBlack, #24062): to be replaced with a runtime exception
            InteropFatal("InteropFatal, interop object must be used in the same interopCtx as it was created.");
        }

        return sharedRef->GetJsRef();
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

    void SetBigInt(std::pair<SmallVector<uint64_t, 4U>, int> &&value)
    {
        SetType(napi_bigint);
        SetData(new std::pair<SmallVector<uint64_t, 4U>, int>(std::move(value)));
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    }

    void SetRefValue(InteropCtx *ctx, napi_value jsValue, napi_valuetype type)
    {
        ASSERT(GetValueType(ctx->GetJSEnv(), jsValue) == type);
        ASSERT(IsRefType(type));
        SetType(type);
        SetData(ctx->GetSharedRefStorage()->CreateJSObjectRef(ctx, this, jsValue));
    }

    FIELD_UNUSED uint32_t type_;
    FIELD_UNUSED uint32_t padding_;
    FIELD_UNUSED uint64_t data_;

    friend class testing::JSValueOffsets;
};

class ESValue : private EtsObject {
public:
    JSValue *GetEo()
    {
        return JSValue::FromCoreType(ObjectAccessor::GetObject(this, GetEoOffset()));
    }

    static constexpr uint32_t GetEoOffset()
    {
        return MEMBER_OFFSET(ESValue, eo_);
    }

    ESValue() = delete;
    ~ESValue() = delete;

    NO_COPY_SEMANTIC(ESValue);
    NO_MOVE_SEMANTIC(ESValue);

private:
    FIELD_UNUSED ObjectPointer<JSValue> eo_;
    FIELD_UNUSED EtsBoolean isStatic_;
    friend class testing::ESValueOffsets;
};

static_assert(JSValue::GetTypeOffset() == sizeof(ObjectHeader));
static_assert(ESValue::GetEoOffset() == sizeof(ObjectHeader));

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_H_
