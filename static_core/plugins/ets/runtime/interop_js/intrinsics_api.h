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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_H_

#include <cstdint>

namespace panda::ets {
class EtsObject;
class EtsString;
class EtsObject;
class EtsClass;
}  // namespace panda::ets

namespace panda::ets::interop::js {

class JSValue;

struct IntrinsicsAPI {
    // NOLINTBEGIN(readability-identifier-naming)
    void (*JSRuntimeFinalizationQueueCallback)(EtsObject *obj);
    JSValue *(*JSRuntimeNewJSValueDouble)(double v);
    JSValue *(*JSRuntimeNewJSValueString)(EtsString *v);
    JSValue *(*JSRuntimeNewJSValueObject)(EtsObject *v);
    double (*JSRuntimeGetValueDouble)(JSValue *ets_js_value);
    uint8_t (*JSRuntimeGetValueBoolean)(JSValue *ets_js_value);
    EtsString *(*JSRuntimeGetValueString)(JSValue *ets_js_value);
    EtsObject *(*JSRuntimeGetValueObject)(JSValue *ets_js_value, EtsClass *cls);
    JSValue *(*JSRuntimeGetPropertyJSValue)(JSValue *ets_js_value, EtsString *ets_prop_name);
    double (*JSRuntimeGetPropertyDouble)(JSValue *ets_js_value, EtsString *ets_prop_name);
    EtsString *(*JSRuntimeGetPropertyString)(JSValue *ets_js_value, EtsString *ets_prop_name);
    void (*JSRuntimeSetPropertyJSValue)(JSValue *ets_js_value, EtsString *ets_prop_name, JSValue *value);
    void (*JSRuntimeSetPropertyDouble)(JSValue *ets_js_value, EtsString *ets_prop_name, double value);
    void (*JSRuntimeSetPropertyString)(JSValue *ets_js_value, EtsString *ets_prop_name, EtsString *value);
    JSValue *(*JSRuntimeGetElementJSValue)(JSValue *ets_js_value, int32_t index);
    double (*JSRuntimeGetElementDouble)(JSValue *ets_js_value, int32_t index);
    JSValue *(*JSRuntimeGetUndefined)();
    JSValue *(*JSRuntimeGetNull)();
    JSValue *(*JSRuntimeGetGlobal)();
    JSValue *(*JSRuntimeCreateObject)();
    uint8_t (*JSRuntimeInstanceOf)(JSValue *object, JSValue *ctor);
    uint8_t (*JSRuntimeInitJSCallClass)(EtsString *cls_name);
    uint8_t (*JSRuntimeInitJSNewClass)(EtsString *cls_name);
    JSValue *(*JSRuntimeCreateLambdaProxy)(EtsObject *lambda);
    JSValue *(*JSRuntimeLoadModule)(EtsString *module);
    uint8_t (*JSRuntimeStrictEqual)(JSValue *lhs, JSValue *rhs);
    void *(*CompilerGetJSNamedProperty)(void *val, char *prop_name);
    void *(*CompilerResolveQualifiedJSCall)(void *val, EtsString *qname_str);
    void *(*CompilerJSCallCheck)(void *fn);
    void *(*CompilerJSCallFunction)(void *obj, void *fn, uint32_t argc, void *args);
    void *(*CompilerJSNewInstance)(void *fn, uint32_t argc, void *args);
    void (*CreateLocalScope)();
    void (*CompilerDestroyLocalScope)();
    void *(*CompilerConvertVoidToLocal)();
    void *(*CompilerConvertU1ToLocal)(bool ets_val);
    void *(*CompilerConvertU8ToLocal)(uint8_t ets_val);
    void *(*CompilerConvertI8ToLocal)(int8_t ets_val);
    void *(*CompilerConvertU16ToLocal)(uint16_t ets_val);
    void *(*CompilerConvertI16ToLocal)(int16_t ets_val);
    void *(*CompilerConvertU32ToLocal)(uint32_t ets_val);
    void *(*CompilerConvertI32ToLocal)(int32_t ets_val);
    void *(*CompilerConvertU64ToLocal)(uint64_t ets_val);
    void *(*CompilerConvertI64ToLocal)(int64_t ets_val);
    void *(*CompilerConvertF32ToLocal)(float ets_val);
    void *(*CompilerConvertF64ToLocal)(double ets_val);
    void *(*CompilerConvertJSValueToLocal)(JSValue *ets_val);
    void *(*CompilerConvertRefTypeToLocal)(EtsObject *ets_val);
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
    EtsObject *(*CompilerConvertLocalToRefType)(void *klass_ptr, void *val);
    // NOLINTEND(readability-identifier-naming)
};

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTRINSICS_API_H_
