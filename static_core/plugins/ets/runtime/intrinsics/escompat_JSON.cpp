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
panda::JsonObjectBuilder ObjectToJSON(panda::ets::EtsObject *d);

std::string EtsCharToString(ets_char data)
{
    constexpr auto HIGH_SURROGATE_MIN = 0xD800;
    constexpr auto LOW_SURROGATE_MAX = 0xDFFF;
    if ((data < HIGH_SURROGATE_MIN) || (data > LOW_SURROGATE_MAX)) {
        return std::string(reinterpret_cast<const char *>(&data), 1);
    }
    return std::string(reinterpret_cast<const char *>(&data), 2);
}

std::string_view EtsStringToView(panda::ets::EtsString *s)
{
    auto str = std::string_view();
    if (s->IsUtf16()) {
        str = std::string_view(reinterpret_cast<const char *>(s->GetDataUtf16()), s->GetUtf16Length() - 1);
    } else {
        str = std::string_view(reinterpret_cast<const char *>(s->GetDataMUtf8()), s->GetMUtf8Length() - 1);
    }
    return str;
}

template <typename PrimArr>
void EtsPrimitiveArrayToJSON(panda::JsonArrayBuilder &json_builder, PrimArr *arr_ptr)
{
    ASSERT(arr_ptr->GetClass()->IsArrayClass() && arr_ptr->IsPrimitive());
    auto len = arr_ptr->GetLength();
    for (size_t i = 0; i < len; ++i) {
        json_builder.Add(arr_ptr->Get(i));
    }
}

template <>
void EtsPrimitiveArrayToJSON(panda::JsonArrayBuilder &json_builder, panda::ets::EtsBooleanArray *arr_ptr)
{
    ASSERT(arr_ptr->IsPrimitive());
    auto len = arr_ptr->GetLength();
    for (size_t i = 0; i < len; ++i) {
        if (arr_ptr->Get(i) != 0) {
            json_builder.Add(true);
        } else {
            json_builder.Add(false);
        }
    }
}

template <>
void EtsPrimitiveArrayToJSON(panda::JsonArrayBuilder &json_builder, panda::ets::EtsCharArray *arr_ptr)
{
    ASSERT(arr_ptr->IsPrimitive());
    auto len = arr_ptr->GetLength();
    for (size_t i = 0; i < len; ++i) {
        auto data = arr_ptr->Get(i);
        json_builder.Add(EtsCharToString(data));
    }
}

panda::JsonArrayBuilder EtsArrayToJSON(panda::ets::EtsArray *arr_ptr)
{
    auto json_builder = panda::JsonArrayBuilder();
    if (arr_ptr->IsPrimitive()) {
        panda::panda_file::Type::TypeId component_type =
            arr_ptr->GetCoreType()->ClassAddr<panda::Class>()->GetComponentType()->GetType().GetId();
        switch (component_type) {
            case panda::panda_file::Type::TypeId::U1: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsBooleanArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::I8: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsByteArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::I16: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsShortArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::U16: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsCharArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::I32: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsIntArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::F32: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsFloatArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::F64: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsDoubleArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::I64: {
                EtsPrimitiveArrayToJSON(json_builder, reinterpret_cast<panda::ets::EtsLongArray *>(arr_ptr));
                break;
            }
            case panda::panda_file::Type::TypeId::U8:
            case panda::panda_file::Type::TypeId::U32:
            case panda::panda_file::Type::TypeId::U64:
            case panda::panda_file::Type::TypeId::REFERENCE:
            case panda::panda_file::Type::TypeId::TAGGED:
            case panda::panda_file::Type::TypeId::INVALID:
            case panda::panda_file::Type::TypeId::VOID:
                UNREACHABLE();
                break;
        }
    } else {
        auto arr_obj_ptr = reinterpret_cast<panda::ets::EtsObjectArray *>(arr_ptr);
        auto len = arr_obj_ptr->GetLength();
        for (size_t i = 0; i < len; ++i) {
            auto d = arr_obj_ptr->Get(i);
            auto d_cls = d->GetClass();
            auto type_desc = d_cls->GetDescriptor();
            if (d_cls->IsStringClass()) {
                auto s_ptr = reinterpret_cast<panda::ets::EtsString *>(d);
                json_builder.Add(EtsStringToView(s_ptr));
            } else if (d_cls->IsArrayClass()) {
                json_builder.Add([d](panda::JsonArrayBuilder &x) {
                    x = EtsArrayToJSON(reinterpret_cast<panda::ets::EtsArray *>(d));
                });
            } else if (d_cls->IsBoxedClass()) {
                if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsBoolean>::FromCoreType(d)->GetValue());
                } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BYTE) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsByte>::FromCoreType(d)->GetValue());
                } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_CHAR) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsChar>::FromCoreType(d)->GetValue());
                } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_SHORT) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsShort>::FromCoreType(d)->GetValue());
                } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_INT) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsInt>::FromCoreType(d)->GetValue());
                } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_LONG) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsLong>::FromCoreType(d)->GetValue());
                } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsFloat>::FromCoreType(d)->GetValue());
                } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
                    json_builder.Add(panda::ets::EtsBoxPrimitive<panda::ets::EtsDouble>::FromCoreType(d)->GetValue());
                } else {
                    UNREACHABLE();
                }
            } else {
                json_builder.Add([d](panda::JsonObjectBuilder &x) { x = ObjectToJSON(d); });
            }
        }
    }
    return json_builder;
}

