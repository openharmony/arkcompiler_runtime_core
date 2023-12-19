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

#include <sstream>
#include <string>
#include <string_view>
#include "include/coretypes/array.h"
#include "include/managed_thread.h"
#include "macros.h"
#include "mem/vm_handle.h"
#include "napi/ets_napi.h"
#include "runtime/handle_scope-inl.h"
#include "intrinsics.h"
#include "type.h"
#include "types/ets_array.h"
#include "types/ets_class.h"
#include "types/ets_primitives.h"
#include "types/ets_box_primitive-inl.h"
#include "types/ets_string.h"
#include "utils/json_builder.h"

namespace {
ark::JsonObjectBuilder ObjectToJSON(ark::ets::EtsObject *d);

std::string EtsCharToString(ets_char data)
{
    constexpr auto HIGH_SURROGATE_MIN = 0xD800;
    constexpr auto LOW_SURROGATE_MAX = 0xDFFF;
    if ((data < HIGH_SURROGATE_MIN) || (data > LOW_SURROGATE_MAX)) {
        return std::string(reinterpret_cast<const char *>(&data), 1);
    }
    return std::string(reinterpret_cast<const char *>(&data), 2);
}

std::string_view EtsStringToView(ark::ets::EtsString *s)
{
    std::string_view str;
    if (s->IsUtf16()) {
        str = std::string_view(reinterpret_cast<const char *>(s->GetDataUtf16()), s->GetUtf16Length() - 1);
    } else {
        str = std::string_view(reinterpret_cast<const char *>(s->GetDataMUtf8()), s->GetMUtf8Length() - 1);
    }
    return str;
}

template <typename PrimArr>
void EtsPrimitiveArrayToJSON(ark::JsonArrayBuilder &jsonBuilder, PrimArr *arrPtr)
{
    ASSERT(arrPtr->GetClass()->IsArrayClass() && arrPtr->IsPrimitive());
    auto len = arrPtr->GetLength();
    for (size_t i = 0; i < len; ++i) {
        jsonBuilder.Add(arrPtr->Get(i));
    }
}

template <>
void EtsPrimitiveArrayToJSON(ark::JsonArrayBuilder &jsonBuilder, ark::ets::EtsBooleanArray *arrPtr)
{
    ASSERT(arrPtr->IsPrimitive());
    auto len = arrPtr->GetLength();
    for (size_t i = 0; i < len; ++i) {
        if (arrPtr->Get(i) != 0) {
            jsonBuilder.Add(true);
        } else {
            jsonBuilder.Add(false);
        }
    }
}

template <>
void EtsPrimitiveArrayToJSON(ark::JsonArrayBuilder &jsonBuilder, ark::ets::EtsCharArray *arrPtr)
{
    ASSERT(arrPtr->IsPrimitive());
    auto len = arrPtr->GetLength();
    for (size_t i = 0; i < len; ++i) {
        auto data = arrPtr->Get(i);
        jsonBuilder.Add(EtsCharToString(data));
    }
}

void EtsTypedPrimitiveArrayToJSON(ark::JsonArrayBuilder &jsonBuilder, ark::ets::EtsArray *arrPtr)
{
    ark::panda_file::Type::TypeId componentType =
        arrPtr->GetCoreType()->ClassAddr<ark::Class>()->GetComponentType()->GetType().GetId();
    switch (componentType) {
        case ark::panda_file::Type::TypeId::U1: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsBooleanArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::I8: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsByteArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::I16: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsShortArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::U16: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsCharArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::I32: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsIntArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::F32: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsFloatArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::F64: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsDoubleArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::I64: {
            EtsPrimitiveArrayToJSON(jsonBuilder, reinterpret_cast<ark::ets::EtsLongArray *>(arrPtr));
            break;
        }
        case ark::panda_file::Type::TypeId::U8:
        case ark::panda_file::Type::TypeId::U32:
        case ark::panda_file::Type::TypeId::U64:
        case ark::panda_file::Type::TypeId::REFERENCE:
        case ark::panda_file::Type::TypeId::TAGGED:
        case ark::panda_file::Type::TypeId::INVALID:
        case ark::panda_file::Type::TypeId::VOID:
        default:
            UNREACHABLE();
            break;
    }
}

void EtsBoxedClassToJSON(ark::JsonArrayBuilder &jsonBuilder, const char *typeDesc, ark::ets::EtsObject *d)
{
    if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsBoolean>::FromCoreType(d)->GetValue());
    } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_BYTE) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsByte>::FromCoreType(d)->GetValue());
    } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_CHAR) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsChar>::FromCoreType(d)->GetValue());
    } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_SHORT) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsShort>::FromCoreType(d)->GetValue());
    } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_INT) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsInt>::FromCoreType(d)->GetValue());
    } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_LONG) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsLong>::FromCoreType(d)->GetValue());
    } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsFloat>::FromCoreType(d)->GetValue());
    } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
        jsonBuilder.Add(ark::ets::EtsBoxPrimitive<ark::ets::EtsDouble>::FromCoreType(d)->GetValue());
    } else {
        UNREACHABLE();
    }
}

