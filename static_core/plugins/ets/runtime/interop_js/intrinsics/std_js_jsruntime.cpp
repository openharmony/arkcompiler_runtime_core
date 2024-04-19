/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

namespace ark::ets::interop::js {

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

PANDA_PUBLIC_API void JSRuntimeIntrinsicsSetIntrinsicsAPI(const IntrinsicsAPI *intrinsicsApi)
{
    S_INTRINSICS_API = intrinsicsApi;
}

namespace intrinsics {

void JSRuntimeFinalizationRegistryCallbackIntrinsic(EtsObject *obj)
{
    S_INTRINSICS_API->JSRuntimeFinalizationRegistryCallback(obj);
}

JSValue *JSRuntimeNewJSValueDoubleIntrinsic(double v)
{
    return S_INTRINSICS_API->JSRuntimeNewJSValueDouble(v);
}

JSValue *JSRuntimeNewJSValueBooleanIntrinsic(uint8_t v)
{
    return S_INTRINSICS_API->JSRuntimeNewJSValueBoolean(v);
}

JSValue *JSRuntimeNewJSValueStringIntrinsic(EtsString *v)
{
    return S_INTRINSICS_API->JSRuntimeNewJSValueString(v);
}

JSValue *JSRuntimeNewJSValueObjectIntrinsic(EtsObject *v)
{
    return S_INTRINSICS_API->JSRuntimeNewJSValueObject(v);
}

double JSRuntimeGetValueDoubleIntrinsic(JSValue *etsJsValue)
{
    return S_INTRINSICS_API->JSRuntimeGetValueDouble(etsJsValue);
}

uint8_t JSRuntimeGetValueBooleanIntrinsic(JSValue *etsJsValue)
{
    return S_INTRINSICS_API->JSRuntimeGetValueBoolean(etsJsValue);
}

EtsString *JSRuntimeGetValueStringIntrinsic(JSValue *etsJsValue)
{
    return S_INTRINSICS_API->JSRuntimeGetValueString(etsJsValue);
}

EtsObject *JSRuntimeGetValueObjectIntrinsic(JSValue *etsJsValue, EtsObject *cls)
{
    return S_INTRINSICS_API->JSRuntimeGetValueObject(etsJsValue, cls);
}

JSValue *JSRuntimeGetPropertyJSValueIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return S_INTRINSICS_API->JSRuntimeGetPropertyJSValue(etsJsValue, etsPropName);
}

double JSRuntimeGetPropertyDoubleIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return S_INTRINSICS_API->JSRuntimeGetPropertyDouble(etsJsValue, etsPropName);
}

EtsString *JSRuntimeGetPropertyStringIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return S_INTRINSICS_API->JSRuntimeGetPropertyString(etsJsValue, etsPropName);
}

uint8_t JSRuntimeGetPropertyBooleanIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return static_cast<uint8_t>(S_INTRINSICS_API->JSRuntimeGetPropertyBoolean(etsJsValue, etsPropName));
}

void JSRuntimeSetPropertyJSValueIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, JSValue *value)
{
    S_INTRINSICS_API->JSRuntimeSetPropertyJSValue(etsJsValue, etsPropName, value);
}

void JSRuntimeSetPropertyDoubleIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, double value)
{
    S_INTRINSICS_API->JSRuntimeSetPropertyDouble(etsJsValue, etsPropName, value);
}

void JSRuntimeSetPropertyStringIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, EtsString *value)
{
    S_INTRINSICS_API->JSRuntimeSetPropertyString(etsJsValue, etsPropName, value);
}

void JSRuntimeSetPropertyBooleanIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, uint8_t value)
{
    S_INTRINSICS_API->JSRuntimeSetPropertyBoolean(etsJsValue, etsPropName, static_cast<bool>(value));
}

JSValue *JSRuntimeGetElementJSValueIntrinsic(JSValue *etsJsValue, int32_t index)
{
    return S_INTRINSICS_API->JSRuntimeGetElementJSValue(etsJsValue, index);
}

double JSRuntimeGetElementDoubleIntrinsic(JSValue *etsJsValue, int32_t index)
{
    return S_INTRINSICS_API->JSRuntimeGetElementDouble(etsJsValue, index);
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

uint8_t JSRuntimeInitJSCallClassIntrinsic(EtsString *clsName)
{
    return S_INTRINSICS_API->JSRuntimeInitJSCallClass(clsName);
}

uint8_t JSRuntimeInitJSNewClassIntrinsic(EtsString *clsName)
{
    return S_INTRINSICS_API->JSRuntimeInitJSNewClass(clsName);
}

JSValue *JSRuntimeLoadModuleIntrinsic(EtsString *module)
{
    return S_INTRINSICS_API->JSRuntimeLoadModule(module);
}

uint8_t JSRuntimeStrictEqualIntrinsic(JSValue *lhs, JSValue *rhs)
{
    return S_INTRINSICS_API->JSRuntimeStrictEqual(lhs, rhs);
}

EtsString *JSValueToStringIntrinsic(JSValue *object)
{
    return S_INTRINSICS_API->JSValueToString(object);
}

// Compiler intrinsics for fast interop
void *CompilerGetJSNamedPropertyIntrinsic(void *val, void *propName)
{
    return S_INTRINSICS_API->CompilerGetJSNamedProperty(val, reinterpret_cast<char *>(propName));
}

void *CompilerGetJSPropertyIntrinsic(void *val, void *prop)
{
    return S_INTRINSICS_API->CompilerGetJSProperty(val, prop);
}

void *CompilerGetJSElementIntrinsic(void *val, int32_t index)
{
    return S_INTRINSICS_API->CompilerGetJSElement(val, index);
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
#define CONVERT_LOCAL_VALUE(type, cpptype)                               \
    void *CompilerConvert##type##ToLocalIntrinsic(cpptype etsVal)        \
    {                                                                    \
        return S_INTRINSICS_API->CompilerConvert##type##ToLocal(etsVal); \
    }                                                                    \
                                                                         \
    cpptype CompilerConvertLocalTo##type##Intrinsic(void *val)           \
    {                                                                    \
        return S_INTRINSICS_API->CompilerConvertLocalTo##type(val);      \
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

void *CompilerConvertRefTypeToLocalIntrinsic(EtsObject *etsVal)
{
    return S_INTRINSICS_API->CompilerConvertRefTypeToLocal(etsVal);
}
EtsString *CompilerConvertLocalToStringIntrinsic(void *val)
{
    return S_INTRINSICS_API->CompilerConvertLocalToString(val);
}

EtsObject *CompilerConvertLocalToRefTypeIntrinsic(void *klassPtr, void *val)
{
    return S_INTRINSICS_API->CompilerConvertLocalToRefType(klassPtr, val);
}

void CompilerCreateLocalScopeIntrinsic()
{
    S_INTRINSICS_API->CreateLocalScope();
}

void CompilerDestroyLocalScopeIntrinsic()
{
    S_INTRINSICS_API->CompilerDestroyLocalScope();
}

void *CompilerLoadJSConstantPoolIntrinsic()
{
    return S_INTRINSICS_API->CompilerLoadJSConstantPool();
}

void CompilerInitJSCallClassForCtxIntrinsic(void *klass)
{
    return S_INTRINSICS_API->CompilerInitJSCallClassForCtx(klass);
}

}  // namespace intrinsics
}  // namespace ark::ets::interop::js
