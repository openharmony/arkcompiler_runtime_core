/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/xref_object_operator.h"

#include <string>

#include "mem/local_object_handle.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js {

using JSConvertEtsObject = interop::js::JSConvertEtsObject;

XRefObjectOperator::XRefObjectOperator(EtsObject *etsObject) : etsObject_(etsObject) {}

XRefObjectOperator XRefObjectOperator::FromEtsObject(EtsObject *etsObject)
{
    ASSERT(etsObject == nullptr || etsObject->GetClass()->GetRuntimeClass()->IsXRefClass());
    return XRefObjectOperator(etsObject);
}

EtsObject *XRefObjectOperator::AsObject()
{
    return reinterpret_cast<EtsObject *>(this);
}

const EtsObject *XRefObjectOperator::AsObject() const
{
    return reinterpret_cast<const EtsObject *>(this);
}

EtsObject *XRefObjectOperator::GetProperty(EtsCoroutine *coro, const std::string &name) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);
    napi_value jsThis = this->GetNapiValue(coro);

    auto resultTaggedType =
        common::DynamicObjectAccessorUtil::GetProperty(ArkNapiHelper::ToBaseObject(jsThis), name.c_str());
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(resultTaggedType)).value();
}

EtsObject *XRefObjectOperator::GetProperty(EtsCoroutine *coro, EtsObject *keyObject) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsKey = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, keyObject);
    napi_value jsVal = nullptr;
    napi_status jsStatus = napi_get_property(env, jsThis, jsKey, &jsVal);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsVal).value();
}

EtsObject *XRefObjectOperator::GetProperty(EtsCoroutine *coro, const uint32_t index) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsVal = nullptr;
    napi_status jsStatus = napi_get_element(env, jsThis, index, &jsVal);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return {};
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsVal).value();
}

bool XRefObjectOperator::SetProperty(EtsCoroutine *coro, const std::string &name, EtsObject *valueObject) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsValue = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, valueObject);
    napi_status jsStatus = napi_set_named_property(env, jsThis, name.c_str(), jsValue);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return true;
}

bool XRefObjectOperator::SetProperty(EtsCoroutine *coro, EtsObject *keyObject, EtsObject *valueObject) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsKey = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, keyObject);
    napi_value jsValue = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, valueObject);
    napi_status jsStatus = napi_set_property(env, jsThis, jsKey, jsValue);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return true;
}

bool XRefObjectOperator::SetProperty(EtsCoroutine *coro, uint32_t index, EtsObject *valueObject) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsValue = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, valueObject);
    napi_status jsStatus = napi_set_element(env, jsThis, index, jsValue);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return true;
}

bool XRefObjectOperator::IsInstanceOf(EtsCoroutine *coro, const XRefObjectOperator &rhsObject) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsRhs = rhsObject.GetNapiValue(coro);
    bool isInstanceOf = false;
    napi_status jsStatus = napi_instanceof(env, jsThis, jsRhs, &isInstanceOf);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }

    return isInstanceOf;
}

EtsObject *XRefObjectOperator::Invoke(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);
    napi_value jsFunc = this->GetNapiValue(coro);

    // convert static args to dynamic args
    PandaVector<TaggedType> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        if (dynamicArg == nullptr) {
            ctx->ForwardJSException(coro);
            return nullptr;
        }
        auto dynamicArgTaggedType = ArkNapiHelper::GetTaggedType(dynamicArg);
        dynamicArgs.push_back(dynamicArgTaggedType);
    }

    auto undefinedTaggedType = ArkNapiHelper::GetTaggedType(GetUndefined(env));
    auto jsFuncTaggedType = ArkNapiHelper::GetTaggedType(jsFunc);
    TaggedType *jsRetTaggedType = common::DynamicObjectAccessorUtil::CallFunction(
        undefinedTaggedType, jsFuncTaggedType, dynamicArgs.size(), dynamicArgs.data());
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(jsRetTaggedType)).value();
}

EtsObject *XRefObjectOperator::InvokeMethod(EtsCoroutine *coro, const std::string &name,
                                            Span<VMHandle<ObjectHeader>> args) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);
    napi_value jsThis = this->GetNapiValue(coro);

    // convert static args to dynamic args
    PandaVector<napi_value> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        dynamicArgs.push_back(dynamicArg);
    }

    napi_value jsMethod = nullptr;
    napi_status jsStatus = napi_get_named_property(env, jsThis, name.c_str(), &jsMethod);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    napi_value jsRet = nullptr;
    jsStatus = napi_call_function(env, jsThis, jsMethod, dynamicArgs.size(), dynamicArgs.data(), &jsRet);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsRet).value();
}

