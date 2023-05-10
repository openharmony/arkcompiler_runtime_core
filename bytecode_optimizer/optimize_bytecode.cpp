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

#include "optimize_bytecode.h"

#include "assembler/assembly-emitter.h"
#include "assembler/extensions/extensions.h"
#include "bytecode_instruction.h"
#include "bytecodeopt_options.h"
#include "codegen.h"
#include "common.h"
#include "compiler/optimizer/ir/constants.h"
#include "compiler/optimizer/ir_builder/ir_builder.h"
#include "compiler/optimizer/ir_builder/pbc_iterator.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/move_constants.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "libpandabase/mem/arena_allocator.h"
#include "libpandabase/mem/pool_manager.h"
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/method_data_accessor.h"
#include "reg_acc_alloc.h"
#include "reg_encoder.h"
#include "runtime_adapter.h"

#include <regex>

namespace panda::bytecodeopt {
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
panda::bytecodeopt::Options options("");

template <typename T>
constexpr void RunOpts(compiler::Graph *graph)
{
    graph->RunPass<compiler::Cleanup>();
    graph->RunPass<T>();
}

template <typename First, typename Second, typename... Rest>
constexpr void RunOpts(compiler::Graph *graph)
{
    RunOpts<First>(graph);
    RunOpts<Second, Rest...>(graph);
}

bool RunOptimizations(compiler::Graph *graph, BytecodeOptIrInterface *iface)
{
    constexpr int OPT_LEVEL_0 = 0;

    if (panda::bytecodeopt::options.GetOptLevel() == OPT_LEVEL_0) {
        return false;
    }

    graph->RunPass<compiler::Cleanup>();
    ASSERT(graph->IsDynamicMethod());
    RunOpts<compiler::ValNum, compiler::Lowering, compiler::MoveConstants>(graph);

    // this pass should run just before register allocator
    graph->RunPass<compiler::Cleanup>();
    graph->RunPass<RegAccAlloc>();

    graph->RunPass<compiler::Cleanup>();
    if (!RegAlloc(graph)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Failed compiler::RegAlloc";
        return false;
    }

    graph->RunPass<compiler::Cleanup>();
    if (!graph->RunPass<RegEncoder>()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Failed RegEncoder";
        return false;
    }

    return true;
}

void BuildMapFromPcToIns(pandasm::Function &function, BytecodeOptIrInterface &ir_interface,
                         const compiler::Graph *graph, compiler::RuntimeInterface::MethodPtr method_ptr)
{
    function.local_variable_debug.clear();
    auto *pc_ins_map = ir_interface.GetPcInsMap();
    pc_ins_map->reserve(function.ins.size());
    auto instructions_buf = graph->GetRuntime()->GetMethodCode(method_ptr);
    compiler::BytecodeInstructions instructions(instructions_buf, graph->GetRuntime()->GetMethodCodeSize(method_ptr));
    size_t idx = 0;
    for (auto insn : instructions) {
        pandasm::Ins &ins = function.ins[idx++];
        pc_ins_map->emplace(instructions.GetPc(insn), &ins);
        if (idx >= function.ins.size()) {
            break;
        }
    }
}

static void ExtractTypeInfo(const pandasm::Function &function, compiler::RuntimeInterface *adapter,
                            std::unordered_map<int32_t, TypeInfoIndex> *order_type_map, const pandasm::Program *prog)
{
    const auto &annos = function.metadata->GetAnnotations();
    const auto type_anno = std::find_if(annos.begin(), annos.end(),
                                        [](const auto &an) { return an.GetName() == TSTYPE_ANNO_RECORD_NAME; });
    if (type_anno == annos.end()) {
        return;
    }
    const auto &elems = type_anno->GetElements();
    const auto type_elem = std::find_if(elems.begin(), elems.end(),
                                        [](const auto &e) { return e.GetName() == TSTYPE_ANNO_ELEMENT_NAME; });
    if (type_elem == elems.end()) {
        return;
    }
    const auto *key_val = type_elem->GetValue();
    ASSERT(key_val != nullptr);
    ASSERT(key_val->GetType() == pandasm::Value::Type::LITERALARRAY);
    const auto key = key_val->GetAsScalar()->GetValue<std::string>();
    adapter->SetTypeLiteralArrayKey(key);
    auto array_iter = prog->literalarray_table.find(key);
    ASSERT(array_iter != prog->literalarray_table.end());
    const auto &array = array_iter->second.literals_;
    // 4: size must be multiple of 4 because values consits of tuple of tag, order, tag, type
    ASSERT(array.size() % 4 == 0);
    size_t i = 1;
    while (i < array.size()) {
        auto order = bit_cast<int32_t>(std::get<uint32_t>(array[i].value_));
        i += 2;  // 2: skip tag between order and type
        TypeInfoIndex type;
        if (array[i].tag_ == panda_file::LiteralTag::LITERALARRAY) {
            type = std::get<std::string>(array[i].value_);
        } else {
            ASSERT(array[i].tag_ == panda_file::LiteralTag::BUILTINTYPEINDEX);
            type = std::get<BuiltinIndexType>(array[i].value_);
        }

        if (order < 0) {
            adapter->AddPcTypePair(order, type);  // arguments
        } else {
            order_type_map->emplace(order, type);  // instructions
        }
        i += 2;  // 2: skip tag between type and order
    }
}

static void BuildMapFromPcToType(const pandasm::Function &function, const compiler::Graph *graph,
                                 compiler::RuntimeInterface::MethodPtr method_ptr, const pandasm::Program *prog)
{
    std::unordered_map<int32_t, TypeInfoIndex> tmp_order_type_map;
    ExtractTypeInfo(function, graph->GetRuntime(), &tmp_order_type_map, prog);
    if (tmp_order_type_map.empty()) {
        return;
    }
    const auto *instruction_buf = graph->GetRuntime()->GetMethodCode(method_ptr);
    compiler::BytecodeInstructions instructions(instruction_buf, graph->GetRuntime()->GetMethodCodeSize(method_ptr));
    int32_t order = 0;
    size_t num_collected = 0;
    for (const auto &insn : instructions) {
        const auto it = tmp_order_type_map.find(order++);
        if (it == tmp_order_type_map.end()) {
            continue;
        }
        auto pc = static_cast<int32_t>(instructions.GetPc(insn));
        graph->GetRuntime()->AddPcTypePair(pc, it->second);
        num_collected++;

        // stop when all typeinfo has been collected
        if (num_collected == tmp_order_type_map.size()) {
            break;
        }
    }
}

static void ColumnNumberPropagate(pandasm::Function *function)
{
    auto &ins_vec = function->ins;
    uint32_t cn = compiler::INVALID_COLUMN_NUM;
    // handle the instructions that are at the beginning of code but do not have column number
    size_t k = 0;
    while (k < ins_vec.size() && cn == compiler::INVALID_COLUMN_NUM) {
        cn = ins_vec[k++].ins_debug.column_number;
    }
    if (cn == compiler::INVALID_COLUMN_NUM) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "Failed ColumnNumberPropagate: All insts have invalid column number";
        return;
    }
    for (size_t j = 0; j < k - 1; j++) {
        ins_vec[j].ins_debug.SetColumnNumber(cn);
    }

