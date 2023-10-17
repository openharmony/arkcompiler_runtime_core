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
#ifndef PANDA_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H_
#define PANDA_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H_

#include "compiler/optimizer/ir/runtime_interface.h"
#include "libpandafile/bytecode_instruction.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/code_data_accessor.h"
#include "libpandafile/field_data_accessor.h"
#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "libpandafile/method_data_accessor.h"
#include "libpandafile/proto_data_accessor.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandafile/type_helper.h"

namespace panda {
using compiler::RuntimeInterface;

class BytecodeOptimizerRuntimeAdapter : public RuntimeInterface {
public:
    explicit BytecodeOptimizerRuntimeAdapter(const panda_file::File &panda_file) : panda_file_(panda_file) {}
    ~BytecodeOptimizerRuntimeAdapter() override = default;
    NO_COPY_SEMANTIC(BytecodeOptimizerRuntimeAdapter);
    NO_MOVE_SEMANTIC(BytecodeOptimizerRuntimeAdapter);

    BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const override
    {
        return const_cast<panda_file::File *>(&panda_file_);
    }

    MethodId ResolveMethodIndex(MethodPtr parent_method, MethodIndex index) const override
    {
        return panda_file_.ResolveMethodIndex(MethodCast(parent_method), index).GetOffset();
    }

    FieldId ResolveFieldIndex(MethodPtr parent_method, FieldIndex index) const override
    {
        return panda_file_.ResolveFieldIndex(MethodCast(parent_method), index).GetOffset();
    }

    IdType ResolveTypeIndex(MethodPtr parent_method, TypeIndex index) const override
    {
        return panda_file_.ResolveClassIndex(MethodCast(parent_method), index).GetOffset();
    }

    MethodPtr GetMethodById([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        return reinterpret_cast<MethodPtr>(id);
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return static_cast<MethodId>(reinterpret_cast<uintptr_t>(method));
    }

    compiler::DataType::Type GetMethodReturnType(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));
        panda_file::ProtoDataAccessor pda(panda_file_, mda.GetProtoId());

        return ToCompilerType(panda_file::GetEffectiveType(pda.GetReturnType()));
    }

