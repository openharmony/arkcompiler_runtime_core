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

#include "helpers_static.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"
#include "assembler/annotation.h"
#include "libabckit/include/c/statuses.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"

#include "libabckit/src/wrappers/graph_wrapper/graph_wrapper.h"

#include "mem/arena_allocator.h"
#include "modifiers.h"
#include "src/logger.h"
#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"
#include "static_core/assembler/assembly-emitter.h"

#include "static_core/compiler/compiler_options.h"
#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir_builder/ir_builder.h"
#include "static_core/compiler/optimizer/optimizations/cleanup.h"
#include "static_core/bytecode_optimizer/check_resolver.h"

#include <cstdint>
#include <iostream>

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

namespace libabckit {

// ========================================
// Module
// ========================================

bool ModuleEnumerateAnonymousFunctionsStatic(AbckitCoreModule *m, void *data,
                                             bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &function : m->functions) {
        if (!FunctionIsAnonymousStatic(function.get())) {
            continue;
        }
        if (!cb(function.get(), data)) {
            return false;
        }
    }
    return true;
}

// ========================================
// Namespace
// ========================================

// ========================================
// Class
// ========================================

AbckitString *ClassGetNameStatic(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;
    auto *record = klass->GetArkTSImpl()->impl.GetStaticClass();
    auto [moduleName, className] = ClassGetNames(record->name);

    return CreateStringStatic(klass->owningModule->file, className.data());
}

// ========================================
// Function
// ========================================
AbckitString *FunctionGetNameStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *functionImpl = function->GetArkTSImpl()->GetStaticImpl();

    auto functionName = pandasm::MangleFunctionName(functionImpl->name, functionImpl->params, functionImpl->returnType);
    auto croppedName = FuncNameCropModule(functionName);
    return CreateStringStatic(function->owningModule->file, croppedName.data());
}

bool SetMethodOffset(pandasm::Function *func, pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                     uint32_t *methodOffset)
{
    auto funcFullName = pandasm::MangleFunctionName(func->name, func->params, func->returnType);
    for (auto &[id, s] : maps->methods) {
        if (s == funcFullName) {
            *methodOffset = id;
        }
    }
    if (*methodOffset == 0) {
        LIBABCKIT_LOG(DEBUG) << "methodOffset == 0\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_TODO);
        return true;
    }
    return false;
}

void RemoveInsts(compiler::Graph *graphImpl)
{
    graphImpl->RunPass<ark::bytecodeopt::CheckResolver>();
    graphImpl->RunPass<compiler::Cleanup>(false);
    std::vector<std::pair<compiler::BasicBlock *, compiler::Inst *>> instsToRemove;
    for (auto bb : graphImpl->GetBlocksRPO()) {
        for ([[maybe_unused]] auto inst : bb->AllInsts()) {
            if (inst->IsSaveState() || (inst->GetOpcode() == compiler::Opcode::SaveStateDeoptimize)) {
                ASSERT(!inst->HasUsers());
                /*
                 * NOTE(nsizov): find out why we still have non-movable
                 *               SaveStates with no users
                 */
                instsToRemove.emplace_back(bb, inst);
            }
        }
    }
    for (auto &[bb, inst] : instsToRemove) {
        bb->RemoveInst(inst);
    }
}

static void DeleteGraphArgs(ArenaAllocator *allocator, ArenaAllocator *localAllocator,
                            AbckitRuntimeAdapterStatic *adapter, AbckitIrInterface *irInterface)
{
    delete allocator;
    delete localAllocator;
    delete adapter;
    delete irInterface;
    statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_TODO);
}

static AbckitGraph *CreateGraph(AbckitCoreFunction *function, AbckitIrInterface *irInterface,
                                ark::compiler::Graph *graphImpl, AbckitRuntimeAdapterStatic *adapter)
{
    auto graph = new AbckitGraph;
    graph->file = function->owningModule->file;
    graph->function = function;
    graph->irInterface = irInterface;
    graph->impl = graphImpl;
    GraphWrapper::CreateGraphWrappers(graph);

    auto *ctxGInternal =
        new CtxGInternal {graphImpl->GetAllocator(), graphImpl->GetLocalAllocator(), irInterface, adapter};
    graph->internal = ctxGInternal;
    return graph;
}

AbckitGraph *CreateGraphFromFunctionStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;

    ark::compiler::g_options.SetCompilerFrameSize("default");
    ark::compiler::g_options.SetCompilerUseSafepoint(false);
    auto *func = function->GetArkTSImpl()->GetStaticImpl();

    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);
    LIBABCKIT_LOG(DEBUG) << func->name << '\n';

    auto *file = function->owningModule->file;
    auto program = file->GetStaticProgram();

    auto maps = std::make_unique<pandasm::AsmEmitter::PandaFileToPandaAsmMaps>();
    auto pf = pandasm::AsmEmitter::Emit(*program, maps.get()).release();
    if (pf == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "pf == nullptr\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_TODO);
        return nullptr;
    }

    uint32_t methodOffset = 0;
    if (SetMethodOffset(func, maps.get(), &methodOffset)) {
        return nullptr;
    }

    auto *irInterface =
        new AbckitIrInterface(maps->methods, maps->fields, maps->classes, maps->strings, maps->literalarrays);

    auto *allocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    auto *localAllocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
    auto *adapter = new AbckitRuntimeAdapterStatic(*pf);

    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(methodOffset);  // NOTE
    auto graphImpl = allocator->New<compiler::Graph>(
        compiler::Graph::GraphArgs {allocator, localAllocator, Arch::NONE, methodPtr, adapter}, nullptr, false, false,
        true);
    if (graphImpl == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "graph == nullptr\n";
        DeleteGraphArgs(allocator, localAllocator, adapter, irInterface);
        return nullptr;
    }
    graphImpl->SetLanguage(SourceLanguage::ETS);
    graphImpl->SetAbcKit();
#ifndef NDEBUG
    graphImpl->SetLowLevelInstructionsEnabled();
#endif

    bool irBuilderRes = graphImpl->RunPass<ark::compiler::IrBuilder>();
    if (!irBuilderRes) {
        LIBABCKIT_LOG(DEBUG) << "!irBuilderRes\n";
        DeleteGraphArgs(allocator, localAllocator, adapter, irInterface);
        return nullptr;
    }

    RemoveInsts(graphImpl);
    CheckInvalidOpcodes(graphImpl, false);
    LIBABCKIT_LOG_DUMP(graphImpl->Dump(&std::cerr), DEBUG);

    return CreateGraph(function, irInterface, graphImpl, adapter);
}

bool FunctionIsStaticStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_STATIC) != 0x0;
}

bool FunctionIsCtorStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    size_t pos = func->name.rfind('.');
    ASSERT(pos != std::string::npos);
    std::string name = func->name.substr(pos + 1);
    return name == "_ctor_";  // NOTE(ivagin)
}

bool FunctionIsAnonymousStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    size_t pos = func->name.rfind('.');
    ASSERT(pos != std::string::npos);
    std::string name = func->name.substr(pos + 1);
    return name.find("lambda$invoke$") == 0;
}

bool FunctionIsNativeStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_NATIVE) != 0x0;
}

// ========================================
// Annotation
// ========================================

// ========================================
// ImportDescriptor
// ========================================

// ========================================
// ExportDescriptor
// ========================================

// ========================================
// Literal
// ========================================

bool LiteralGetBoolStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsBoolValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return false;
    }
    return std::get<bool>(literal->value);
}

uint8_t LiteralGetU8Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsByteValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint8_t>(literal->value);
}

uint16_t LiteralGetU16Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsShortValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint16_t>(literal->value);
}

uint16_t LiteralGetMethodAffiliateStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (literal->tag != panda_file::LiteralTag::METHODAFFILIATE) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint16_t>(literal->value);
}

uint32_t LiteralGetU32Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsIntegerValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint32_t>(literal->value);
}

uint64_t LiteralGetU64Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsLongValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint64_t>(literal->value);
}

float LiteralGetFloatStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsFloatValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<float>(literal->value);
}

double LiteralGetDoubleStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsDoubleValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<double>(literal->value);
}

AbckitString *LiteralGetStringStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto s = std::make_unique<AbckitString>();
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsStringValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return nullptr;
    }

    auto &strings = lit->file->strings;

    auto &val = std::get<std::string>(literal->value);
    if (strings.find(val) != strings.end()) {
        return strings[val].get();
    }
    s->impl = val;
    strings.insert({val, std::move(s)});

    return strings[val].get();
}