    // handle other instructions that do not have column number
    for (; k < ins_vec.size(); k++) {
        if (ins_vec[k].ins_debug.column_number != compiler::INVALID_COLUMN_NUM) {
            cn = ins_vec[k].ins_debug.column_number;
        } else {
            ins_vec[k].ins_debug.SetColumnNumber(cn);
        }
    }
}

static void LineNumberPropagate(pandasm::Function *function)
{
    if (function == nullptr || function->ins.empty()) {
        return;
    }
    size_t ln = 0;
    auto &ins_vec = function->ins;

    // handle the instructions that are at the beginning of code but do not have line number
    size_t i = 0;
    while (i < ins_vec.size() && ln == 0) {
        ln = ins_vec[i++].ins_debug.line_number;
    }
    if (ln == 0) {
        LOG(DEBUG, BYTECODE_OPTIMIZER) << "Failed LineNumberPropagate: All insts have invalid line number";
        return;
    }
    for (size_t j = 0; j < i - 1; j++) {
        ins_vec[j].ins_debug.SetLineNumber(ln);
    }

    // handle other instructions that do not have line number
    for (; i < ins_vec.size(); i++) {
        if (ins_vec[i].ins_debug.line_number != 0) {
            ln = ins_vec[i].ins_debug.line_number;
        } else {
            ins_vec[i].ins_debug.SetLineNumber(ln);
        }
    }
}

static void DebugInfoPropagate(pandasm::Function &function, const compiler::Graph *graph,
                               BytecodeOptIrInterface &ir_interface)
{
    LineNumberPropagate(&function);
    if (graph->IsDynamicMethod()) {
        ColumnNumberPropagate(&function);
    }
    ir_interface.ClearPcInsMap();
}

