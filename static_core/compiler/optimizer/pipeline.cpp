/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "pipeline.h"
#include "compiler_options.h"
#include "inplace_task_runner.h"
#include "background_task_runner.h"
#include "compiler_task_runner.h"

#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/codegen_native.h"
#include "optimizer/code_generator/method_properties.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/visualizer_printer.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/monitor_analysis.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/optimizations/balance_expressions.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/checks_elimination.h"
#include "optimizer/optimizations/code_sink.h"
#include "optimizer/optimizations/deoptimize_elimination.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/escape.h"
#include "optimizer/optimizations/if_conversion.h"
#include "optimizer/optimizations/inlining.h"
#include "optimizer/optimizations/licm.h"
#include "optimizer/optimizations/licm_conditions.h"
#include "optimizer/optimizations/loop_idioms.h"
#include "optimizer/optimizations/loop_peeling.h"
#include "optimizer/optimizations/loop_unswitch.h"
#include "optimizer/optimizations/loop_unroll.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/lse.h"
#include "optimizer/optimizations/memory_barriers.h"
#include "optimizer/optimizations/memory_coalescing.h"
#include "optimizer/optimizations/peepholes.h"
#include "optimizer/optimizations/phi_type_resolving.h"
#include "optimizer/optimizations/redundant_loop_elimination.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/scheduler.h"
#include "optimizer/optimizations/simplify_string_builder.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "optimizer/optimizations/inline_intrinsics.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/optimizations/cse.h"
#include "optimizer/optimizations/move_constants.h"
#include "optimizer/optimizations/adjust_arefs.h"
#include "optimizer/optimizations/if_merging.h"

#include "plugins/create_pipeline_includes.h"

namespace panda::compiler {

std::unique_ptr<Pipeline> Pipeline::Create(Graph *graph)
{
    switch (graph->GetLanguage()) {
#include "plugins/create_pipeline.h"
        default:
            return std::make_unique<Pipeline>(graph);
    }
}

static inline bool RunCodegenPass(Graph *graph)
{
    if (graph->GetMethodProperties().GetRequireFrameSetup()) {
        return graph->RunPass<Codegen>();
    }
    return graph->RunPass<CodegenNative>();
}

/* static */
template <TaskRunnerMode RUNNER_MODE>
void Pipeline::Run(CompilerTaskRunner<RUNNER_MODE> task_runner)
{
    auto pipeline = task_runner.GetContext().GetPipeline();
    auto *graph = pipeline->GetGraph();
#if !defined(NDEBUG) && !defined(PANDA_TARGET_MOBILE)
    if (OPTIONS.IsCompilerVisualizerDump()) {
        graph->GetPassManager()->InitialDumpVisualizerGraph();
    }
#endif  // NDEBUG && PANDA_TARGET_MOBILE

    task_runner.AddFinalize(
        [](CompilerContext<RUNNER_MODE> &compiler_ctx) { compiler_ctx.GetGraph()->GetPassManager()->Finalize(); });

    if (OPTIONS.WasSetCompilerRegallocRegMask()) {
        COMPILER_LOG(DEBUG, REGALLOC) << "Regalloc mask force set to " << std::hex
                                      << OPTIONS.GetCompilerRegallocRegMask() << "\n";
        graph->SetArchUsedRegs(OPTIONS.GetCompilerRegallocRegMask());
    }

    if (!OPTIONS.IsCompilerNonOptimizing()) {
        task_runner.SetTaskOnSuccess([](CompilerTaskRunner<RUNNER_MODE> next_runner) {
            Pipeline::RunRegAllocAndCodeGenPass<RUNNER_MODE>(std::move(next_runner));
        });
        bool success = pipeline->RunOptimizations();
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), success);
        return;
    }
    // TryCatchResolving is needed in the non-optimizing mode since it removes unreachable for compiler
    // catch-handlers; After supporting catch-handlers' compilation, this pass can be run in the optimizing mode
    // only.
    graph->template RunPass<TryCatchResolving>();
    if (!graph->template RunPass<MonitorAnalysis>()) {
        LOG(WARNING, COMPILER) << "Compiler detected incorrect monitor policy";
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), false);
        return;
    }
    Pipeline::RunRegAllocAndCodeGenPass<RUNNER_MODE>(std::move(task_runner));
}

/* static */
template <TaskRunnerMode RUNNER_MODE>
void Pipeline::RunRegAllocAndCodeGenPass(CompilerTaskRunner<RUNNER_MODE> task_runner)
{
    auto *graph = task_runner.GetContext().GetPipeline()->GetGraph();
    bool fatal_on_err = !OPTIONS.IsCompilerAllowBackendFailures();
    // Do not try to encode too large graph
    auto inst_size = graph->GetCurrentInstructionId();
    auto insts_per_byte = graph->GetEncoder()->MaxArchInstPerEncoded();
    auto max_bits_in_inst = GetInstructionSizeBits(graph->GetArch());
    if ((inst_size * insts_per_byte * max_bits_in_inst) > OPTIONS.GetCompilerMaxGenCodeSize()) {
        if (fatal_on_err) {
            LOG(FATAL, COMPILER) << "RunOptimizations failed: code predicted size too big";
        }
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), false);
        return;
    }
    graph->template RunPass<Cleanup>();

    task_runner.SetTaskOnSuccess([fatal_on_err](CompilerTaskRunner<RUNNER_MODE> next_runner) {
        next_runner.AddCallbackOnFail([fatal_on_err]([[maybe_unused]] CompilerContext<RUNNER_MODE> &compiler_ctx) {
            if (fatal_on_err) {
                LOG(FATAL, COMPILER) << "RunOptimizations failed: code generation error";
            }
        });
        bool success = RunCodegenPass(next_runner.GetContext().GetPipeline()->GetGraph());
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(next_runner), success);
    });
    bool success = RegAlloc(graph);
    if (!success && fatal_on_err) {
        LOG(FATAL, COMPILER) << "RunOptimizations failed: register allocation error";
    }
    CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(task_runner), success);
}