AbckitLiteralTag LiteralGetTagStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *litImpl = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    switch (litImpl->tag) {
        case panda_file::LiteralTag::ARRAY_U1:
        case panda_file::LiteralTag::BOOL:
            return ABCKIT_LITERAL_TAG_BOOL;
        case panda_file::LiteralTag::ARRAY_U8:
        case panda_file::LiteralTag::ARRAY_I8:
            return ABCKIT_LITERAL_TAG_BYTE;
        case panda_file::LiteralTag::ARRAY_U16:
        case panda_file::LiteralTag::ARRAY_I16:
            return ABCKIT_LITERAL_TAG_SHORT;
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::INTEGER:
            return ABCKIT_LITERAL_TAG_INTEGER;
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_I64:
            return ABCKIT_LITERAL_TAG_LONG;
        case panda_file::LiteralTag::ARRAY_F32:
        case panda_file::LiteralTag::FLOAT:
            return ABCKIT_LITERAL_TAG_FLOAT;
        case panda_file::LiteralTag::ARRAY_F64:
        case panda_file::LiteralTag::DOUBLE:
            return ABCKIT_LITERAL_TAG_DOUBLE;
        case panda_file::LiteralTag::ARRAY_STRING:
        case panda_file::LiteralTag::STRING:
            return ABCKIT_LITERAL_TAG_STRING;
        case panda_file::LiteralTag::NULLVALUE:
            return ABCKIT_LITERAL_TAG_NULLVALUE;
        case panda_file::LiteralTag::TAGVALUE:
            return ABCKIT_LITERAL_TAG_TAGVALUE;
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
            return ABCKIT_LITERAL_TAG_METHOD;
        case panda_file::LiteralTag::METHODAFFILIATE:
            return ABCKIT_LITERAL_TAG_METHODAFFILIATE;
        default:
            return ABCKIT_LITERAL_TAG_INVALID;
    }
}

// ========================================
// Value
// ========================================

AbckitType *ValueGetTypeStatic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    auto *pVal = static_cast<pandasm::ScalarValue *>(value->GetStaticImpl());
    AbckitTypeId id;
    switch (pVal->GetType()) {
        case pandasm::Value::Type::U1:
            id = ABCKIT_TYPE_ID_U1;
            break;
        case pandasm::Value::Type::U8:
            id = ABCKIT_TYPE_ID_U8;
            break;
        case pandasm::Value::Type::U16:
            id = ABCKIT_TYPE_ID_U16;
            break;
        case pandasm::Value::Type::U32:
            id = ABCKIT_TYPE_ID_U32;
            break;
        case pandasm::Value::Type::U64:
            id = ABCKIT_TYPE_ID_U64;
            break;
        case pandasm::Value::Type::F64:
            id = ABCKIT_TYPE_ID_F64;
            break;
        case pandasm::Value::Type::STRING:
            id = ABCKIT_TYPE_ID_STRING;
            break;
        default:
            LIBABCKIT_UNIMPLEMENTED;
    }
    // NOTE implement logic for classes
    return GetOrCreateType(value->file, id, 0, nullptr);
}

bool ValueGetU1Static(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeStatic(value)->id != ABCKIT_TYPE_ID_U1) {
        statuses::SetLastError(ABCKIT_STATUS_TODO);
        return false;
    }

    auto *pVal = static_cast<pandasm::ScalarValue *>(value->GetStaticImpl());
    return pVal->GetValue<bool>();
}

double ValueGetDoubleStatic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeStatic(value)->id != ABCKIT_TYPE_ID_F64) {
        statuses::SetLastError(ABCKIT_STATUS_TODO);
        return 0.0;
    }

    auto *pVal = static_cast<pandasm::ScalarValue *>(value->GetStaticImpl());
    return pVal->GetValue<double>();
}

AbckitString *ValueGetStringStatic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeStatic(value)->id != ABCKIT_TYPE_ID_STRING) {
        statuses::SetLastError(ABCKIT_STATUS_TODO);
        return nullptr;
    }

    auto *pVal = static_cast<pandasm::ScalarValue *>(value->GetStaticImpl());
    auto valImpl = pVal->GetValue<std::string>();
    return CreateStringStatic(value->file, valImpl.data());
}

AbckitLiteralArray *ArrayValueGetLiteralArrayStatic(AbckitValue * /*value*/)
{
    LIBABCKIT_LOG_FUNC;
    statuses::SetLastError(ABCKIT_STATUS_TODO);
    return nullptr;
}

}  // namespace libabckit