void AddFieldsToJSON(panda::JsonObjectBuilder &cur_json, const panda::Span<panda::Field> &fields,
                     panda::ets::EtsObject *d)
{
    for (const auto &f : fields) {
        ASSERT(f.IsStatic() == false);
        auto field_name = reinterpret_cast<const char *>(f.GetName().data);

        switch (f.GetTypeId()) {
            case panda::panda_file::Type::TypeId::U1:
                cur_json.AddProperty(field_name, d->GetFieldPrimitive<bool>(f.GetOffset()));
                break;
            case panda::panda_file::Type::TypeId::I8:
                cur_json.AddProperty(field_name, d->GetFieldPrimitive<int8_t>(f.GetOffset()));
                break;
            case panda::panda_file::Type::TypeId::I16:
                cur_json.AddProperty(field_name, d->GetFieldPrimitive<int16_t>(f.GetOffset()));
                break;
            case panda::panda_file::Type::TypeId::U16:
                cur_json.AddProperty(field_name, EtsCharToString(d->GetFieldPrimitive<ets_char>(f.GetOffset())));
                break;
            case panda::panda_file::Type::TypeId::I32:
                cur_json.AddProperty(field_name, d->GetFieldPrimitive<int32_t>(f.GetOffset()));
                break;
            case panda::panda_file::Type::TypeId::F32:
                cur_json.AddProperty(field_name, d->GetFieldPrimitive<float>(f.GetOffset()));
                break;
            case panda::panda_file::Type::TypeId::F64:
                cur_json.AddProperty(field_name, d->GetFieldPrimitive<double>(f.GetOffset()));
                break;
            case panda::panda_file::Type::TypeId::I64:
                cur_json.AddProperty(field_name, d->GetFieldPrimitive<int64_t>(f.GetOffset()));
                break;
            case panda::panda_file::Type::TypeId::REFERENCE: {
                auto *f_ptr = d->GetFieldObject(f.GetOffset());
                if (f_ptr != nullptr) {
                    auto f_cls = f_ptr->GetClass();
                    auto type_desc = f_cls->GetDescriptor();
                    if (f_cls->IsStringClass()) {
                        auto s_ptr = reinterpret_cast<panda::ets::EtsString *>(f_ptr);
                        cur_json.AddProperty(field_name, EtsStringToView(s_ptr));
                    } else if (f_cls->IsArrayClass()) {
                        auto a_ptr = reinterpret_cast<panda::ets::EtsArray *>(f_ptr);
                        cur_json.AddProperty(field_name,
                                             [a_ptr](panda::JsonArrayBuilder &x) { x = EtsArrayToJSON(a_ptr); });
                    } else if (f_cls->IsBoxedClass()) {
                        if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
                            cur_json.AddProperty(
                                field_name, static_cast<bool>(
                                                panda::ets::EtsBoxPrimitive<panda::ets::EtsBoolean>::FromCoreType(f_ptr)
                                                    ->GetValue()));
                        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BYTE) {
                            cur_json.AddProperty(
                                field_name,
                                panda::ets::EtsBoxPrimitive<panda::ets::EtsByte>::FromCoreType(f_ptr)->GetValue());
                        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_CHAR) {
                            cur_json.AddProperty(
                                field_name,
                                panda::ets::EtsBoxPrimitive<panda::ets::EtsChar>::FromCoreType(f_ptr)->GetValue());
                        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_SHORT) {
                            cur_json.AddProperty(
                                field_name,
                                panda::ets::EtsBoxPrimitive<panda::ets::EtsShort>::FromCoreType(f_ptr)->GetValue());
                        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_INT) {
                            cur_json.AddProperty(
                                field_name,
                                panda::ets::EtsBoxPrimitive<panda::ets::EtsInt>::FromCoreType(f_ptr)->GetValue());
                        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_LONG) {
                            cur_json.AddProperty(
                                field_name,
                                panda::ets::EtsBoxPrimitive<panda::ets::EtsLong>::FromCoreType(f_ptr)->GetValue());
                        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
                            cur_json.AddProperty(
                                field_name,
                                panda::ets::EtsBoxPrimitive<panda::ets::EtsFloat>::FromCoreType(f_ptr)->GetValue());
                        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
                            cur_json.AddProperty(
                                field_name,
                                panda::ets::EtsBoxPrimitive<panda::ets::EtsDouble>::FromCoreType(f_ptr)->GetValue());
                        } else {
                            UNREACHABLE();
                        }
                    } else {
                        cur_json.AddProperty(field_name,
                                             [f_ptr](panda::JsonObjectBuilder &x) { x = ObjectToJSON(f_ptr); });
                    }
                } else {
                    cur_json.AddProperty(field_name, std::nullptr_t());
                }
                break;
            }
            case panda::panda_file::Type::TypeId::U8:
            case panda::panda_file::Type::TypeId::U32:
            case panda::panda_file::Type::TypeId::U64:
            case panda::panda_file::Type::TypeId::INVALID:
            case panda::panda_file::Type::TypeId::VOID:
            case panda::panda_file::Type::TypeId::TAGGED:
                UNREACHABLE();
                break;
        }
    }
}

