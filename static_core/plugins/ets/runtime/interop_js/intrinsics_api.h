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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_H_

#include <cstdint>

namespace ark::ets {
class EtsObject;
class EtsString;
class EtsObject;
class EtsClass;
}  // namespace ark::ets

namespace ark::ets::interop::js {

class JSValue;

struct IntrinsicsAPI {
    // NOLINTBEGIN(readability-identifier-naming)
    void (*JSRuntimeFinalizationRegistryCallback)(EtsObject *obj);
    JSValue *(*JSRuntimeNewJSValueDouble)(double v);
    JSValue *(*JSRuntimeNewJSValueBoolean)(uint8_t v);
    JSValue *(*JSRuntimeNewJSValueString)(EtsString *v);
    JSValue *(*JSRuntimeNewJSValueObject)(EtsObject *v);
    double (*JSRuntimeGetValueDouble)(JSValue *etsJsValue);
    uint8_t (*JSRuntimeGetValueBoolean)(JSValue *etsJsValue);
    EtsString *(*JSRuntimeGetValueString)(JSValue *etsJsValue);
    EtsObject *(*JSRuntimeGetValueObject)(JSValue *etsJsValue, EtsObject *cls);
    JSValue *(*JSRuntimeGetPropertyJSValue)(JSValue *etsJsValue, EtsString *etsPropName);
    double (*JSRuntimeGetPropertyDouble)(JSValue *etsJsValue, EtsString *etsPropName);
    bool (*JSRuntimeGetPropertyBoolean)(JSValue *etsJsValue, EtsString *etsPropName);
    EtsString *(*JSRuntimeGetPropertyString)(JSValue *etsJsValue, EtsString *etsPropName);
    void (*JSRuntimeSetPropertyJSValue)(JSValue *etsJsValue, EtsString *etsPropName, JSValue *value);
    void (*JSRuntimeSetPropertyDouble)(JSValue *etsJsValue, EtsString *etsPropName, double value);
    void (*JSRuntimeSetPropertyBoolean)(JSValue *etsJsValue, EtsString *etsPropName, bool value);
    void (*JSRuntimeSetPropertyString)(JSValue *etsJsValue, EtsString *etsPropName, EtsString *value);
    JSValue *(*JSRuntimeGetElementJSValue)(JSValue *etsJsValue, int32_t index);
    double (*JSRuntimeGetElementDouble)(JSValue *etsJsValue, int32_t index);
    JSValue *(*JSRuntimeGetUndefined)();
    JSValue *(*JSRuntimeGetNull)();
    JSValue *(*JSRuntimeGetGlobal)();
    JSValue *(*JSRuntimeCreateObject)();
    uint8_t (*JSRuntimeInstanceOf)(JSValue *object, JSValue *ctor);
    uint8_t (*JSRuntimeInitJSCallClass)(EtsString *cls_name);
    uint8_t (*JSRuntimeInitJSNewClass)(EtsString *cls_name);
    JSValue *(*JSRuntimeLoadModule)(EtsString *module);
    uint8_t (*JSRuntimeStrictEqual)(JSValue *lhs, JSValue *rhs);
    EtsString *(*JSValueToString)(JSValue *obj);
    void *(*CompilerGetJSNamedProperty)(void *val, char *propName);
    void *(*CompilerGetJSProperty)(void *val, void *prop);
    void *(*CompilerGetJSElement)(void *val, int32_t index);
    void *(*CompilerJSCallCheck)(void *fn);
    void *(*CompilerJSCallFunction)(void *obj, void *fn, uint32_t argc, void *args);
    void (*CompilerJSCallVoidFunction)(void *obj, void *fn, uint32_t argc, void *args);
    void *(*CompilerJSNewInstance)(void *fn, uint32_t argc, void *args);
    void (*CreateLocalScope)();
    void (*CompilerDestroyLocalScope)();
    void *(*CompilerLoadJSConstantPool)();
    void (*CompilerInitJSCallClassForCtx)(void *klassPtr);
    void *(*CompilerConvertVoidToLocal)();
    void *(*CompilerConvertU1ToLocal)(bool etsVal);
    void *(*CompilerConvertU8ToLocal)(uint8_t etsVal);
    void *(*CompilerConvertI8ToLocal)(int8_t etsVal);
    void *(*CompilerConvertU16ToLocal)(uint16_t etsVal);
    void *(*CompilerConvertI16ToLocal)(int16_t etsVal);
    void *(*CompilerConvertU32ToLocal)(uint32_t etsVal);
    void *(*CompilerConvertI32ToLocal)(int32_t etsVal);
    void *(*CompilerConvertU64ToLocal)(uint64_t etsVal);
    void *(*CompilerConvertI64ToLocal)(int64_t etsVal);
    void *(*CompilerConvertF32ToLocal)(float etsVal);
    void *(*CompilerConvertF64ToLocal)(double etsVal);
    void *(*CompilerConvertJSValueToLocal)(JSValue *etsVal);
    void *(*CompilerConvertRefTypeToLocal)(EtsObject *etsVal);
    bool (*CompilerConvertLocalToU1)(void *val);
    uint8_t (*CompilerConvertLocalToU8)(void *val);
    int8_t (*CompilerConvertLocalToI8)(void *val);
    uint16_t (*CompilerConvertLocalToU16)(void *val);
    int16_t (*CompilerConvertLocalToI16)(void *val);
    uint32_t (*CompilerConvertLocalToU32)(void *val);
    int32_t (*CompilerConvertLocalToI32)(void *val);
    uint64_t (*CompilerConvertLocalToU64)(void *val);
    int64_t (*CompilerConvertLocalToI64)(void *val);
    float (*CompilerConvertLocalToF32)(void *val);
    double (*CompilerConvertLocalToF64)(void *val);
    JSValue *(*CompilerConvertLocalToJSValue)(void *val);
    EtsString *(*CompilerConvertLocalToString)(void *val);
    EtsObject *(*CompilerConvertLocalToRefType)(void *klassPtr, void *val);
    // NOLINTEND(readability-identifier-naming)
};

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_H_