ark::JsonArrayBuilder EtsArrayToJSON(ark::ets::EtsArray *arrPtr)
{
    auto jsonBuilder = ark::JsonArrayBuilder();
    if (arrPtr->IsPrimitive()) {
        EtsTypedPrimitiveArrayToJSON(jsonBuilder, arrPtr);
    } else {
        auto arrObjPtr = reinterpret_cast<ark::ets::EtsObjectArray *>(arrPtr);
        auto len = arrObjPtr->GetLength();
        for (size_t i = 0; i < len; ++i) {
            auto d = arrObjPtr->Get(i);
            auto dCls = d->GetClass();
            auto typeDesc = dCls->GetDescriptor();
            auto arrayClassCb = [d](ark::JsonArrayBuilder &x) {
                x = EtsArrayToJSON(reinterpret_cast<ark::ets::EtsArray *>(d));
            };
            auto objectCb = [d](ark::JsonObjectBuilder &x) { x = ObjectToJSON(d); };
            if (dCls->IsStringClass()) {
                auto sPtr = reinterpret_cast<ark::ets::EtsString *>(d);
                jsonBuilder.Add(EtsStringToView(sPtr));
            } else if (dCls->IsArrayClass()) {
                jsonBuilder.Add(arrayClassCb);
            } else if (dCls->IsBoxedClass()) {
                EtsBoxedClassToJSON(jsonBuilder, typeDesc, d);
            } else {
                jsonBuilder.Add(objectCb);
            }
        }
    }
    return jsonBuilder;
}

void AddPropertyForReferenceType(ark::JsonObjectBuilder &curJson, const ark::Field &f, const char *fieldName,
                                 ark::ets::EtsObject *d)
{
    auto *fPtr = d->GetFieldObject(f.GetOffset());
    if (fPtr != nullptr) {
        auto fCls = fPtr->GetClass();
        auto typeDesc = fCls->GetDescriptor();
        if (fCls->IsStringClass()) {
            auto sPtr = reinterpret_cast<ark::ets::EtsString *>(fPtr);
            curJson.AddProperty(fieldName, EtsStringToView(sPtr));
        } else if (fCls->IsArrayClass()) {
            auto aPtr = reinterpret_cast<ark::ets::EtsArray *>(fPtr);
            curJson.AddProperty(fieldName, [aPtr](ark::JsonArrayBuilder &x) { x = EtsArrayToJSON(aPtr); });
        } else if (fCls->IsBoxedClass()) {
            if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
                curJson.AddProperty(
                    fieldName,
                    static_cast<bool>(ark::ets::EtsBoxPrimitive<ark::ets::EtsBoolean>::FromCoreType(fPtr)->GetValue()));
            } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_BYTE) {
                curJson.AddProperty(fieldName,
                                    ark::ets::EtsBoxPrimitive<ark::ets::EtsByte>::FromCoreType(fPtr)->GetValue());
            } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_CHAR) {
                curJson.AddProperty(fieldName,
                                    ark::ets::EtsBoxPrimitive<ark::ets::EtsChar>::FromCoreType(fPtr)->GetValue());
            } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_SHORT) {
                curJson.AddProperty(fieldName,
                                    ark::ets::EtsBoxPrimitive<ark::ets::EtsShort>::FromCoreType(fPtr)->GetValue());
            } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_INT) {
                curJson.AddProperty(fieldName,
                                    ark::ets::EtsBoxPrimitive<ark::ets::EtsInt>::FromCoreType(fPtr)->GetValue());
            } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_LONG) {
                curJson.AddProperty(fieldName,
                                    ark::ets::EtsBoxPrimitive<ark::ets::EtsLong>::FromCoreType(fPtr)->GetValue());
            } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
                curJson.AddProperty(fieldName,
                                    ark::ets::EtsBoxPrimitive<ark::ets::EtsFloat>::FromCoreType(fPtr)->GetValue());
            } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
                curJson.AddProperty(fieldName,
                                    ark::ets::EtsBoxPrimitive<ark::ets::EtsDouble>::FromCoreType(fPtr)->GetValue());
            } else {
                UNREACHABLE();
            }
        } else {
            curJson.AddProperty(fieldName, [fPtr](ark::JsonObjectBuilder &x) { x = ObjectToJSON(fPtr); });
        }
    } else {
        curJson.AddProperty(fieldName, std::nullptr_t());
    }
}