bool Pipeline::RunOptimizations()
{
    auto graph = GetGraph();

    /* peepholer and branch elimination have some parts that have
     * to be delayed up until loop unrolling is done, however, if
     * loop unrolling is not going to be run we don't have to delay */
    if (!OPTIONS.IsCompilerLoopUnroll()) {
        graph->SetUnrollComplete();
    }
    graph->RunPass<Peepholes>();
    graph->RunPass<BranchElimination>();
    graph->RunPass<SimplifyStringBuilder>();
    graph->RunPass<Cleanup>();

    // The problem with inlining in OSR mode can be found in `bitops-nsieve-bits` benchmark and it is in the
    // following: we inline the method that has user X within a loop, then peepholes optimize datflow and def of
    // the X become another instruction within inlined method, but SaveStateOsr didn't take it into account, thus,
    // we don't restore value of this new definition.
    // NOTE(msherstennikov): find way to inline in OSR mode
    if (!graph->IsOsrMode()) {
        graph->RunPass<Inlining>();
    }
    graph->RunPass<CatchInputs>();
    graph->RunPass<TryCatchResolving>();
    if (!graph->RunPass<MonitorAnalysis>()) {
        LOG(WARNING, COMPILER) << "Compiler detected incorrect monitor policy";
        return false;
    }
    graph->RunPass<Peepholes>();
    graph->RunPass<BranchElimination>();
    graph->RunPass<ValNum>();
    graph->RunPass<IfMerging>();
    graph->RunPass<Cleanup>(false);
    graph->RunPass<Peepholes>();
    if (graph->IsAotMode()) {
        graph->RunPass<Cse>();
    }
    if (graph->IsDynamicMethod()) {
        graph->RunPass<InlineIntrinsics>();
        graph->RunPass<PhiTypeResolving>();
        graph->RunPass<Peepholes>();
        graph->RunPass<BranchElimination>();
        graph->RunPass<ValNum>();
        graph->RunPass<Cleanup>(false);
    }
    graph->RunPass<ChecksElimination>();
    graph->RunPass<Licm>(OPTIONS.GetCompilerLicmHoistLimit());
    graph->RunPass<LicmConditions>();
    graph->RunPass<RedundantLoopElimination>();
    graph->RunPass<LoopPeeling>();
    graph->RunPass<LoopUnswitch>(OPTIONS.GetCompilerLoopUnswitchMaxLevel(), OPTIONS.GetCompilerLoopUnswitchMaxInsts());
    graph->RunPass<Lse>();
    graph->RunPass<ValNum>();
    if (graph->RunPass<Peepholes>() && graph->RunPass<BranchElimination>()) {
        graph->RunPass<Peepholes>();
        graph->RunPass<ValNum>();
    }
    graph->RunPass<Cleanup>();
    if (graph->IsAotMode()) {
        graph->RunPass<Cse>();
    }
    graph->RunPass<EscapeAnalysis>();
    graph->RunPass<LoopIdioms>();
    graph->RunPass<ChecksElimination>();
    graph->RunPass<LoopUnroll>(OPTIONS.GetCompilerLoopUnrollInstLimit(), OPTIONS.GetCompilerLoopUnrollFactor());

    /* to be removed once generic loop unrolling is implemented */
    ASSERT(graph->IsUnrollComplete());

    graph->RunPass<Peepholes>();
    graph->RunPass<BranchElimination>();
    graph->RunPass<BalanceExpressions>();
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
    graph->RunPass<Cleanup>(false);
    graph->RunPass<Lowering>();
    graph->RunPass<Cleanup>(false);
    graph->RunPass<CodeSink>();
    graph->RunPass<MemoryCoalescing>(OPTIONS.IsCompilerMemoryCoalescingAligned());
    graph->RunPass<IfConversion>(OPTIONS.GetCompilerIfConversionLimit());
    graph->RunPass<Scheduler>();
    // Perform MoveConstants after Scheduler because Scheduler can rearrange constants
    // and cause spillfill in reg alloc
    graph->RunPass<MoveConstants>();
    if (graph->RunPass<AdjustRefs>()) {
        graph->RunPass<Cleanup>(false);
    }
    graph->RunPass<OptimizeMemoryBarriers>();

    return true;
}

template void Pipeline::Run<BACKGROUND_MODE>(CompilerTaskRunner<BACKGROUND_MODE>);
template void Pipeline::Run<INPLACE_MODE>(CompilerTaskRunner<INPLACE_MODE>);
template void Pipeline::RunRegAllocAndCodeGenPass<BACKGROUND_MODE>(CompilerTaskRunner<BACKGROUND_MODE>);
template void Pipeline::RunRegAllocAndCodeGenPass<INPLACE_MODE>(CompilerTaskRunner<INPLACE_MODE>);

}  // namespace panda::compiler
