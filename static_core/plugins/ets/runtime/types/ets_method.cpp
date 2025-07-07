/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_method.h"

#include "libpandabase/macros.h"
#include "libpandabase/utils/utf.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_type.h"

namespace ark::ets {

class EtsObject;

bool EtsMethod::IsMethod(const PandaString &td)
{
    return td[0] == METHOD_PREFIX;
}

static EtsMethod *FindInvokeMethodInFunctionalType(EtsClass *type)
{
    ASSERT(type->IsFunction());
    for (size_t arity = 0; arity <= STD_CORE_FUNCTION_MAX_ARITY; ++arity) {
        PandaStringStream ss;
        ss << STD_CORE_FUNCTION_INVOKE_PREFIX << arity;
        PandaString str = ss.str();
        EtsMethod *method = type->GetInstanceMethod(str.c_str(), nullptr);
        if (method != nullptr) {
            return method;
        }
    }
    UNREACHABLE();
}

EtsMethod *EtsMethod::FromTypeDescriptor(const PandaString &td, EtsRuntimeLinker *contextLinker)
{
    ASSERT(contextLinker != nullptr);
    auto *ctx = contextLinker->GetClassLinkerContext();

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
        auto *method = classLinker->GetMethod(*pandaFile, panda_file::File::EntityId(id), ctx);
        return EtsMethod::FromRuntimeMethod(method);
    }
    ASSERT(td[0] == CLASS_TYPE_PREFIX);
    auto type = classLinker->GetClass(td.c_str(), true, ctx);
    ASSERT(type != nullptr);
    EtsMethod *method = type->GetInstanceMethod(ark::ets::INVOKE_METHOD_NAME, nullptr);
    method = method == nullptr ? type->GetStaticMethod(ark::ets::INVOKE_METHOD_NAME, nullptr) : method;
    if (method != nullptr) {
        return method;
    }
    return FindInvokeMethodInFunctionalType(type);
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

ani_status EtsMethod::Invoke(ani::ScopedManagedCodeFix &s, Value *args, EtsValue *result)
{
    Value res = GetPandaMethod()->Invoke(s.GetCoroutine(), args);
    ANI_CHECK_RETURN_IF_EQ(s.HasPendingException(), true, ANI_PENDING_ERROR);
    if (GetReturnValueType() == EtsType::VOID) {
        // Return any value, will be ignored
        *result = EtsValue(0);
        return ANI_OK;
    }
    if (GetReturnValueType() == EtsType::OBJECT) {
        auto *obj = reinterpret_cast<EtsObject *>(res.GetAs<ObjectHeader *>());
        return s.AddLocalRef(obj, reinterpret_cast<ani_ref *>(result));
    }
    *result = EtsValue(res.GetAs<EtsLong>());
    return ANI_OK;
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

inline std::optional<uint32_t> TryGetMinArgCountFromAnnotation(const panda_file::AnnotationDataAccessor &accessor,
                                                               const panda_file::File &pandaFile)
{
    for (uint32_t i = 0; i < accessor.GetCount(); i++) {
        panda_file::AnnotationDataAccessor::Elem accessorElem = accessor.GetElement(i);
        if (utf::IsEqual(pandaFile.GetStringData(accessorElem.GetNameId()).data, utf::CStringAsMutf8("minArgCount"))) {
            // annotation value is int in ets code which must be positive, so it is be casted to uint32_t safely
            auto minArgCount = accessorElem.GetScalarValue().Get<int32_t>();
            ASSERT(minArgCount >= 0);
            return static_cast<uint32_t>(minArgCount);
        }
    }
    return std::nullopt;
}

std::optional<uint32_t> EtsMethod::TryGetMinArgCount()
{
    std::optional<uint32_t> resOpt = std::nullopt;

    panda_file::MethodDataAccessor mda(*(GetPandaMethod()->GetPandaFile()), GetPandaMethod()->GetFileId());
    mda.EnumerateAnnotations([&](panda_file::File::EntityId annotationId) {
        panda_file::AnnotationDataAccessor accessor(mda.GetPandaFile(), annotationId);
        auto annotationName = mda.GetPandaFile().GetStringData(accessor.GetClassId()).data;
        auto expectedAnnotationName =
            utf::CStringAsMutf8(panda_file_items::class_descriptors::OPTIONAL_PARAMETERS_ANNOTATION.data());
        // check if annotation is optional parameters
        if (utf::IsEqual(annotationName, expectedAnnotationName)) {
            std::optional<uint32_t> minArgCountOpt = TryGetMinArgCountFromAnnotation(accessor, mda.GetPandaFile());

            if (minArgCountOpt.has_value()) {
                resOpt = minArgCountOpt;
            }
        }
    });
    return resOpt;
}

uint32_t EtsMethod::GetNumMandatoryArgs()
{
    // NOTE(MockMockBlack, #IC787J): support default parameters and optional parameters for lambda functon
    size_t numMandatoryArgs = 0;

    bool hasRestParam = ((this->GetAccessFlags() & ACC_VARARGS) != 0);
    if (hasRestParam) {
        // rest param is not mandatory
        numMandatoryArgs = this->GetParametersNum() - 1;
    } else {
        numMandatoryArgs = this->GetParametersNum();
    }

    auto minArgCountOpt = this->TryGetMinArgCount();
    if (minArgCountOpt.has_value()) {
        numMandatoryArgs = minArgCountOpt.value();
    }

    return numMandatoryArgs;
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
