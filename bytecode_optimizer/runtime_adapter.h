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

#ifndef BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
#define BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H

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

    uint32_t ResolveOffsetByIndex(MethodPtr parent_method, uint16_t index) const override
    {
        return panda_file_.ResolveOffsetByIndex(MethodCast(parent_method), index).GetOffset();
    }

    MethodId GetMethodId(MethodPtr method) const override
    {
        return static_cast<MethodId>(reinterpret_cast<uintptr_t>(method));
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

    size_t GetClassIdForMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        return static_cast<size_t>(mda.GetClassId().GetOffset());
    }

    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        auto string_data = panda_file_.GetStringData(mda.GetClassId());

        return std::string(reinterpret_cast<const char *>(string_data.data));
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        panda_file::MethodDataAccessor mda(panda_file_, MethodCast(method));

        auto string_data = panda_file_.GetStringData(mda.GetNameId());

        return std::string(reinterpret_cast<const char *>(string_data.data));
    }

    std::string GetMethodFullName(MethodPtr method, bool /* with_signature */) const override
    {
        auto class_name = GetClassNameFromMethod(method);
        auto method_name = GetMethodName(method);

        return class_name + "::" + method_name;
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
            case panda_file::Type::TypeId::I32:
                return compiler::DataType::INT32;
            case panda_file::Type::TypeId::U32:
                return compiler::DataType::UINT32;
            case panda_file::Type::TypeId::I64:
                return compiler::DataType::INT64;
            case panda_file::Type::TypeId::U64:
                return compiler::DataType::UINT64;
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

    const panda_file::File &panda_file_;
    std::unordered_map<size_t, TypeInfoIndex> instid_type_map_;
    std::unordered_map<int32_t, TypeInfoIndex> pc_type_map_;
    std::string literalarray_key;
    std::pair<size_t, size_t> anno_elem_idx_ = std::make_pair(INVALID_TYPE_INDEX, INVALID_TYPE_INDEX);
};
}  // namespace panda

#endif  // BYTECODE_OPTIMIZER_RUNTIME_ADAPTER_H
