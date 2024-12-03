/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::ets {

class EtsObject;

bool EtsMethod::IsMethod(const PandaString &td)
{
    return td[0] == METHOD_PREFIX;
}

EtsMethod *EtsMethod::FromTypeDescriptor(const PandaString &td)
{
    EtsClassLinker *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    if (td[0] == METHOD_PREFIX) {
        // here we resolve method in existing class, which is stored as pointer to panda file + entity id
        uint64_t filePtr;
        uint64_t id;
        const auto scanfStr = std::string_view {td}.substr(1).data();
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,cert-err34-c)
        [[maybe_unused]] auto res = sscanf_s(scanfStr, "%" PRIu64 ";%" PRIu64 ";", &filePtr, &id);
        // NOLINTEND(cppcoreguidelines-pro-type-vararg,cert-err34-c)
        [[maybe_unused]] static constexpr int SCANF_PARAM_CNT = 2;
        ASSERT(res == SCANF_PARAM_CNT);
        auto pandaFile = reinterpret_cast<const panda_file::File *>(filePtr);
        return EtsMethod::FromRuntimeMethod(classLinker->GetMethod(*pandaFile, panda_file::File::EntityId(id)));
    }
    ASSERT(td[0] == CLASS_TYPE_PREFIX);
    auto type = classLinker->GetClass(td.c_str());

    auto method = type->GetMethod(LAMBDA_METHOD_NAME);
    if (method != nullptr) {
        return method;
    }

    return type->GetMethod(FN_INVOKE_METHOD_NAME);
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
    uint32_t numOfSlots = 0;
    auto proto = GetPandaMethod()->GetProto();
    auto &shorty = proto.GetShorty();
    auto shortyEnd = shorty.end();
    // must skip the return type
    auto shortyIt = shorty.begin() + 1;
    for (; shortyIt != shortyEnd; ++shortyIt) {
        auto argTypeId = shortyIt->GetId();
        // double and long arguments take two slots
        if (argTypeId == panda_file::Type::TypeId::I64 || argTypeId == panda_file::Type::TypeId::F64) {
            numOfSlots += 2U;
        } else {
            numOfSlots += 1U;
        }
    }
    if (!IsStatic()) {
        ++numOfSlots;
    }
    return numOfSlots;
}

EtsClass *EtsMethod::ResolveArgType(uint32_t idx)
{
    if (!IsStatic()) {
        if (idx == 0) {
            return GetClass();
        }
    }

    // get reference type
    EtsClassLinker *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto type = GetPandaMethod()->GetArgType(idx);
    if (!type.IsPrimitive()) {
        size_t refIdx = 0;
        size_t shortEnd = IsStatic() ? (idx + 1) : idx;  // first is return type
        auto proto = GetPandaMethod()->GetProto();
        for (size_t shortIdx = 0; shortIdx < shortEnd; shortIdx++) {
            if (proto.GetShorty()[shortIdx].IsReference()) {
                refIdx++;
            }
        }
        ASSERT(refIdx <= proto.GetRefTypes().size());
        return classLinker->GetClass(proto.GetRefTypes()[refIdx].data(), false, GetClass()->GetLoadContext());
    }

    // get primitive type
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:
            return classLinker->GetClassRoot(EtsClassRoot::BOOLEAN);
        case panda_file::Type::TypeId::I8:
            return classLinker->GetClassRoot(EtsClassRoot::BYTE);
        case panda_file::Type::TypeId::I16:
            return classLinker->GetClassRoot(EtsClassRoot::SHORT);
        case panda_file::Type::TypeId::U16:
            return classLinker->GetClassRoot(EtsClassRoot::CHAR);
        case panda_file::Type::TypeId::I32:
            return classLinker->GetClassRoot(EtsClassRoot::INT);
        case panda_file::Type::TypeId::I64:
            return classLinker->GetClassRoot(EtsClassRoot::LONG);
        case panda_file::Type::TypeId::F32:
            return classLinker->GetClassRoot(EtsClassRoot::FLOAT);
        case panda_file::Type::TypeId::F64:
            return classLinker->GetClassRoot(EtsClassRoot::DOUBLE);
        default:
            LOG(FATAL, RUNTIME) << "ResolveArgType: not a valid ets type for " << type;
            return nullptr;
    };
}

PandaString EtsMethod::GetMethodSignature(bool includeReturnType) const
{
    PandaOStringStream signature;
    auto proto = GetPandaMethod()->GetProto();
    auto &shorty = proto.GetShorty();
    auto &refTypes = proto.GetRefTypes();

    auto refIt = refTypes.begin();
    panda_file::Type returnType = shorty[0];
    if (!returnType.IsPrimitive()) {
        ++refIt;
    }

    auto shortyEnd = shorty.end();
    auto shortyIt = shorty.begin() + 1;
    for (; shortyIt != shortyEnd; ++shortyIt) {
        if ((*shortyIt).IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(*shortyIt);
        } else {
            signature << *refIt;
            ++refIt;
        }
    }

    if (includeReturnType) {
        signature << ":";
        if (returnType.IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(returnType);
        } else {
            signature << refTypes[0];
        }
    }
    return signature.str();
}

PandaString EtsMethod::GetDescriptor() const
{
    constexpr size_t TD_MAX_SIZE = 256;
    std::array<char, TD_MAX_SIZE> actualTd;  // NOLINT(cppcoreguidelines-pro-type-member-init)
    // initialize in printf
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    snprintf_s(actualTd.data(), actualTd.size(), actualTd.size() - 1, "%c%" PRIu64 ";%" PRIu64 ";", METHOD_PREFIX,
               reinterpret_cast<uint64_t>(GetPandaMethod()->GetPandaFile()),
               static_cast<uint64_t>(GetPandaMethod()->GetFileId().GetOffset()));
    return {actualTd.data()};
}

}  // namespace ark::ets
