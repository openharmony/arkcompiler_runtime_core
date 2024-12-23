/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "inst.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/statuses_impl.h"

#include "libabckit/src/codegen/codegen_static.h"

#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/analysis/rpo.h"
#include "src/adapter_static/metadata_inspect_static.h"
#include "src/adapter_static/abckit_static.h"
#include "src/adapter_static/helpers_static.h"
#include "static_core/assembler/assembly-program.h"

#include "static_core/compiler/optimizer/ir/graph_checker.h"
#include "static_core/compiler/optimizer/ir/graph_cloner.h"
#include "static_core/compiler/optimizer/analysis/loop_analyzer.h"
#include "static_core/compiler/optimizer/optimizations/cleanup.h"
#include "static_core/compiler/optimizer/optimizations/move_constants.h"
#include "static_core/compiler/optimizer/optimizations/lowering.h"
#include "static_core/bytecode_optimizer/reg_encoder.h"

#include <cstdint>
#include <string>

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

namespace libabckit {

// ========================================
// Create / Update
// ========================================
AbckitString *CreateStringStatic(AbckitFile *file, const char *value)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "\"" << value << "\"" << '\n';
    auto *prog = file->GetStaticProgram();
    const auto &[progValueIter, _] = prog->strings.insert(value);
    const auto &progValue = *progValueIter;
    auto &strings = file->strings;

    if (strings.find(progValue) != strings.end()) {
        return strings.at(progValue).get();
    }

    auto s = std::make_unique<AbckitString>();
    s->impl = progValue;
    strings.insert({progValue, std::move(s)});
    return strings[progValue].get();
}

void FunctionSetGraphStatic(AbckitCoreFunction *function, AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG_FUNC;

    auto *func = function->GetArkTSImpl()->GetStaticImpl();

    auto graphImpl =
        compiler::GraphCloner(graph->impl, graph->impl->GetAllocator(), graph->impl->GetLocalAllocator()).CloneGraph();

    graphImpl->RemoveUnreachableBlocks();

    CheckInvalidOpcodes(graphImpl, false);

    LIBABCKIT_LOG(DEBUG) << "======================== BEFORE CODEGEN ========================\n";
    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);
    LIBABCKIT_LOG(DEBUG) << "============================================\n";

    LIBABCKIT_LOG_DUMP(graphImpl->Dump(&std::cerr), DEBUG);

    LIBABCKIT_LOG(DEBUG) << "============================================\n";

    graphImpl->InvalidateAnalysis<compiler::LoopAnalyzer>();

    if (!ark::compiler::GraphChecker(graphImpl).Check()) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": Graph Verifier failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    graphImpl->RunPass<compiler::Lowering>();

    graphImpl->RunPass<compiler::Cleanup>(false);

    graphImpl->RunPass<compiler::MoveConstants>();

    if (!AllocateRegisters(graphImpl, CodeGenStatic::RESERVED_REG)) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": RegAllocGraphColoring failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    if (!graphImpl->RunPass<bytecodeopt::RegEncoder>()) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": RegEncoder failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    if (!graphImpl->RunPass<CodeGenStatic>(func, graph->irInterface)) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": Code generation failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    func->valueOfFirstParam = static_cast<int64_t>(graphImpl->GetStackSlotsCount()) - 1L;
    func->regsNum = static_cast<size_t>(func->valueOfFirstParam + 1U);

    LIBABCKIT_LOG(DEBUG) << "======================== AFTER CODEGEN ========================\n";
    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);
    LIBABCKIT_LOG(DEBUG) << "============================================\n";
}

