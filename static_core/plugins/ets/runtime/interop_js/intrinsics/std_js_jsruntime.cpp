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

#include "intrinsics.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_void.h"

namespace panda::ets::interop::js {

namespace notimpl {

[[noreturn]] static void NotImplementedHook()
{
    LOG(FATAL, ETS) << "NotImplementedHook called for JSRuntime intrinsic";
    UNREACHABLE();
}

template <typename R, typename... Args>
static R NotImplementedAdapter([[maybe_unused]] Args... args)
{
    NotImplementedHook();
}

static const IntrinsicsAPI S_INTRINSICS_API = {
    // clang-format off
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    NotImplementedAdapter,
    // clang-format on
};

}  // namespace notimpl

static const IntrinsicsAPI *S_INTRINSICS_API = &notimpl::S_INTRINSICS_API;

PANDA_PUBLIC_API EtsVoid *JSRuntimeIntrinsicsSetIntrinsicsAPI(const IntrinsicsAPI *intrinsics_api)
{
    S_INTRINSICS_API = intrinsics_api;
    return EtsVoid::GetInstance();
}

namespace intrinsics {

EtsVoid *JSRuntimeFinalizationQueueCallbackIntrinsic(EtsObject *obj)
{
    S_INTRINSICS_API->JSRuntimeFinalizationQueueCallback(obj);
    return EtsVoid::GetInstance();
}

JSValue *JSRuntimeNewJSValueDoubleIntrinsic(double v)
{
    return S_INTRINSICS_API->JSRuntimeNewJSValueDouble(v);
}

JSValue *JSRuntimeNewJSValueStringIntrinsic(EtsString *v)
{
    return S_INTRINSICS_API->JSRuntimeNewJSValueString(v);
}

JSValue *JSRuntimeNewJSValueObjectIntrinsic(EtsObject *v)
{
    return S_INTRINSICS_API->JSRuntimeNewJSValueObject(v);
}

double JSRuntimeGetValueDoubleIntrinsic(JSValue *ets_js_value)
{
    return S_INTRINSICS_API->JSRuntimeGetValueDouble(ets_js_value);
}

uint8_t JSRuntimeGetValueBooleanIntrinsic(JSValue *ets_js_value)
{
    return S_INTRINSICS_API->JSRuntimeGetValueBoolean(ets_js_value);
}

EtsString *JSRuntimeGetValueStringIntrinsic(JSValue *ets_js_value)
{
    return S_INTRINSICS_API->JSRuntimeGetValueString(ets_js_value);
}

EtsObject *JSRuntimeGetValueObjectIntrinsic(JSValue *ets_js_value, EtsObject *cls)
{
    return S_INTRINSICS_API->JSRuntimeGetValueObject(ets_js_value, cls);
}

JSValue *JSRuntimeGetPropertyJSValueIntrinsic(JSValue *ets_js_value, EtsString *ets_prop_name)
{
    return S_INTRINSICS_API->JSRuntimeGetPropertyJSValue(ets_js_value, ets_prop_name);
}

double JSRuntimeGetPropertyDoubleIntrinsic(JSValue *ets_js_value, EtsString *ets_prop_name)
{
    return S_INTRINSICS_API->JSRuntimeGetPropertyDouble(ets_js_value, ets_prop_name);
}

EtsString *JSRuntimeGetPropertyStringIntrinsic(JSValue *ets_js_value, EtsString *ets_prop_name)
{
    return S_INTRINSICS_API->JSRuntimeGetPropertyString(ets_js_value, ets_prop_name);
}

EtsVoid *JSRuntimeSetPropertyJSValueIntrinsic(JSValue *ets_js_value, EtsString *ets_prop_name, JSValue *value)
{
    S_INTRINSICS_API->JSRuntimeSetPropertyJSValue(ets_js_value, ets_prop_name, value);
    return EtsVoid::GetInstance();
}

EtsVoid *JSRuntimeSetPropertyDoubleIntrinsic(JSValue *ets_js_value, EtsString *ets_prop_name, double value)
{
    S_INTRINSICS_API->JSRuntimeSetPropertyDouble(ets_js_value, ets_prop_name, value);
    return EtsVoid::GetInstance();
}

EtsVoid *JSRuntimeSetPropertyStringIntrinsic(JSValue *ets_js_value, EtsString *ets_prop_name, EtsString *value)
{
    S_INTRINSICS_API->JSRuntimeSetPropertyString(ets_js_value, ets_prop_name, value);
    return EtsVoid::GetInstance();
}

JSValue *JSRuntimeGetElementJSValueIntrinsic(JSValue *ets_js_value, int32_t index)
{
    return S_INTRINSICS_API->JSRuntimeGetElementJSValue(ets_js_value, index);
}

double JSRuntimeGetElementDoubleIntrinsic(JSValue *ets_js_value, int32_t index)
{
    return S_INTRINSICS_API->JSRuntimeGetElementDouble(ets_js_value, index);
}

JSValue *JSRuntimeGetUndefinedIntrinsic()
{
    return S_INTRINSICS_API->JSRuntimeGetUndefined();
}

JSValue *JSRuntimeGetNullIntrinsic()
{
    return S_INTRINSICS_API->JSRuntimeGetNull();
}

JSValue *JSRuntimeGetGlobalIntrinsic()
{
    return S_INTRINSICS_API->JSRuntimeGetGlobal();
}

JSValue *JSRuntimeCreateObjectIntrinsic()
{
    return S_INTRINSICS_API->JSRuntimeCreateObject();
}

uint8_t JSRuntimeInstanceOfIntrinsic(JSValue *object, JSValue *ctor)
{
    return S_INTRINSICS_API->JSRuntimeInstanceOf(object, ctor);
}

uint8_t JSRuntimeInitJSCallClassIntrinsic(EtsString *cls_name)
{
    return S_INTRINSICS_API->JSRuntimeInitJSCallClass(cls_name);
}

uint8_t JSRuntimeInitJSNewClassIntrinsic(EtsString *cls_name)
{
    return S_INTRINSICS_API->JSRuntimeInitJSNewClass(cls_name);
}

JSValue *JSRuntimeCreateLambdaProxyIntrinsic(EtsObject *lambda)
{
    return S_INTRINSICS_API->JSRuntimeCreateLambdaProxy(lambda);
}

JSValue *JSRuntimeLoadModuleIntrinsic(EtsString *module)
{
    return S_INTRINSICS_API->JSRuntimeLoadModule(module);
}

uint8_t JSRuntimeStrictEqualIntrinsic(JSValue *lhs, JSValue *rhs)
{
    return S_INTRINSICS_API->JSRuntimeStrictEqual(lhs, rhs);
}

// Compiler intrinsics for fast interop
void *CompilerGetJSNamedPropertyIntrinsic(void *val, void *prop_name)
{
    return S_INTRINSICS_API->CompilerGetJSNamedProperty(val, reinterpret_cast<char *>(prop_name));
}

void *CompilerResolveQualifiedJSCallIntrinsic(void *val, EtsString *qname_str)
{
    return S_INTRINSICS_API->CompilerResolveQualifiedJSCall(val, qname_str);
}

void *CompilerJSCallCheckIntrinsic(void *fn)
{
    return S_INTRINSICS_API->CompilerJSCallCheck(fn);
}

void *CompilerJSCallFunctionIntrinsic(void *obj, void *fn, uint32_t argc, void *args)
{
    return S_INTRINSICS_API->CompilerJSCallFunction(obj, fn, argc, args);
}

void CompilerJSCallVoidFunctionIntrinsic(void *obj, void *fn, uint32_t argc, void *args)
{
    S_INTRINSICS_API->CompilerJSCallVoidFunction(obj, fn, argc, args);
}

void *CompilerJSNewInstanceIntrinsic(void *fn, uint32_t argc, void *args)
{
    return S_INTRINSICS_API->CompilerJSNewInstance(fn, argc, args);
}

void *CompilerConvertVoidToLocalIntrinsic()
{
    return S_INTRINSICS_API->CompilerConvertVoidToLocal();
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CONVERT_LOCAL_VALUE(type, cpptype)                                \
    void *CompilerConvert##type##ToLocalIntrinsic(cpptype ets_val)        \
    {                                                                     \
        return S_INTRINSICS_API->CompilerConvert##type##ToLocal(ets_val); \
    }                                                                     \
                                                                          \
    cpptype CompilerConvertLocalTo##type##Intrinsic(void *val)            \
    {                                                                     \
        return S_INTRINSICS_API->CompilerConvertLocalTo##type(val);       \
    }

CONVERT_LOCAL_VALUE(U1, uint8_t)
CONVERT_LOCAL_VALUE(U8, uint8_t)
CONVERT_LOCAL_VALUE(I8, int8_t)
CONVERT_LOCAL_VALUE(U16, uint16_t)
CONVERT_LOCAL_VALUE(I16, int16_t)
CONVERT_LOCAL_VALUE(U32, uint32_t)
CONVERT_LOCAL_VALUE(I32, int32_t)
CONVERT_LOCAL_VALUE(U64, uint64_t)
CONVERT_LOCAL_VALUE(I64, int64_t)
CONVERT_LOCAL_VALUE(F32, float)
CONVERT_LOCAL_VALUE(F64, double)
CONVERT_LOCAL_VALUE(JSValue, JSValue *)

#undef CONVERT_LOCAL_VALUE

void *CompilerConvertRefTypeToLocalIntrinsic(EtsObject *ets_val)
{
    return S_INTRINSICS_API->CompilerConvertRefTypeToLocal(ets_val);
}
EtsString *CompilerConvertLocalToStringIntrinsic(void *val)
{
    return S_INTRINSICS_API->CompilerConvertLocalToString(val);
}

EtsObject *CompilerConvertLocalToRefTypeIntrinsic(void *klass_ptr, void *val)
{
    return S_INTRINSICS_API->CompilerConvertLocalToRefType(klass_ptr, val);
}

void CompilerCreateLocalScopeIntrinsic()
{
    S_INTRINSICS_API->CreateLocalScope();
}

void CompilerDestroyLocalScopeIntrinsic()
{
    S_INTRINSICS_API->CompilerDestroyLocalScope();
}

}  // namespace intrinsics
}  // namespace panda::ets::interop::js
