/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_ETS_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
#define PANDA_ETS_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H

#include "bytecode_optimizer/runtime_adapter.h"

namespace ark {

class EtsBytecodeOptimizerRuntimeAdapter : public BytecodeOptimizerRuntimeAdapter {
public:
    explicit EtsBytecodeOptimizerRuntimeAdapter(const panda_file::File &pandaFile)
        : BytecodeOptimizerRuntimeAdapter(pandaFile)
    {
    }
    ~EtsBytecodeOptimizerRuntimeAdapter() override = default;
    NO_COPY_SEMANTIC(EtsBytecodeOptimizerRuntimeAdapter);
    NO_MOVE_SEMANTIC(EtsBytecodeOptimizerRuntimeAdapter);

    bool IsEtsConstructor(MethodPtr method) const
    {
        const auto methodName = GetMethodName(method);
        return (methodName == CTOR || methodName == CCTOR || methodName == DOT_CTOR || methodName == DOT_CCTOR);
    }

    std::string GetSignature(MethodPtr method) const
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        panda_file::ProtoDataAccessor pda(pandaFile_, mda.GetProtoId());

        std::ostringstream signature;
        signature << '(';

        auto methodId = GetMethodId(method);
        auto argCount = GetMethodArgumentsCount(method, methodId);
        auto returnType = pda.GetReturnType();

        for (size_t i = 0; i < argCount; i++) {
            auto argType = pda.GetArgType(i);
            if (argType.IsPrimitive()) {
                signature << panda_file::Type::GetSignatureByTypeId(argType);
            } else {
                size_t argOffset = returnType.IsPrimitive() ? 0 : 1;
                auto id = pda.GetReferenceType(i + argOffset);
                auto refType = pandaFile_.GetStringData(id);
                signature << reinterpret_cast<const char *>(refType.data);
            }
        }
        signature << ')';

        if (returnType.IsPrimitive()) {
            signature << panda_file::Type::GetSignatureByTypeId(returnType);
        } else {
            auto classId = pda.GetReferenceType(0);
            auto type = pandaFile_.GetStringData(classId);
            signature << reinterpret_cast<const char *>(type.data);
        }

        return signature.str();
    }

    bool IsMethodStringConcat(MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == METHODS_STRING_CONCAT &&
               GetSignature(method) == SIGNATURES_STRING_ARRAY_RET_STRING;
    }

    bool IsMethodStringGetLength(MethodPtr method) const override
    {
        auto methodName = GetMethodFullName(method, false);
        return (methodName == GETTERS_STRING_GET_LENGTH || methodName == METHODS_STRING_GET_LENGTH) &&
               GetSignature(method) == SIGNATURES_RET_INT;
    }

    bool IsMethodStringBuilderConstructorWithStringArg(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        return IsEtsConstructor(method) && GetClassNameFromMethod(method) == CLASSES_STRING_BUILDER &&
               GetSignature(method) == SIGNATURES_STRING_RET_VOID;
    }

    bool IsMethodStringBuilderConstructorWithCharArrayArg(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        return IsEtsConstructor(method) && GetClassNameFromMethod(method) == CLASSES_STRING_BUILDER &&
               GetSignature(method) == SIGNATURES_CHAR_ARRAY_RET_VOID;
    }

    bool IsMethodStringBuilderDefaultConstructor(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(pandaFile_, MethodCast(method));
        return IsEtsConstructor(method) && GetClassNameFromMethod(method) == CLASSES_STRING_BUILDER &&
               GetSignature(method) == SIGNATURES_RET_VOID;
    }

    bool IsMethodStringBuilderToString(MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == METHODS_STRING_BUILDER_TO_STRING &&
               GetSignature(method) == SIGNATURES_RET_STRING;
    }

    bool IsMethodStringBuilderAppend(MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == METHODS_STRING_BUILDER_APPEND;
    }