EtsObject *XRefObjectOperator::InvokeMethod(EtsCoroutine *coro, EtsObject *methodObject,
                                            Span<VMHandle<ObjectHeader>> args) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);
    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsMethod = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, methodObject);

    // convert static args to dynamic args
    PandaVector<TaggedType> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        if (dynamicArg == nullptr) {
            ctx->ForwardJSException(coro);
            return nullptr;
        }
        auto dynamicArgTaggedType = ArkNapiHelper::GetTaggedType(dynamicArg);
        dynamicArgs.push_back(dynamicArgTaggedType);
    }
    auto jsThisTaggedType = ArkNapiHelper::GetTaggedType(jsThis);
    auto jsMethodTaggedType = ArkNapiHelper::GetTaggedType(jsMethod);
    TaggedType *jsRetTaggedType = common::DynamicObjectAccessorUtil::CallFunction(
        jsThisTaggedType, jsMethodTaggedType, dynamicArgs.size(), dynamicArgs.data());
    if (NapiIsExceptionPending(env)) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, ArkNapiHelper::ToNapiValue(jsRetTaggedType)).value();
}

bool XRefObjectOperator::HasProperty(EtsCoroutine *coro, const std::string &name, bool isOwnProperty) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    bool res = false;
    napi_status jsStatus;
    if (isOwnProperty) {
        napi_value jsKey = nullptr;
        NAPI_CHECK_FATAL(napi_create_string_utf8(env, name.c_str(), name.size(), &jsKey));
        jsStatus = napi_has_own_property(env, jsThis, jsKey, &res);
    } else {
        jsStatus = napi_has_named_property(env, jsThis, name.c_str(), &res);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return res;
}

bool XRefObjectOperator::HasProperty(EtsCoroutine *coro, EtsObject *keyObject, bool isOwnProperty) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    napi_value jsKey = XRefObjectOperator::ConvertStaticObjectToDynamic(coro, keyObject);
    bool res = false;
    napi_status jsStatus;
    if (isOwnProperty) {
        jsStatus = napi_has_own_property(env, jsThis, jsKey, &res);
    } else {
        jsStatus = napi_has_property(env, jsThis, jsKey, &res);
    }
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return res;
}

bool XRefObjectOperator::HasProperty(EtsCoroutine *coro, const uint32_t index) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);

    napi_value jsThis = this->GetNapiValue(coro);
    bool res = false;
    napi_status jsStatus = napi_has_element(env, jsThis, index, &res);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return false;
    }
    return res;
}

EtsObject *XRefObjectOperator::Instantiate(EtsCoroutine *coro, Span<VMHandle<ObjectHeader>> args) const
{
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    ScopedNativeCodeThread nativeScope(coro);
    NapiScope jsHandleScope(env);
    napi_value jsCtor = this->GetNapiValue(coro);

    // convert static args to dynamic args
    PandaVector<napi_value> dynamicArgs;
    for (auto &objHeader : args) {
        EtsObject *arg = EtsObject::FromCoreType(objHeader.GetPtr());
        auto dynamicArg = JSConvertEtsObject::WrapWithNullCheck(env, arg);
        dynamicArgs.push_back(dynamicArg);
    }
    napi_value jsRet = nullptr;
    napi_status jsStatus = napi_new_instance(env, jsCtor, dynamicArgs.size(), dynamicArgs.data(), &jsRet);
    if (jsStatus != napi_ok) {
        ctx->ForwardJSException(coro);
        return nullptr;
    }
    return JSConvertEtsObject::UnwrapWithNullCheck(ctx, env, jsRet).value();
}

napi_valuetype XRefObjectOperator::GetValueType(EtsCoroutine *coro) const
{
    // Background: GC-safe is required here!
    // StrictEquals bytecode is not allowed to raise gc.
    // So we cannot use some napi api here, because some napi api may raise a gc.

    // 1. If it is nullptr, then we return napi_undefined
    if (this->etsObject_ == nullptr) {
        return napi_undefined;
    }

    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    // 2. If it is a JSValue, then we use the JSValue's GetNapiValue method
    // Note(MockMockBlack, #ICQS8L): this is a workaround and will be removed
    if (this->etsObject_->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::FromEtsObject(this->etsObject_)->GetType();
    }

    // 3. Otherwise, we assume that it is a XRefObject
    // In this case, this object is either a function or an object.
    if (this->etsObject_->GetClass()->IsFunction()) {
        return napi_function;
    }
    return napi_object;
}