void AddFieldsToJSON(ark::JsonObjectBuilder &curJson, const ark::Span<ark::Field> &fields, ark::ets::EtsObject *d)
{
    for (const auto &f : fields) {
        ASSERT(f.IsStatic() == false);
        auto fieldName = reinterpret_cast<const char *>(f.GetName().data);

        switch (f.GetTypeId()) {
            case ark::panda_file::Type::TypeId::U1:
                curJson.AddProperty(fieldName, d->GetFieldPrimitive<bool>(f.GetOffset()));
                break;
            case ark::panda_file::Type::TypeId::I8:
                curJson.AddProperty(fieldName, d->GetFieldPrimitive<int8_t>(f.GetOffset()));
                break;
            case ark::panda_file::Type::TypeId::I16:
                curJson.AddProperty(fieldName, d->GetFieldPrimitive<int16_t>(f.GetOffset()));
                break;
            case ark::panda_file::Type::TypeId::U16:
                curJson.AddProperty(fieldName, EtsCharToString(d->GetFieldPrimitive<ets_char>(f.GetOffset())));
                break;
            case ark::panda_file::Type::TypeId::I32:
                curJson.AddProperty(fieldName, d->GetFieldPrimitive<int32_t>(f.GetOffset()));
                break;
            case ark::panda_file::Type::TypeId::F32:
                curJson.AddProperty(fieldName, d->GetFieldPrimitive<float>(f.GetOffset()));
                break;
            case ark::panda_file::Type::TypeId::F64:
                curJson.AddProperty(fieldName, d->GetFieldPrimitive<double>(f.GetOffset()));
                break;
            case ark::panda_file::Type::TypeId::I64:
                curJson.AddProperty(fieldName, d->GetFieldPrimitive<int64_t>(f.GetOffset()));
                break;
            case ark::panda_file::Type::TypeId::REFERENCE: {
                AddPropertyForReferenceType(curJson, f, fieldName, d);
                break;
            }
            case ark::panda_file::Type::TypeId::U8:
            case ark::panda_file::Type::TypeId::U32:
            case ark::panda_file::Type::TypeId::U64:
            case ark::panda_file::Type::TypeId::INVALID:
            case ark::panda_file::Type::TypeId::VOID:
            case ark::panda_file::Type::TypeId::TAGGED:
                UNREACHABLE();
                break;
        }
    }
}

ark::JsonObjectBuilder ObjectToJSON(ark::ets::EtsObject *d)
{
    ASSERT(d != nullptr);
    ark::ets::EtsClass *kls = d->GetClass();
    ASSERT(kls != nullptr);

    // Only instance fields are required according to JS/TS JSON.stringify behaviour
    auto curJson = ark::JsonObjectBuilder();
    kls->EnumerateBaseClasses([&](ark::ets::EtsClass *c) {
        AddFieldsToJSON(curJson, c->GetRuntimeClass()->GetInstanceFields(), d);
        return false;
    });
    return curJson;
}
}  // namespace

namespace ark::ets::intrinsics {
EtsString *EscompatJSONStringifyObj(EtsObject *d)
{
    ASSERT(d != nullptr);
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] auto _ = HandleScope<ObjectHeader *>(thread);
    auto dHandle = VMHandle<EtsObject>(thread, d->GetCoreType());
    auto cls = dHandle.GetPtr()->GetClass();
    auto typeDesc = cls->GetDescriptor();

    std::string resString;
    if (cls->IsArrayClass()) {
        auto arr = reinterpret_cast<ark::ets::EtsArray *>(dHandle.GetPtr());
        resString = EtsArrayToJSON(reinterpret_cast<ark::ets::EtsArray *>(arr)).Build();
    } else if (cls->IsBoxedClass()) {
        std::stringstream ss;
        ss.setf(std::stringstream::boolalpha);
        if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
            ss << static_cast<bool>(ark::ets::EtsBoxPrimitive<ark::ets::EtsBoolean>::FromCoreType(d)->GetValue());
        } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_BYTE) {
            ss << ark::ets::EtsBoxPrimitive<ark::ets::EtsByte>::FromCoreType(d)->GetValue();
        } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_CHAR) {
            ss << ark::ets::EtsBoxPrimitive<ark::ets::EtsChar>::FromCoreType(d)->GetValue();
        } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_SHORT) {
            ss << ark::ets::EtsBoxPrimitive<ark::ets::EtsShort>::FromCoreType(d)->GetValue();
        } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_INT) {
            ss << ark::ets::EtsBoxPrimitive<ark::ets::EtsInt>::FromCoreType(d)->GetValue();
        } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_LONG) {
            ss << ark::ets::EtsBoxPrimitive<ark::ets::EtsLong>::FromCoreType(d)->GetValue();
        } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
            ss << ark::ets::EtsBoxPrimitive<ark::ets::EtsFloat>::FromCoreType(d)->GetValue();
        } else if (typeDesc == ark::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
            ss << ark::ets::EtsBoxPrimitive<ark::ets::EtsDouble>::FromCoreType(d)->GetValue();
        } else {
            UNREACHABLE();
        }
        resString = ss.str();
    } else {
        resString = ObjectToJSON(dHandle.GetPtr()).Build();
    }
    auto etsResString = EtsString::CreateFromUtf8(resString.data(), resString.size());
    return etsResString;
}

}  // namespace ark::ets::intrinsics