AbckitLiteral *FindOrCreateLiteralBoolStatic(AbckitFile *file, bool value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralBoolStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU8Static(AbckitFile *file, uint8_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU8StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU16Static(AbckitFile *file, uint16_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU16StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralMethodAffiliateStatic(AbckitFile *file, uint16_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralMethodAffiliateStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU32Static(AbckitFile *file, uint32_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU32StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU64Static(AbckitFile *file, uint64_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU64StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralFloatStatic(AbckitFile *file, float value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralFloatStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralDoubleStatic(AbckitFile *file, double value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralDoubleStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralStringStatic(AbckitFile *file, const char *value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralStringStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralMethodStatic(AbckitFile *file, AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralMethodStaticImpl(file, GetMangleFuncName(function));
}

static void EmplaceType(ark::pandasm::Program *prog, AbckitLiteral *value)
{
    switch (LiteralGetTagStatic(value)) {
        case ABCKIT_LITERAL_TAG_BOOL:
            prog->arrayTypes.emplace(pandasm::Type("u1", 1));
            break;
        case ABCKIT_LITERAL_TAG_BYTE:
            prog->arrayTypes.emplace(pandasm::Type("u8", 1));
            break;
        case ABCKIT_LITERAL_TAG_SHORT:
            prog->arrayTypes.emplace(pandasm::Type("u16", 1));
            break;
        case ABCKIT_LITERAL_TAG_INTEGER:
            prog->arrayTypes.emplace(pandasm::Type("u32", 1));
            break;
        case ABCKIT_LITERAL_TAG_LONG:
            prog->arrayTypes.emplace(pandasm::Type("u64", 1));
            break;
        case ABCKIT_LITERAL_TAG_FLOAT:
            prog->arrayTypes.emplace(pandasm::Type("f32", 1));
            break;
        case ABCKIT_LITERAL_TAG_DOUBLE:
            prog->arrayTypes.emplace(pandasm::Type("f64", 1));
            break;
        case ABCKIT_LITERAL_TAG_STRING:
            prog->arrayTypes.emplace(pandasm::Type("reference", 1));
            break;
        default:
            UNREACHABLE();
    }
}

AbckitLiteralArray *CreateLiteralArrayStatic(AbckitFile *file, AbckitLiteral **value, size_t size)
{
    LIBABCKIT_LOG_FUNC;
    auto *prog = file->GetStaticProgram();
    EmplaceType(prog, *value);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto lit = *reinterpret_cast<pandasm::LiteralArray::Literal *>(value[0]->val.get());
    pandasm::LiteralArray::Literal tagLit;
    tagLit.tag = panda_file::LiteralTag::TAGVALUE;
    tagLit.value = static_cast<uint8_t>(lit.tag);
    std::vector<pandasm::LiteralArray::Literal> stdLitArr;
    stdLitArr.emplace_back(tagLit);

    pandasm::LiteralArray::Literal len;
    len.tag = panda_file::LiteralTag::INTEGER;
    len.value = static_cast<uint32_t>(size);
    stdLitArr.emplace_back(len);

    prog->arrayTypes.emplace(pandasm::Type("u32", 1));
    for (size_t i = 0; i < size; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto lit = *reinterpret_cast<pandasm::LiteralArray::Literal *>(value[i]->val.get());
        stdLitArr.emplace_back(lit);
    }

    // NOLINTNEXTLINE(cert-msc51-cpp)
    uint32_t arrayOffset = 0;
    while (prog->literalarrayTable.find(std::to_string(arrayOffset)) != prog->literalarrayTable.end()) {
        arrayOffset++;
    }
    auto arrayName = std::to_string(arrayOffset);

    prog->literalarrayTable.emplace(arrayName, pandasm::LiteralArray());
    pandasm::LiteralArray &arrImpl = prog->literalarrayTable[arrayName];
    arrImpl.literals = std::move(stdLitArr);

    auto litarr = std::make_unique<AbckitLiteralArray>(file, &arrImpl);
    return file->litarrs.emplace_back(std::move(litarr)).get();
}

AbckitValue *FindOrCreateValueU1Static(AbckitFile *file, bool value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateValueU1StaticImpl(file, value);
}

AbckitValue *FindOrCreateValueDoubleStatic(AbckitFile *file, double value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateValueDoubleStaticImpl(file, value);
}

AbckitValue *FindOrCreateValueStringStatic(AbckitFile *file, const char *value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateValueStringStaticImpl(file, value);
}

}  // namespace libabckit