std::string XRefObjectOperator::TypeOf(EtsCoroutine *coro) const
{
    napi_valuetype valueType = this->GetValueType(coro);
    switch (valueType) {
        // NOTE(MockMockBlack): moved this code from JSValue::TypeOf
        case napi_string:
            return "string";
        case napi_undefined:
            return "undefined";
        case napi_null:
            return "object";
        case napi_number:
            return "number";
        case napi_bigint:
            return "bigint";
        case napi_function:
            return "function";
        default:
            return "object";
    }
}

bool XRefObjectOperator::IsTrue(EtsCoroutine *coro) const
{
    // Background: GC-safe is required here!
    // StrictEquals bytecode is not allowed to raise gc.
    // So we cannot use some napi api here, because some napi api may raise a gc.

    // 1. If it is nullptr, then we return false
    if (this->etsObject_ == nullptr) {
        return false;
    }

    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_ETS_TO_JS(coro);
    auto env = ctx->GetJSEnv();
    NapiScope jsHandleScope(env);

    // 2. If it is a JSValue, then we use the JSValue's IsTrue method
    // Note(MockMockBlack, #ICQS8L): this is a workaround and will be removed
    if (this->etsObject_->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::FromEtsObject(this->etsObject_)->IsTrue();
    }

    // 3. Otherwise, we assume that it is a XRefObject
    // In this case, this object is either a function or an object,
    // so a `true` value is returned.
    return true;
}

napi_value XRefObjectOperator::GetNapiValue(EtsCoroutine *coro) const
{
    // NOTE(MockMockBlack): will just get from shared_refenece_table
    // after jsvalue it put into shared_reference_table
    auto ctx = InteropCtx::Current(coro);
    auto env = ctx->GetJSEnv();

    // 1. If it is undefiend(nullptr in native),
    // then return an undefined value
    if (this->etsObject_ == nullptr) {
        return GetUndefined(env);
    }

    // 2. If it is already in shared_reference_table,
    // get it from there
    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (UNLIKELY(storage->HasReference(this->etsObject_, env))) {
        auto jsObject = storage->GetJsObject(this->etsObject_, env);
        return jsObject;
    }

    // 3. otherwise, we assume that it is a jsvalue object,
    // so we will treat it as a JSValue
    auto jsValueObject = JSValue::FromEtsObject(this->etsObject_);
    auto jsObject = jsValueObject->GetNapiValue(env);
    // NOTE(MockMockBlack): will be removed after this is stable
    if (UNLIKELY(jsObject == nullptr)) {
        InteropCtx::Fatal("Failed to get NAPI value for XRefObject");
    }

    return jsObject;
}

bool XRefObjectOperator::StrictEquals(EtsCoroutine *coro, const XRefObjectOperator &lhs, const XRefObjectOperator &rhs)
{
    // Background: GC-safe is required here!
    // StrictEquals bytecode is not allowed to raise gc.
    // So we cannot use some napi api here, because some napi api may raise a gc.

    // 1. lhs or rhs is both nullptr,
    // so we can just return false
    if (lhs.etsObject_ == nullptr && rhs.etsObject_ == nullptr) {
        return true;
    }

    // 2. if one and only one of them is nullptr,
    // then we can just return false
    if (lhs.etsObject_ == nullptr || rhs.etsObject_ == nullptr) {
        return false;
    }

    // 3. If both of them are JSValue,
    // then we can use JSValue's StrictEquals method
    // Note(MockMockBlack, #ICQS8L): this is a workaround and will be removed
    if (lhs.etsObject_->GetClass() == PlatformTypes(coro)->interopJSValue &&
        rhs.etsObject_->GetClass() == PlatformTypes(coro)->interopJSValue) {
        return JSValue::FromEtsObject(lhs.etsObject_)->StrictEquals(JSValue::FromEtsObject(rhs.etsObject_));
    }

    // 4. Otherwise, we just compare their pointers
    return lhs.etsObject_ == rhs.etsObject_;
}

napi_value XRefObjectOperator::ConvertStaticObjectToDynamic(EtsCoroutine *coro, EtsObject *object)
{
    auto ctx = interop::js::InteropCtx::Current(coro);

    // static undefined is nullptr in runtime
    // handle undefined case
    if (UNLIKELY(object == nullptr)) {
        return interop::js::GetUndefined(ctx->GetJSEnv());
    }

    // if it a XRefObject, then we can just return its napi value
    if (object->GetClass()->GetRuntimeClass()->IsXRefClass()) {
        auto xRefObjectOperator = interop::js::XRefObjectOperator::FromEtsObject(object);
        return xRefObjectOperator.GetNapiValue(coro);
    }

    return JSRefConvertResolve(ctx, object->GetClass()->GetRuntimeClass())->Wrap(ctx, object);
}
}  // namespace ark::ets::interop::js