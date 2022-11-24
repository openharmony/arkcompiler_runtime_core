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
#include "libpandafile/class_data_accessor.h"
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

    BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const override
    {
        return const_cast<panda_file::File *>(&panda_file_);
    }

    MethodId ResolveMethodIndex(MethodPtr parent_method, MethodIndex index) const override
    {
        return panda_file_.ResolveMethodIndex(MethodCast(parent_method), index).GetOffset();
    }

    uint32_t ResolveOffsetByIndex(MethodPtr parent_method, uint16_t index) const override
    {
        return panda_file_.ResolveOffsetByIndex(MethodCast(parent_method), index).GetOffset();
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

        ASSERT(!mda.IsExternal());
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

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetNumVregs();
    }

    const uint8_t *GetMethodCode(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetInstructions();
    }

    size_t GetMethodCodeSize(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());
        panda_file::CodeDataAccessor cda(panda_file_, mda.GetCodeId().value());

        return cda.GetCodeSize();
    }

    compiler::SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        ASSERT(!mda.IsExternal());

        auto source_lang = mda.GetSourceLang();
        ASSERT(source_lang.has_value());

        return static_cast<compiler::SourceLanguage>(source_lang.value());
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

    // return true if the method is Jni with exception
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

    bool IsConstructor(MethodPtr method, uint32_t class_id) override
    {
        if (GetClassIdForMethod(method) != class_id) {
            return false;
        }

        panda_file::File::EntityId entity_id(class_id);
        panda_file::SourceLang lang = panda_file::SourceLang::PANDA_ASSEMBLY;

        if (!panda_file_.IsExternal(entity_id)) {
            panda_file::ClassDataAccessor cda(panda_file_, entity_id);
            lang = cda.GetSourceLang().value_or(lang);
        }

        return GetMethodName(method) == GetCtorName(lang);
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

    bool IsFieldVolatile(FieldPtr field) const override
    {
        panda_file::FieldDataAccessor fda(panda_file_, FieldCast(field));

        return fda.IsVolatile();
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

    TypeInfoIndex GetTypeInfoIndexByInstId(size_t id) const override
    {
        const auto it = instid_type_map_.find(id);
        if (it == instid_type_map_.end()) {
            return NO_EXPLICIT_TYPE;
        }
        return it->second;
    }

    bool IsPcBindType(int32_t pc) const override
    {
        return pc_type_map_.find(pc) != pc_type_map_.end();
    }

    bool FillInstIdTypePairByPc(size_t id, int32_t pc) override
    {
        const auto it = pc_type_map_.find(pc);
        if (it != pc_type_map_.end()) {
            instid_type_map_.emplace(id, it->second);
            return true;
        }
        return false;
    }

    bool HasInsTypeinfo() const override
    {
        return !instid_type_map_.empty();
    }

    bool AddPcTypePair(int32_t pc, TypeInfoIndex type) override
    {
        if (pc_type_map_.find(pc) != pc_type_map_.end()) {
            return false;
        }
        pc_type_map_.emplace(pc, type);
        return true;
    }

    bool FillArgTypePairs(std::unordered_map<int32_t, TypeInfoIndex> *map) const override
    {
        ASSERT(map != nullptr);
        ASSERT(map->empty());
        for (const auto &[pc, type] : pc_type_map_) {
            if (pc < 0) {
                map->emplace(pc, type);
            }
        }
        return !map->empty();
    }

    bool SetTypeLiteralArrayKey(std::string key) override
    {
        literalarray_key = key;
        return !literalarray_key.empty();
    }

    const std::string *GetTypeLiteralArrayKey() const override
    {
        return &literalarray_key;
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
    std::unordered_map<size_t, TypeInfoIndex> instid_type_map_;
    std::unordered_map<int32_t, TypeInfoIndex> pc_type_map_;
    std::string literalarray_key;
    std::pair<size_t, size_t> anno_elem_idx_ = std::make_pair(INVALID_TYPE_INDEX, INVALID_TYPE_INDEX);
};
}  // namespace panda

#endif  // PANDA_BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H_