    IdType GetMethodReturnTypeId(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));
        panda_file::ProtoDataAccessor pda(panda_file_, mda.GetProtoId());

        return pda.GetReferenceType(0).GetOffset();
    }

    compiler::DataType::Type GetMethodTotalArgumentType(MethodPtr method, size_t index) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        if (!mda.IsStatic()) {
            if (index == 0) {
                return ToCompilerType(
                    panda_file::GetEffectiveType(panda_file::Type(panda_file::Type::TypeId::REFERENCE)));
            }
            --index;
        }

        panda_file::ProtoDataAccessor pda(panda_file_, mda.GetProtoId());
        return ToCompilerType(panda_file::GetEffectiveType(pda.GetArgType(index)));
    }

    compiler::DataType::Type GetMethodArgumentType([[maybe_unused]] MethodPtr caller, MethodId id,
                                                   size_t index) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, panda_file::File::EntityId(id));
        panda_file::ProtoDataAccessor pda(panda_file_, mda.GetProtoId());

        return ToCompilerType(panda_file::GetEffectiveType(pda.GetArgType(index)));
    }

    size_t GetMethodTotalArgumentsCount(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetNumArgs();
    }

    size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, panda_file::File::EntityId(id));
        panda_file::ProtoDataAccessor pda(panda_file_, mda.GetProtoId());

        return pda.GetNumArgs();
    }

    compiler::DataType::Type GetMethodReturnType(MethodPtr caller, MethodId id) const override
    {
        return GetMethodReturnType(GetMethodById(caller, id));
    }

    size_t GetMethodRegistersCount(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetNumVregs();
    }

    const uint8_t *GetMethodCode(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetInstructions();
    }

    size_t GetMethodCodeSize(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetCodeSize();
    }

    SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative());

        auto source_lang = mda.GetSourceLang();
        ASSERT(source_lang.has_value());

        return static_cast<SourceLanguage>(source_lang.value());
    }

    size_t GetClassIdForField([[maybe_unused]] MethodPtr method, size_t field_id) const override
    {
        panda_file::FieldDataAccessor fda(panda_file_, panda_file::File::EntityId(field_id));

        return static_cast<size_t>(fda.GetClassId().GetOffset());
    }

    ClassPtr GetClassForField(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(panda_file_, FieldCast(field));

        return reinterpret_cast<ClassPtr>(fda.GetClassId().GetOffset());
    }

    size_t GetClassIdForMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        return static_cast<size_t>(mda.GetClassId().GetOffset());
    }

    size_t GetClassIdForMethod([[maybe_unused]] MethodPtr caller, size_t method_id) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, panda_file::File::EntityId(method_id));

        return static_cast<size_t>(mda.GetClassId().GetOffset());
    }

    bool IsMethodExternal([[maybe_unused]] MethodPtr caller, MethodPtr callee) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(callee));

        return mda.IsExternal();
    }

    bool IsMethodIntrinsic([[maybe_unused]] MethodPtr method) const override
    {
        return false;
    }

    bool IsMethodIntrinsic([[maybe_unused]] MethodPtr caller, [[maybe_unused]] MethodId id) const override
    {
        return false;
    }

    bool IsMethodStatic(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        return mda.IsStatic();
    }

    bool IsMethodStatic([[maybe_unused]] MethodPtr caller, MethodId id) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, panda_file::File::EntityId(id));

        return mda.IsStatic();
    }

    // return true if the method is Native with exception
    bool HasNativeException([[maybe_unused]] MethodPtr method) const override
    {
        return false;
    }

    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        auto string_data = panda_file_.GetStringData(mda.GetClassId());

        return std::string(reinterpret_cast<const char *>(string_data.data));
    }

    std::string GetClassName(ClassPtr cls) const override
    {
        auto string_data = panda_file_.GetStringData(ClassCast(cls));

        return std::string(reinterpret_cast<const char *>(string_data.data));
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        auto string_data = panda_file_.GetStringData(mda.GetNameId());

        return std::string(reinterpret_cast<const char *>(string_data.data));
    }

    bool IsConstructor(MethodPtr method, SourceLanguage lang) override
    {
        return GetMethodName(method) == panda_file::GetCtorName(lang);
    }

    std::string GetMethodFullName(MethodPtr method, bool /* with_signature */) const override
    {
        auto class_name = GetClassNameFromMethod(method);
        auto method_name = GetMethodName(method);

        return class_name + "::" + method_name;
    }

    ClassPtr GetClass(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        return reinterpret_cast<ClassPtr>(mda.GetClassId().GetOffset());
    }

    std::string GetBytecodeString(MethodPtr method, uintptr_t pc) const override
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        BytecodeInstruction inst(GetMethodCode(method) + pc);
        std::stringstream ss;

        ss << inst;
        return ss.str();
    }

    bool IsArrayClass([[maybe_unused]] MethodPtr method, IdType id) const override
    {
        panda_file::File::EntityId cid(id);

        return panda_file::IsArrayDescriptor(panda_file_.GetStringData(cid).data);
    }

    FieldPtr ResolveField([[maybe_unused]] MethodPtr method, size_t id, [[maybe_unused]] bool allow_external,
                          uint32_t * /* class_id */) override
    {
        return reinterpret_cast<FieldPtr>(id);
    }

    compiler::DataType::Type GetFieldType(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(panda_file_, FieldCast(field));

        return ToCompilerType(panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
    }

    compiler::DataType::Type GetFieldTypeById([[maybe_unused]] MethodPtr parent_method, IdType id) const override
    {
        panda_file::FieldDataAccessor fda(panda_file_, panda_file::File::EntityId(id));

        return ToCompilerType(panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
    }

    IdType GetFieldValueTypeId([[maybe_unused]] MethodPtr method, IdType id) const override
    {
        auto type_id = panda_file::FieldDataAccessor::GetTypeId(panda_file_, panda_file::File::EntityId(id));
        return type_id.GetOffset();
    }

    bool IsFieldVolatile(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(panda_file_, FieldCast(field));

        if (!fda.IsExternal()) {
            return fda.IsVolatile();
        }

        auto field_id = panda_file::File::EntityId();

        if (panda_file_.IsExternal(fda.GetClassId())) {
            // If the field is external and class of the field is also external
            // assume that field is volatile
            return true;
        }

        auto class_id = panda_file::ClassDataAccessor(panda_file_, fda.GetClassId()).GetSuperClassId();
#ifndef NDEBUG
        auto visited_classes = std::unordered_set<panda_file::File::EntityId> {class_id};
#endif
        while (class_id.IsValid() && !panda_file_.IsExternal(class_id)) {
            auto cda = panda_file::ClassDataAccessor(panda_file_, class_id);
            cda.EnumerateFields([&field_id, &fda](panda_file::FieldDataAccessor &field_data_accessor) {
                auto &pf = fda.GetPandaFile();
                auto field_type = panda_file::Type::GetTypeFromFieldEncoding(fda.GetType());
                if (fda.GetType() != field_data_accessor.GetType()) {
                    return;
                }

                if (pf.GetStringData(fda.GetNameId()) != pf.GetStringData(field_data_accessor.GetNameId())) {
                    return;
                }

                if (field_type.IsReference()) {
                    if (pf.GetStringData(panda_file::File::EntityId(fda.GetType())) !=
                        pf.GetStringData(panda_file::File::EntityId(field_data_accessor.GetType()))) {
                        return;
                    }
                }

                field_id = field_data_accessor.GetFieldId();
            });

            class_id = cda.GetSuperClassId();
#ifndef NDEBUG
            ASSERT_PRINT(visited_classes.count(class_id) == 0, "Class hierarchy is incorrect");
            visited_classes.insert(class_id);
#endif
        }

        if (!field_id.IsValid()) {
            // If we cannot find field (for example it's in the class that located in other panda file)
            // assume that field is volatile
            return true;
        }
        ASSERT(field_id.IsValid());
        panda_file::FieldDataAccessor field_da(panda_file_, field_id);
        return field_da.IsVolatile();
    }

    ClassPtr ResolveType([[maybe_unused]] MethodPtr method, size_t id) const override
    {
        return reinterpret_cast<ClassPtr>(id);
    }

    std::string GetFieldName(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(panda_file_, FieldCast(field));
        auto string_data = panda_file_.GetStringData(fda.GetNameId());
        return utf::Mutf8AsCString(string_data.data);
    }

private:
    static compiler::DataType::Type ToCompilerType(panda_file::Type type)
    {
        switch (type.GetId()) {
            case panda_file::Type::TypeId::VOID:
                return compiler::DataType::VOID;
            case panda_file::Type::TypeId::U1:
                return compiler::DataType::BOOL;
            case panda_file::Type::TypeId::I8:
                return compiler::DataType::INT8;
            case panda_file::Type::TypeId::U8:
                return compiler::DataType::UINT8;
            case panda_file::Type::TypeId::I16:
                return compiler::DataType::INT16;
            case panda_file::Type::TypeId::U16:
                return compiler::DataType::UINT16;
            case panda_file::Type::TypeId::I32:
                return compiler::DataType::INT32;
            case panda_file::Type::TypeId::U32:
                return compiler::DataType::UINT32;
            case panda_file::Type::TypeId::I64:
                return compiler::DataType::INT64;
            case panda_file::Type::TypeId::U64:
                return compiler::DataType::UINT64;
            case panda_file::Type::TypeId::F32:
                return compiler::DataType::FLOAT32;
            case panda_file::Type::TypeId::F64:
                return compiler::DataType::FLOAT64;
            case panda_file::Type::TypeId::REFERENCE:
                return compiler::DataType::REFERENCE;
            case panda_file::Type::TypeId::TAGGED:
            case panda_file::Type::TypeId::INVALID:
                return compiler::DataType::ANY;
            default:
                break;
        }
        UNREACHABLE();
    }

    static panda_file::File::EntityId MethodCast(RuntimeInterface::MethodPtr method)
    {
        return panda_file::File::EntityId(reinterpret_cast<uintptr_t>(method));
    }

    static panda_file::File::EntityId ClassCast(RuntimeInterface::ClassPtr cls)
    {
        return panda_file::File::EntityId(reinterpret_cast<uintptr_t>(cls));
    }

    static panda_file::File::EntityId FieldCast(RuntimeInterface::FieldPtr field)
    {
        return panda_file::File::EntityId(reinterpret_cast<uintptr_t>(field));
    }

    const panda_file::File &panda_file_;
};
}  // namespace panda

#endif  // PANDA_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H_