    bool IsClassStringBuilder([[maybe_unused]] ClassPtr klass) const override
    {
        return GetClassName(klass) == CLASSES_STRING_BUILDER;
    }

    bool IsMethodStringBuilderGetStringLength([[maybe_unused]] MethodPtr method) const override
    {
        return GetMethodFullName(method, false) == GETTERS_STRING_BUILDER_GET_STRING_LENGTH;
    }

    bool IsIntrinsicStringBuilderToString([[maybe_unused]] IntrinsicId id) const override
    {
        return id == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_TO_STRING;
    }

    bool IsIntrinsicStringConcat([[maybe_unused]] IntrinsicId id) const override
    {
        switch (id) {
            case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2:
                return true;
            case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3:
                return true;
            case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4:
                return true;
            default:
                return false;
        }
    }

    bool IsIntrinsicStringBuilderAppendString([[maybe_unused]] IntrinsicId id) const override
    {
        switch (id) {
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4:
                return true;
            default:
                return false;
        }
    }

    bool IsIntrinsicStringBuilderAppend([[maybe_unused]] IntrinsicId id) const override
    {
        switch (id) {
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_FLOAT:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_DOUBLE:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_LONG:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_INT:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BYTE:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_SHORT:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_CHAR:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BOOL:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3:
                return true;
            case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4:
                return true;
            default:
                return false;
        }
    }