static bool SkipFunction(const pandasm::Function &function, const std::string &func_name)
{
    if (panda::bytecodeopt::options.WasSetMethodRegex()) {
        static std::regex rgx(panda::bytecodeopt::options.GetMethodRegex());
        if (!std::regex_match(func_name, rgx)) {
            LOG(INFO, BYTECODE_OPTIMIZER) << "Skip Function " << func_name << ": Function's name doesn't match regex";
            return true;
        }
    }

    if (panda::bytecodeopt::options.IsSkipMethodsWithEh() && !function.catch_blocks.empty()) {
        LOG(INFO, BYTECODE_OPTIMIZER) << "Was not optimized " << func_name << ": Function has catch blocks";
        return true;
    }

    if ((function.regs_num + function.GetParamsNum()) > compiler::VIRTUAL_FRAME_SIZE) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Unable to optimize " << func_name
                                       << ": Function frame size is larger than allowed one";
        return true;
    }
    return false;
}

static void SetCompilerOptions(bool is_dynamic)
{
    compiler::options.SetCompilerUseSafepoint(false);
    compiler::options.SetCompilerSupportInitObjectInst(true);
    if (!compiler::options.WasSetCompilerMaxBytecodeSize()) {
        compiler::options.SetCompilerMaxBytecodeSize(MAX_BYTECODE_SIZE);
    }
    if (is_dynamic) {
        panda::bytecodeopt::options.SetSkipMethodsWithEh(true);
    }
}

bool OptimizeFunction(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const panda_file::MethodDataAccessor &mda, bool is_dynamic)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator local_allocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

    SetCompilerOptions(is_dynamic);

    auto ir_interface = BytecodeOptIrInterface(maps, prog);

    auto func_name = ir_interface.GetMethodIdByOffset(mda.GetMethodId().GetOffset());
    LOG(INFO, BYTECODE_OPTIMIZER) << "Optimizing function: " << func_name;

    auto it = prog->function_table.find(func_name);
    if (it == prog->function_table.end()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Cannot find function: " << func_name;
        return false;
    }
    auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());

    panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
    auto graph = allocator.New<compiler::Graph>(&allocator, &local_allocator, Arch::NONE, method_ptr, &adapter, false,
                                                nullptr, is_dynamic, true);

    panda::pandasm::Function &function = it->second;

    if (SkipFunction(function, func_name)) {
        return false;
    }

    BuildMapFromPcToType(function, graph, method_ptr, prog);

    // build map from pc to pandasm::ins (to re-build line-number info in BytecodeGen)
    BuildMapFromPcToIns(function, ir_interface, graph, method_ptr);

    if ((graph == nullptr) || !graph->RunPass<panda::compiler::IrBuilder>()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": IR builder failed!";
        return false;
    }

    if (graph->HasIrreducibleLoop()) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": Graph has irreducible loop!";
        return false;
    }

    if (!RunOptimizations(graph, &ir_interface)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": Running optimizations failed!";
        return false;
    }

    if (!graph->RunPass<BytecodeGen>(&function, &ir_interface, prog)) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Optimizing " << func_name << ": Code generation failed!";
        return false;
    }

    DebugInfoPropagate(function, graph, ir_interface);

    function.value_of_first_param =
        static_cast<int64_t>(graph->GetStackSlotsCount()) - 1;  // Work-around promotion rules
    function.regs_num = static_cast<size_t>(function.value_of_first_param + 1);

    if (auto frame_size = function.regs_num + function.GetParamsNum(); frame_size >= NUM_COMPACTLY_ENCODED_REGS) {
        LOG(INFO, BYTECODE_OPTIMIZER) << "Function " << func_name << " has frame size " << frame_size;
    }

    LOG(DEBUG, BYTECODE_OPTIMIZER) << "Optimized " << func_name;

    return true;
}

bool OptimizePandaFile(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                       const std::string &pfile_name, bool is_dynamic)
{
    auto pfile = panda_file::OpenPandaFile(pfile_name);
    if (!pfile) {
        LOG(FATAL, BYTECODE_OPTIMIZER) << "Can not open binary file: " << pfile_name;
    }

    bool result = true;

    for (uint32_t id : pfile->GetClasses()) {
        panda_file::File::EntityId record_id {id};

        if (pfile->IsExternal(record_id)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*pfile, record_id};
        cda.EnumerateMethods([prog, maps, is_dynamic, &result](panda_file::MethodDataAccessor &mda) {
            if (!mda.IsExternal()) {
                result = OptimizeFunction(prog, maps, mda, is_dynamic) && result;
            }
        });
    }

    return result;
}

bool OptimizeBytecode(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const std::string &pandafile_name, bool is_dynamic, bool has_memory_pool)
{
    ASSERT(prog != nullptr);
    ASSERT(maps != nullptr);

    if (!has_memory_pool) {
        PoolManager::Initialize(PoolType::MALLOC);
    }

    auto res = OptimizePandaFile(prog, maps, pandafile_name, is_dynamic);

    if (!has_memory_pool) {
        PoolManager::Finalize();
    }

    return res;
}
}  // namespace panda::bytecodeopt
