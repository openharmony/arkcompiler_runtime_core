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

#include "macros.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/runtime.h"
#include "runtime/include/value-inl.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "types/ets_type.h"

namespace panda::ets {

class EtsObject;

bool EtsMethod::IsMethod(const PandaString &td)
{
    return td[0] == METHOD_PREFIX;
}

EtsMethod *EtsMethod::FromTypeDescriptor(const PandaString &td)
{
    EtsClassLinker *class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    if (td[0] == METHOD_PREFIX) {
        // here we resolve method in existing class, which is stored as pointer to panda file + entity id
        uint64_t file_ptr;
        uint64_t id;
        const auto scanf_str = std::string_view {td}.substr(1).data();
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,cert-err34-c)
        [[maybe_unused]] auto res = sscanf_s(scanf_str, "%" PRIu64 ";%" PRIu64 ";", &file_ptr, &id);
        // NOLINTEND(cppcoreguidelines-pro-type-vararg,cert-err34-c)
        [[maybe_unused]] static constexpr int SCANF_PARAM_CNT = 2;
        ASSERT(res == SCANF_PARAM_CNT);
        auto panda_file = reinterpret_cast<const panda_file::File *>(file_ptr);
        return EtsMethod::FromRuntimeMethod(class_linker->GetMethod(*panda_file, panda_file::File::EntityId(id)));
    }
    ASSERT(td[0] == CLASS_TYPE_PREFIX);
    auto type = class_linker->GetClass(td.c_str());
    return type->GetMethod(LAMBDA_METHOD_NAME);
}

EtsValue EtsMethod::Invoke(napi::ScopedManagedCodeFix *s, Value *args)
{
    Value res = GetPandaMethod()->Invoke(s->GetEtsCoroutine(), args);
    if (GetReturnValueType() == EtsType::VOID) {
        // Return any value, will be ignored
        return EtsValue(0);
    }
    if (GetReturnValueType() == EtsType::OBJECT) {
        auto *obj = reinterpret_cast<EtsObject *>(res.GetAs<ObjectHeader *>());
        if (obj == nullptr) {
            return EtsValue(nullptr);
        }
        return EtsValue(napi::ScopedManagedCodeFix::AddLocalRef(s->EtsNapiEnv(), obj));
    }

    return EtsValue(res.GetAs<EtsLong>());
}

uint32_t EtsMethod::GetNumArgSlots() const
{
    uint32_t num_of_slots = 0;
    auto proto = GetPandaMethod()->GetProto();
    auto &shorty = proto.GetShorty();
    auto shorty_end = shorty.end();
    // must skip the return type
    auto shorty_it = shorty.begin() + 1;
    for (; shorty_it != shorty_end; ++shorty_it) {
        auto arg_type_id = shorty_it->GetId();
        // double and long arguments take two slots
        if (arg_type_id == panda_file::Type::TypeId::I64 || arg_type_id == panda_file::Type::TypeId::F64) {
            num_of_slots += 2;
        } else {
            num_of_slots += 1;
        }
    }
    if (!IsStatic()) {
        ++num_of_slots;
    }
    return num_of_slots;
}

EtsClass *EtsMethod::ResolveArgType(uint32_t idx)
{
    if (!IsStatic()) {
        if (idx == 0) {
            return GetClass();
        }
    }

    // get reference type
    EtsClassLinker *class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = GetPandaMethod()->GetArgType(idx);
    if (!type.IsPrimitive()) {
        size_t ref_idx = 0;
        size_t short_end = IsStatic() ? (idx + 1) : idx;  // first is return type
        auto proto = GetPandaMethod()->GetProto();
        for (size_t short_idx = 0; short_idx < short_end; short_idx++) {
            if (proto.GetShorty()[short_idx].IsReference()) {
                ref_idx++;
            }
        }
        ASSERT(ref_idx <= proto.GetRefTypes().size());
        return class_linker->GetClass(proto.GetRefTypes()[ref_idx].data(), false, GetClass()->GetClassLoader());
    }

    // get primitive type
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:
            return class_linker->GetClassRoot(EtsClassRoot::BOOLEAN);
        case panda_file::Type::TypeId::I8:
            return class_linker->GetClassRoot(EtsClassRoot::BYTE);
        case panda_file::Type::TypeId::I16:
            return class_linker->GetClassRoot(EtsClassRoot::SHORT);
        case panda_file::Type::TypeId::U16:
            return class_linker->GetClassRoot(EtsClassRoot::CHAR);
        case panda_file::Type::TypeId::I32:
            return class_linker->GetClassRoot(EtsClassRoot::INT);
        case panda_file::Type::TypeId::I64:
            return class_linker->GetClassRoot(EtsClassRoot::LONG);
        case panda_file::Type::TypeId::F32:
            return class_linker->GetClassRoot(EtsClassRoot::FLOAT);
        case panda_file::Type::TypeId::F64:
            return class_linker->GetClassRoot(EtsClassRoot::DOUBLE);
        default:
            LOG(FATAL, RUNTIME) << "ResolveArgType: not a valid ets type for " << type;
            return nullptr;
    };
}

PandaString EtsMethod::GetMethodSignature(bool include_return_type) const
{
    PandaOStringStream signature;
    auto proto = GetPandaMethod()->GetProto();
    auto &shorty = proto.GetShorty();
    auto &ref_types = proto.GetRefTypes();

    auto ref_it = ref_types.begin();
    panda_file::Type return_type = shorty[0];
    if (!return_type.IsPrimitive()) {
        ++ref_it;
    }

    auto shorty_end = shorty.end();
    auto shorty_it = shorty.begin() + 1;
    for (; shorty_it != shorty_end; ++shorty_it) {
        if ((*shorty_it).IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(*shorty_it);
        } else {
            signature << *ref_it;
            ++ref_it;
        }
    }

    if (include_return_type) {
        signature << ":";
        if (return_type.IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(return_type);
        } else {
            signature << ref_types[0];
        }
    }
    return signature.str();
}

PandaString EtsMethod::GetDescriptor() const
{
    constexpr size_t TD_MAX_SIZE = 256;
    std::array<char, TD_MAX_SIZE> actual_td;  // NOLINT(cppcoreguidelines-pro-type-member-init)
    // initialize in printf
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    snprintf_s(actual_td.data(), actual_td.size(), actual_td.size() - 1, "%c%" PRIu64 ";%" PRIu64 ";", METHOD_PREFIX,
               reinterpret_cast<uint64_t>(GetPandaMethod()->GetPandaFile()),
               static_cast<uint64_t>(GetPandaMethod()->GetFileId().GetOffset()));
    return {actual_td.data()};
}

}  // namespace panda::ets