    IntrinsicId GetStringConcatStringsIntrinsicId([[maybe_unused]] size_t numArgs) const override
    {
        // NOLINTBEGIN(readability-magic-numbers)
        switch (numArgs) {
            case 2U:
                return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2;
            case 3U:
                return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3;
            case 4U:
                return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4;
            default:
                UNREACHABLE();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    IntrinsicId GetStringBuilderAppendStringsIntrinsicId([[maybe_unused]] size_t numArgs) const override
    {
        // NOLINTBEGIN(readability-magic-numbers)
        switch (numArgs) {
            case 1U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING;
            case 2U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2;
            case 3U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3;
            case 4U:
                return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4;
            default:
                UNREACHABLE();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    IntrinsicId GetCompilerNullcheckIntrinsicId() const override
    {
        return IntrinsicId::INTRINSIC_COMPILER_ETS_NULLCHECK;
    }

    ClassPtr GetStringBuilderClass() const override
    {
        const uint8_t *mutf8Name = utf::CStringAsMutf8(CLASS_DESCRIPTORS_STRING_BUILDER.data());
        auto classId = pandaFile_.GetClassId(mutf8Name);
        return reinterpret_cast<ClassPtr>(classId.GetOffset());
    }

    FieldPtr GetFieldStringBuilderLength([[maybe_unused]] ClassPtr klass) const override
    {
        ASSERT(IsClassStringBuilder(klass));

        auto classEntityId = ClassCast(klass);
        auto fieldId = panda_file::File::EntityId();

        if (classEntityId.IsValid() && !pandaFile_.IsExternal(classEntityId)) {
            panda_file::ClassDataAccessor cda(pandaFile_, classEntityId);

            const uint8_t *lengthMutf8 = utf::CStringAsMutf8(FIELDS_LENGTH.data());
            panda_file::File::StringData lengthSD = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(lengthMutf8)),
                                                     lengthMutf8};

            cda.EnumerateFields([&fieldId, &cda, &lengthSD](panda_file::FieldDataAccessor &fieldDataAccessor) {
                // NOLINTNEXTLINE(readability-identifier-naming)
                auto fieldId_ = fieldDataAccessor.GetFieldId();
                auto &pf = cda.GetPandaFile();
                auto stringData = pf.GetStringData(fieldDataAccessor.GetNameId());
                if (lengthSD == stringData) {
                    fieldId = fieldId_;
                    return;
                }
            });

            return reinterpret_cast<FieldPtr>(fieldId.GetOffset());
        }

        return nullptr;
    }

    MethodPtr FindMethodByName([[maybe_unused]] const std::string &methodName) const override
    {
        auto regionHeaders = pandaFile_.GetRegionHeaders();
        for (auto regionHeader : regionHeaders) {
            auto methodIdx = pandaFile_.GetMethodIndex(&regionHeader);

            for (auto methodId : methodIdx) {
                auto methodPtr = reinterpret_cast<MethodPtr>(methodId.GetOffset());
                if (GetMethodFullName(reinterpret_cast<MethodPtr>(methodId.GetOffset()), false) == methodName) {
                    return methodPtr;
                }
            }
        }

        return nullptr;
    }

    MethodPtr FindMethodByName([[maybe_unused]] std::string const &methodName,
                               [[maybe_unused]] std::string const &methodSignature) const override
    {
        auto regionHeaders = pandaFile_.GetRegionHeaders();
        for (auto regionHeader : regionHeaders) {
            auto methodIdx = pandaFile_.GetMethodIndex(&regionHeader);

            for (auto methodId : methodIdx) {
                auto methodPtr = reinterpret_cast<MethodPtr>(methodId.GetOffset());
                if (GetMethodFullName(reinterpret_cast<MethodPtr>(methodId.GetOffset()), false) == methodName &&
                    GetSignature(methodPtr) == methodSignature) {
                    return methodPtr;
                }
            }
        }

        return nullptr;
    }

    MethodPtr GetGetterStringBuilderStringLength() const override
    {
        return FindMethodByName(std::string(GETTERS_STRING_BUILDER_GET_STRING_LENGTH));
    }

    MethodPtr GetMethodStringBuilderAppendString([[maybe_unused]] ClassPtr klass) const override
    {
        return FindMethodByName(std::string(METHODS_STRING_BUILDER_APPEND),
                                std::string(SIGNATURES_STRING_RET_STRING_BUILDER));
    }

private:
    static constexpr std::string_view CLASS_DESCRIPTORS_STRING_BUILDER = "Lstd/core/StringBuilder;";
    static constexpr std::string_view CLASSES_STRING_BUILDER = "std.core.StringBuilder";
    static constexpr std::string_view METHODS_STRING_CONCAT = "std.core.String::concat";
    static constexpr std::string_view METHODS_STRING_GET_LENGTH = "std.core.String::getLength";
    static constexpr std::string_view METHODS_STRING_BUILDER_APPEND = "std.core.StringBuilder::append";
    static constexpr std::string_view METHODS_STRING_BUILDER_TO_STRING = "std.core.StringBuilder::toString";
    static constexpr std::string_view GETTERS_STRING_GET_LENGTH = "std.core.String::%%get-length";
    static constexpr std::string_view GETTERS_STRING_BUILDER_GET_STRING_LENGTH =
        "std.core.StringBuilder::%%get-stringLength";
    static constexpr std::string_view SIGNATURES_RET_VOID = "()V";
    static constexpr std::string_view SIGNATURES_RET_INT = "()I";
    static constexpr std::string_view SIGNATURES_CHAR_ARRAY_RET_VOID = "([C)V";
    static constexpr std::string_view SIGNATURES_STRING_RET_VOID = "(Lstd/core/String;)V";
    static constexpr std::string_view SIGNATURES_STRING_ARRAY_RET_STRING = "([Lstd/core/String;)Lstd/core/String;";
    static constexpr std::string_view SIGNATURES_RET_STRING = "()Lstd/core/String;";
    static constexpr std::string_view SIGNATURES_STRING_RET_STRING_BUILDER =
        "(Lstd/core/String;)Lstd/core/StringBuilder;";
    static constexpr std::string_view FIELDS_LENGTH = "length";
    static constexpr std::string_view CTOR = "<ctor>";
    static constexpr std::string_view CCTOR = "<cctor>";
    static constexpr std::string_view DOT_CTOR = ".ctor";
    static constexpr std::string_view DOT_CCTOR = ".cctor";
};

}  // namespace ark

#endif  // PANDA_ETS_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