panda::JsonObjectBuilder ObjectToJSON(panda::ets::EtsObject *d)
{
    ASSERT(d != nullptr);
    panda::ets::EtsClass *kls = d->GetClass();
    ASSERT(kls != nullptr);

    // Only instance fields are required according to JS/TS JSON.stringify behaviour
    auto cur_json = panda::JsonObjectBuilder();
    kls->EnumerateBaseClasses([&](panda::ets::EtsClass *c) {
        AddFieldsToJSON(cur_json, c->GetRuntimeClass()->GetInstanceFields(), d);
        return false;
    });
    return cur_json;
}
}  // namespace

namespace panda::ets::intrinsics {
EtsString *EscompatJSONStringifyObj(EtsObject *d)
{
    ASSERT(d != nullptr);
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] auto _ = HandleScope<ObjectHeader *>(thread);
    auto d_handle = VMHandle<EtsObject>(thread, d->GetCoreType());
    auto cls = d_handle.GetPtr()->GetClass();
    auto type_desc = cls->GetDescriptor();

    auto res_string = std::string();
    if (cls->IsArrayClass()) {
        auto arr = reinterpret_cast<panda::ets::EtsArray *>(d_handle.GetPtr());
        res_string = EtsArrayToJSON(reinterpret_cast<panda::ets::EtsArray *>(arr)).Build();
    } else if (cls->IsBoxedClass()) {
        std::stringstream ss;
        ss.setf(std::stringstream::boolalpha);
        if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BOOLEAN) {
            ss << static_cast<bool>(panda::ets::EtsBoxPrimitive<panda::ets::EtsBoolean>::FromCoreType(d)->GetValue());
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BYTE) {
            ss << panda::ets::EtsBoxPrimitive<panda::ets::EtsByte>::FromCoreType(d)->GetValue();
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_CHAR) {
            ss << panda::ets::EtsBoxPrimitive<panda::ets::EtsChar>::FromCoreType(d)->GetValue();
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_SHORT) {
            ss << panda::ets::EtsBoxPrimitive<panda::ets::EtsShort>::FromCoreType(d)->GetValue();
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_INT) {
            ss << panda::ets::EtsBoxPrimitive<panda::ets::EtsInt>::FromCoreType(d)->GetValue();
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_LONG) {
            ss << panda::ets::EtsBoxPrimitive<panda::ets::EtsLong>::FromCoreType(d)->GetValue();
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_FLOAT) {
            ss << panda::ets::EtsBoxPrimitive<panda::ets::EtsFloat>::FromCoreType(d)->GetValue();
        } else if (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_DOUBLE) {
            ss << panda::ets::EtsBoxPrimitive<panda::ets::EtsDouble>::FromCoreType(d)->GetValue();
        } else {
            UNREACHABLE();
        }
        res_string = ss.str();
    } else {
        res_string = ObjectToJSON(d_handle.GetPtr()).Build();
    }
    auto ets_res_string = EtsString::CreateFromUtf8(res_string.data(), res_string.size());
    return ets_res_string;
}

}  // namespace panda::ets::intrinsics
