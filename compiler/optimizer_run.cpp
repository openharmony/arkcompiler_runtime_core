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

#include <iomanip>
#include "optimizer/pass_manager.h"
#include "compiler_options.h"
#include "trace/trace.h"
#include "optimizer_run.h"

#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/codegen_native.h"
#include "optimizer/code_generator/method_properties.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/optimizations/balance_expressions.h"
#include "optimizer/optimizations/code_sink.h"
#include "optimizer/optimizations/deoptimize_elimination.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/if_conversion.h"
#include "optimizer/optimizations/loop_peeling.h"
#include "optimizer/optimizations/loop_unroll.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/memory_barriers.h"
#include "optimizer/optimizations/memory_coalescing.h"
#include "optimizer/optimizations/redundant_loop_elimination.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/scheduler.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "optimizer/optimizations/types_resolving.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/optimizations/move_constants.h"
#include "optimizer/optimizations/adjust_arefs.h"

namespace panda::compiler {

static inline bool RunCodegenPass(Graph *graph)
{
    if (graph->GetMethodProperties().GetRequireFrameSetup()) {
        return graph->RunPass<Codegen>();
    }
    return graph->RunPass<CodegenNative>();
}

bool RunOptimizations(Graph *graph)
{
    auto finalizer = [graph](void * /* unused */) { graph->GetPassManager()->Finalize(); };
    std::unique_ptr<void, decltype(finalizer)> pp(&finalizer, finalizer);

    if (options.WasSetCompilerRegallocRegMask()) {
        COMPILER_LOG(DEBUG, REGALLOC) << "Regalloc mask force set to " << std::hex
                                      << options.GetCompilerRegallocRegMask() << "\n";
        graph->SetArchUsedRegs(options.GetCompilerRegallocRegMask());
    }

    if (!options.IsCompilerNonOptimizing()) {
        // Run optimizations

        // The problem with inlining in OSR mode can be found in `bitops-nsieve-bits` benchmark and it is in the
        // following: we inline the method that has user X within a loop, then peepholes optimize datflow and def of
        // the X become another instruction within inlined method, but SaveStateOsr didn't take it into account, thus,
        // we don't restore value of this new definition.
        // TODO(msherstennikov): find way to inline in OSR mode
        if (!graph->IsOsrMode()) {
            graph->RunPass<Inlining>();
        }
        graph->RunPass<TryCatchResolving>();
        graph->RunPass<Peepholes>();
        graph->RunPass<BranchElimination>();
        graph->RunPass<ValNum>();
        graph->RunPass<Cleanup>();
        if (graph->IsAotMode()) {
            graph->RunPass<Cse>();
        }
        if (graph->IsDynamicMethod()) {
            graph->RunPass<TypesResolving>();
        }
        graph->RunPass<Licm>(options.GetCompilerLicmHoistLimit());
        graph->RunPass<RedundantLoopElimination>();
        graph->RunPass<LoopPeeling>();
        graph->RunPass<Lse>();
        graph->RunPass<ValNum>();
        if (graph->RunPass<Peepholes>() && graph->RunPass<BranchElimination>()) {
            graph->RunPass<Peepholes>();
        }
        graph->RunPass<Cleanup>();
        if (graph->IsAotMode()) {
            graph->RunPass<Cse>();
        }
        graph->RunPass<LoopUnroll>(options.GetCompilerLoopUnrollInstLimit(), options.GetCompilerLoopUnrollFactor());
        graph->RunPass<BalanceExpressions>();
        if (graph->RunPass<Peepholes>()) {
            graph->RunPass<BranchElimination>();
        }
        graph->RunPass<ValNum>();
        if (graph->IsAotMode()) {
            graph->RunPass<Cse>();
        }
        if (graph->RunPass<DeoptimizeElimination>()) {
            graph->RunPass<Peepholes>();
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif  // NDEBUG
        graph->RunPass<Cleanup>();
        graph->RunPass<Lowering>();
        graph->RunPass<CodeSink>();
        graph->RunPass<MemoryCoalescing>(options.IsCompilerMemoryCoalescingAligned());
        graph->RunPass<IfConversion>(options.GetCompilerIfConversionLimit());
        graph->RunPass<Scheduler>();
        // Perform MoveConstants after Scheduler because Scheduler can rearrange constants
        // and cause spillfill in reg alloc
        graph->RunPass<MoveConstants>();
        graph->RunPass<AdjustRefs>();
        graph->RunPass<OptimizeMemoryBarriers>();
    } else {
        // TryCatchResolving is needed in the non-optimizing mode since it removes unreachable for compiler
        // catch-handlers; After supporting catch-handlers' compilation, this pass can be run in the optimizing mode
        // only.
        graph->RunPass<TryCatchResolving>();
    }

    bool fatal_on_err = !options.IsCompilerAllowBackendFailures();
    // Do not try to encode too large graph
    auto inst_size = graph->GetCurrentInstructionId();
    auto insts_per_byte = graph->GetEncoder()->MaxArchInstPerEncoded();
    auto max_bits_in_inst = GetInstructionSizeBits(graph->GetArch());
    if ((inst_size * insts_per_byte * max_bits_in_inst) > options.GetCompilerMaxGenCodeSize()) {
        if (fatal_on_err) {
            LOG(FATAL, COMPILER) << "RunOptimizations failed: code predicted size too big";
        }
        return false;
    }
    graph->RunPass<Cleanup>();
    if (!RegAlloc(graph)) {
        if (fatal_on_err) {
            LOG(FATAL, COMPILER) << "RunOptimizations failed: register allocation error";
        }
        return false;
    }

    if (!RunCodegenPass(graph)) {
        if (fatal_on_err) {
            LOG(FATAL, COMPILER) << "RunOptimizations failed: code generation error";
        }
        return false;
    }

    return true;
}

}  // namespace panda::compiler
