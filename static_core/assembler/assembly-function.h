/*
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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_FUNCTION_H
#define PANDA_ASSEMBLER_ASSEMBLY_FUNCTION_H

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "assembly-ins.h"
#include "assembly-label.h"
#include "assembly-type.h"
#include "assembly-debug.h"
#include "assembly-file-location.h"
#include "bytecode_emitter.h"
#include "extensions/extensions.h"
#include "file_items.h"
#include "file_item_container.h"
#include "ide_helpers.h"
#include "meta.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Function {
    struct CatchBlock {
        std::string wholeLine;
        std::string exceptionRecord;
        std::string tryBeginLabel;
        std::string tryEndLabel;
        std::string catchBeginLabel;
        std::string catchEndLabel;
    };

    struct TryCatchInfo {
        std::unordered_map<std::string_view, size_t> tryCatchLabels;
        std::unordered_map<std::string, std::vector<const CatchBlock *>> tryCatchMap;
        std::vector<std::string> tryCatchOrder;
        TryCatchInfo(std::unordered_map<std::string_view, size_t> &labels,
                     std::unordered_map<std::string, std::vector<const CatchBlock *>> &map,
                     std::vector<std::string> &paramTryCatchOrder)
            : tryCatchLabels(labels), tryCatchMap(map), tryCatchOrder(paramTryCatchOrder)
        {
        }
    };

    struct Parameter {
        Type type;
        std::unique_ptr<ParamMetadata> metadata;

        Parameter(Type t, ark::panda_file::SourceLang lang)
            : type(std::move(t)), metadata(extensions::MetadataExtension::CreateParamMetadata(lang))
        {
        }
    };

    std::string name;
    ark::panda_file::SourceLang language;
    std::unique_ptr<FunctionMetadata> metadata;

    std::unordered_map<std::string, ark::pandasm::Label> labelTable;
    std::vector<ark::pandasm::Ins> ins; /* function instruction list */
    std::vector<ark::pandasm::debuginfo::LocalVariable> localVariableDebug;
    std::string sourceFile; /* The file in which the function is defined or empty */
    std::string sourceCode;
    std::vector<CatchBlock> catchBlocks;
    int64_t valueOfFirstParam = -1;
    size_t regsNum = 0;
    size_t profileSize {0};
    std::vector<Parameter> params;
    bool bodyPresence = false;
    Type returnType;
    SourceLocation bodyLocation;
    std::optional<FileLocation> fileLocation;

    void SetInsDebug(const std::vector<debuginfo::Ins> &insDebug)
    {
        ASSERT(insDebug.size() == ins.size());
        for (std::size_t i = 0; i < ins.size(); i++) {
            ins[i].insDebug = insDebug[i];
        }
    }

    void AddInstruction(const ark::pandasm::Ins &instruction)
    {
        ins.emplace_back(instruction);
    }

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    Function(std::string s, ark::panda_file::SourceLang lang, size_t bL, size_t bR, std::string fC, bool d, size_t lN)
        : name(std::move(s)),
          language(lang),
          metadata(extensions::MetadataExtension::CreateFunctionMetadata(lang)),
          fileLocation({fC, bL, bR, lN, d})
    {
    }

    Function(std::string s, ark::panda_file::SourceLang lang)
        : name(std::move(s)), language(lang), metadata(extensions::MetadataExtension::CreateFunctionMetadata(lang))
    {
    }

    std::size_t GetParamsNum() const
    {
        return params.size();
    }

    std::size_t GetTotalRegs() const
    {
        return regsNum;
    }

    bool IsStatic() const
    {
        return (metadata->GetAccessFlags() & ACC_STATIC) != 0;
    }

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    bool Emit(BytecodeEmitter &emitter, panda_file::MethodItem *method,
              const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods,
              const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
              const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes,
              const std::unordered_map<std::string_view, panda_file::StringItem *> &strings,
              const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays) const;

    size_t GetLineNumber(size_t i) const;

    uint32_t GetColumnNumber(size_t i) const;

    void EmitLocalVariable(panda_file::LineNumberProgramItem *program, panda_file::ItemContainer *container,
                           std::vector<uint8_t> *constantPool, uint32_t &pcInc, size_t instructionNumber) const;
    void EmitNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool, uint32_t pcInc,
                    int32_t lineInc) const;
    void EmitLineNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                        uint32_t &prevLineNumber, uint32_t &pcInc, size_t instructionNumber) const;

    // column number is only for dynamic language now
    void EmitColumnNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                          uint32_t &prevColumnNumber, uint32_t &pcInc, size_t instructionNumber) const;

    void BuildLineNumberProgram(panda_file::DebugInfoItem *debugItem, const std::vector<uint8_t> &bytecode,
                                panda_file::ItemContainer *container, std::vector<uint8_t> *constantPool,
                                bool emitDebugInfo) const;

    Function::TryCatchInfo MakeOrderAndOffsets(const std::vector<uint8_t> &bytecode) const;

    std::vector<panda_file::CodeItem::TryBlock> BuildTryBlocks(
        panda_file::MethodItem *method, const std::unordered_map<std::string, panda_file::BaseClassItem *> &classItems,
        const std::vector<uint8_t> &bytecode) const;

    bool HasImplementation() const
    {
        return !metadata->IsForeign() && metadata->HasImplementation();
    }

    bool IsParameter(uint32_t regNumber) const
    {
        return regNumber >= regsNum;
    }

    bool CanThrow() const
    {
        return std::any_of(ins.cbegin(), ins.cend(), [](const Ins &insn) { return insn.CanThrow(); });
    }

    bool HasDebugInfo() const
    {
        return std::any_of(ins.cbegin(), ins.cend(), [](const Ins &insn) { return insn.HasDebugInfo(); });
    }

    void DebugDump() const;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_FUNCTION_H
