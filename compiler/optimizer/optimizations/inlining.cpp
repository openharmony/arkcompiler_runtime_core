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

#include "inlining.h"
#include "compiler_logger.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/object_type_check_elimination.h"
#include "optimizer/optimizations/peepholes.h"
#include "events/events.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage,-warnings-as-errors)
#define LOG_INLINING(level) COMPILER_LOG(level, INLINING) << GetLogIndent()

namespace panda::compiler {
using MethodPtr = RuntimeInterface::MethodPtr;

static size_t CalculateInstructionsCount(Graph *graph)
{
    size_t count = 0;
    for (auto bb : *graph) {
        if (bb != nullptr && !bb->IsStartBlock() && !bb->IsEndBlock()) {
            for ([[maybe_unused]] auto inst : bb->AllInsts()) {
                switch (inst->GetOpcode()) {
                    case Opcode::Return:
                    case Opcode::ReturnI:
                    case Opcode::ReturnVoid:
                        break;
                    default:
                        count++;
                }
            }
        }
    }
    return count;
}

Inlining::Inlining(Graph *graph, uint32_t instructions_count, uint32_t depth, uint32_t methods_inlined)
    : Optimization(graph),
      cha_(graph->GetRuntime()->GetCha()),
      blacklist_(graph->GetLocalAllocator()->Adapter()),
      return_blocks_(graph->GetLocalAllocator()->Adapter()),
      recievers_(graph->GetLocalAllocator()->Adapter()),
      instructions_count_(instructions_count != 0 ? instructions_count : CalculateInstructionsCount(graph)),
      depth_(depth),
      methods_inlined_(methods_inlined),
      vregs_count_(graph->GetVRegsCount())
{
}

bool Inlining::IsInlineCachesEnabled() const
{
    return DoesArchSupportDeoptimization(GetGraph()->GetArch()) && !options.IsCompilerNoPicInlining();
}

#ifdef PANDA_EVENTS_ENABLED
static void EmitEvent(const Graph *graph, const CallInst *call_inst, const InlineContext &ctx, events::InlineResult res)
{
    auto runtime = graph->GetRuntime();
    events::InlineKind kind;
    if (ctx.cha_devirtualize) {
        kind = events::InlineKind::VIRTUAL_CHA;
    } else if (call_inst->GetOpcode() == Opcode::CallVirtual || ctx.replace_to_static) {
        kind = events::InlineKind::VIRTUAL;
    } else {
        kind = events::InlineKind::STATIC;
    }
    EVENT_INLINE(runtime->GetMethodFullName(graph->GetMethod()),
                 ctx.method != nullptr ? runtime->GetMethodFullName(ctx.method) : "null", call_inst->GetId(), kind,
                 res);
}
#else
// NOLINTNEXTLINE(readability-named-parameter)
static void EmitEvent(const Graph *, const CallInst *, const InlineContext &, events::InlineResult) {}
#endif

bool Inlining::RunImpl()
{
    if (instructions_count_ >= options.GetCompilerInliningMaxInsts()) {
        LOG_INLINING(DEBUG) << "Current method already exceeds instructions limit, so skip inlining. "
                               "Instructions count: "
                            << instructions_count_;
        return false;
    }

    auto blacklist_names = options.GetCompilerInliningBlacklist();
    blacklist_.reserve(blacklist_names.size());

    for (const auto &method_name : blacklist_names) {
        blacklist_.insert(method_name);
    }

    bool inlined = false;

    if (GetGraph()->RunPass<ObjectTypeCheckElimination>() && GetGraph()->RunPass<Peepholes>()) {
        GetGraph()->RunPass<BranchElimination>();
    }
    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (SkipBlock(bb)) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            if (GetGraph()->GetVectorBlocks()[inst->GetBasicBlock()->GetId()] == nullptr) {
                break;
            }
            if (!inst->IsCall() || inst->IsLowLevel()) {
                continue;
            }
            auto call_inst = static_cast<CallInst *>(inst);
            if (call_inst->IsInlined()) {
                continue;
            }

            if (call_inst->IsUnresolved()) {
                LOG_INLINING(DEBUG) << "Unknown method " << call_inst->GetCallMethodId();
                continue;
            }

            ASSERT(call_inst->GetCallMethod() != nullptr);
            inlined |= TryInline(call_inst);
        }
    }

#ifndef NDEBUG
    GetGraph()->SetInliningComplete();
#endif  // NDEBUG

    return inlined;
}

void Inlining::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

/**
 * Get next Parameter instruction.
 * TODO(msherstennikov): this is temporary solution, need to find out better approach
 * @param inst current param instruction
 * @return next Parameter instruction, nullptr if there no more Parameter instructions
 */
Inst *GetNextParam(Inst *inst)
{
    for (auto next_inst = inst->GetNext(); next_inst != nullptr; next_inst = next_inst->GetNext()) {
        if (next_inst->GetOpcode() == Opcode::Parameter) {
            return next_inst;
        }
    }
    return nullptr;
}

bool Inlining::TryInline(CallInst *call_inst)
{
    InlineContext ctx;
    if (!ResolveTarget(call_inst, &ctx)) {
        if (IsInlineCachesEnabled()) {
            return TryInlineWithInlineCaches(call_inst);
        }
        return false;
    }

    ASSERT(!call_inst->IsInlined());

    LOG_INLINING(DEBUG) << "Try to inline(id=" << call_inst->GetId() << (call_inst->IsVirtualCall() ? ", virtual" : "")
                        << ", size=" << GetGraph()->GetRuntime()->GetMethodCodeSize(ctx.method) << ", vregs="
                        << GetGraph()->GetRuntime()->GetMethodArgumentsCount(ctx.method) +
                               GetGraph()->GetRuntime()->GetMethodRegistersCount(ctx.method) + 1
                        << (ctx.cha_devirtualize ? ", CHA" : "")
                        << "): " << GetGraph()->GetRuntime()->GetMethodFullName(ctx.method, true);

    if (DoInline(call_inst, &ctx)) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::SUCCESS);
        return true;
    }
    if (ctx.replace_to_static && !ctx.cha_devirtualize) {
        ASSERT(ctx.method != nullptr);
        call_inst->SetCallMethod(ctx.method);
        call_inst->SetCallMethodId(GetGraph()->GetRuntime()->GetMethodId(ctx.method));
        call_inst->SetOpcode(Opcode::CallStatic);
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::DEVIRTUALIZED);
        return true;
    }

    return false;
}

bool Inlining::TryInlineWithInlineCaches(CallInst *call_inst)
{
    if (GetGraph()->IsAotMode()) {
        // We don't support offline inline caches yet.
        return false;
    }
    auto runtime = GetGraph()->GetRuntime();
    auto pic = runtime->GetInlineCaches();
    if (pic == nullptr) {
        return false;
    }

    auto call_kind = pic->GetClasses(GetGraph()->GetMethod(), call_inst->GetPc(), &recievers_);
    switch (call_kind) {
        case InlineCachesInterface::CallKind::MEGAMORPHIC:
            EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), "-", call_inst->GetId(),
                         events::InlineKind::VIRTUAL_POLYMORPHIC, events::InlineResult::FAIL_MEGAMORPHIC);
            return false;
        case InlineCachesInterface::CallKind::UNKNOWN:
            return false;
        case InlineCachesInterface::CallKind::MONOMORPHIC:
            return DoInlineMonomorphic(call_inst, recievers_[0]);
        case InlineCachesInterface::CallKind::POLYMORPHIC:
            ASSERT(call_kind == InlineCachesInterface::CallKind::POLYMORPHIC);
            return DoInlinePolymorphic(call_inst);
        default:
            break;
    }
    return false;
}

bool Inlining::DoInlineMonomorphic(CallInst *call_inst, RuntimeInterface::ClassPtr receiver)
{
    auto runtime = GetGraph()->GetRuntime();
    InlineContext ctx;
    ctx.method = runtime->ResolveVirtualMethod(receiver, call_inst->GetCallMethod());

    auto call_bb = call_inst->GetBasicBlock();
    auto save_state = call_inst->GetSaveState();
    auto obj_inst = call_inst->GetInput(0).GetInst();

    LOG_INLINING(DEBUG) << "Try to inline monomorphic(size=" << GetGraph()->GetRuntime()->GetMethodCodeSize(ctx.method)
                        << "): " << GetMethodFullName(GetGraph(), ctx.method);

    if (!DoInline(call_inst, &ctx)) {
        return false;
    }

    // Add type guard
    auto get_cls_inst = GetGraph()->CreateInstGetInstanceClass(DataType::REFERENCE, call_inst->GetPc());
    get_cls_inst->SetInput(0, obj_inst);
    auto load_cls_inst = GetGraph()->CreateInstClassImmediate(DataType::REFERENCE, call_inst->GetPc(), receiver);
    auto cmp_inst = GetGraph()->CreateInstCompare(DataType::BOOL, call_inst->GetPc(), ConditionCode::CC_NE);
    cmp_inst->SetInput(0, get_cls_inst);
    cmp_inst->SetInput(1, load_cls_inst);
    cmp_inst->SetOperandsType(DataType::REFERENCE);
    auto deopt_inst = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, call_inst->GetPc());
    deopt_inst->SetDeoptimizeType(DeoptimizeType::INLINE_IC);
    deopt_inst->SetInput(0, cmp_inst);
    deopt_inst->SetSaveState(save_state);
    call_bb->AppendInst(load_cls_inst);
    call_bb->AppendInst(get_cls_inst);
    call_bb->AppendInst(cmp_inst);
    call_bb->AppendInst(deopt_inst);
    EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), runtime->GetMethodFullName(ctx.method),
                 call_inst->GetId(), events::InlineKind::VIRTUAL_MONOMORPHIC, events::InlineResult::SUCCESS);
    return true;
}

void Inlining::CreateCompareClass(CallInst *call_inst, Inst *get_cls_inst, RuntimeInterface::ClassPtr receiver,
                                  BasicBlock *call_bb)
{
    auto load_cls_inst = GetGraph()->CreateInstClassImmediate(DataType::REFERENCE, call_inst->GetPc(), receiver);
    auto cmp_inst = GetGraph()->CreateInstCompare(DataType::BOOL, call_inst->GetPc(), ConditionCode::CC_EQ);
    auto if_inst = GetGraph()->CreateInstIfImm(DataType::BOOL, call_inst->GetPc(), ConditionCode::CC_NE, 0);
    cmp_inst->SetInput(0, load_cls_inst);
    cmp_inst->SetInput(1, get_cls_inst);
    cmp_inst->SetOperandsType(DataType::REFERENCE);
    if_inst->SetInput(0, cmp_inst);
    if_inst->SetOperandsType(DataType::BOOL);
    call_bb->AppendInst(load_cls_inst);
    call_bb->AppendInst(cmp_inst);
    call_bb->AppendInst(if_inst);
}

void Inlining::InsertDeoptimizeInst(CallInst *call_inst, BasicBlock *call_bb)
{
    // If last class compare returns false we need to deoptimize the method.
    // So we construct instruction DeoptimizeIf and insert instead of IfImm inst.
    auto if_inst = call_bb->GetLastInst();
    ASSERT(if_inst != nullptr && if_inst->GetOpcode() == Opcode::IfImm);
    ASSERT(if_inst->CastToIfImm()->GetImm() == 0 && if_inst->CastToIfImm()->GetCc() == ConditionCode::CC_NE);

    auto compare_inst = if_inst->GetInput(0).GetInst()->CastToCompare();
    ASSERT(compare_inst != nullptr && compare_inst->GetCc() == ConditionCode::CC_EQ);
    compare_inst->SetCc(ConditionCode::CC_NE);

    auto deopt_inst = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, call_inst->GetPc());
    deopt_inst->SetDeoptimizeType(DeoptimizeType::INLINE_IC);
    deopt_inst->SetInput(0, compare_inst);
    deopt_inst->SetSaveState(call_inst->GetSaveState());

    call_bb->RemoveInst(if_inst);
    call_bb->AppendInst(deopt_inst);
}

void UpdateParameterDataflow(Graph *graph_inl, Inst *call_inst)
{
    // Replace inlined graph incoming dataflow edges
    auto start_bb = graph_inl->GetStartBlock();
    // Last input is SaveState
    if (call_inst->GetInputsCount() > 1) {
        Inst *param_inst = *start_bb->Insts().begin();
        if (param_inst != nullptr && param_inst->GetOpcode() != Opcode::Parameter) {
            param_inst = GetNextParam(param_inst);
        }
        while (param_inst != nullptr) {
            ASSERT(param_inst);
            auto arg_num = param_inst->CastToParameter()->GetArgNumber();
            ASSERT(arg_num < call_inst->GetInputsCount() - 1);
            auto input = call_inst->GetInput(arg_num);
            param_inst->ReplaceUsers(input.GetInst());
            param_inst = GetNextParam(param_inst);
        }
    }
}

void UpdateExternalParameterDataflow(Graph *graph_inl, Inst *call_inst)
{
    // Replace inlined graph incoming dataflow edges
    auto start_bb = graph_inl->GetStartBlock();
    // Last input is SaveState
    if (call_inst->GetInputsCount() <= 1) {
        return;
    }
    Inst *param_inst = *start_bb->Insts().begin();
    if (param_inst != nullptr && param_inst->GetOpcode() != Opcode::Parameter) {
        param_inst = GetNextParam(param_inst);
    }
    ArenaVector<Inst *> worklist {graph_inl->GetLocalAllocator()->Adapter()};
    while (param_inst != nullptr) {
        auto arg_num = param_inst->CastToParameter()->GetArgNumber();
        ASSERT(arg_num < call_inst->GetInputsCount() - 1);
        auto input = call_inst->GetInput(arg_num);
        for (auto &user : param_inst->GetUsers()) {
            if (user.GetInst()->GetOpcode() == Opcode::NullCheck) {
                user.GetInst()->ReplaceUsers(input.GetInst());
                worklist.push_back(user.GetInst());
            }
        }
        param_inst->ReplaceUsers(input.GetInst());
        param_inst = GetNextParam(param_inst);
    }
    for (auto inst : worklist) {
        auto ss = inst->GetInput(1).GetInst();
        inst->RemoveInputs();
        inst->GetBasicBlock()->EraseInst(inst);
        ss->RemoveInputs();
        ss->GetBasicBlock()->EraseInst(ss);
    }
}

bool Inlining::DoInlinePolymorphic(CallInst *call_inst)
{
    auto runtime = GetGraph()->GetRuntime();
    PhiInst *phi_inst = nullptr;
    PhiInst *local_phi_inst = nullptr;
    bool has_runtime_calls = false;
    BasicBlock *call_bb = nullptr;
    BasicBlock *call_cont_bb = nullptr;

    auto get_cls_inst = GetGraph()->CreateInstGetInstanceClass(DataType::REFERENCE, call_inst->GetPc());

    LOG_INLINING(DEBUG) << "Try inline polymorphic call(" << recievers_.size() << " receivers):";
    LOG_INLINING(DEBUG) << "  instruction: " << *call_inst;

    bool first_inline = true;
    auto size = recievers_.size();
    bool has_unreachable_blocks = false;
    // For each receiver we construct BB for CallVirtual inlined, and BB for Return.Inlined
    // Inlined graph we inserts between the blocks
    for (auto receiver : recievers_) {
        ASSERT(size != 0);
        --size;
        InlineContext ctx;
        ctx.method = runtime->ResolveVirtualMethod(receiver, call_inst->GetCallMethod());
        ASSERT(ctx.method != nullptr);

        LOG_INLINING(DEBUG) << "Inline receiver " << runtime->GetMethodFullName(ctx.method);

        if (!CheckMethodCanBeInlined<true>(call_inst, &ctx)) {
            continue;
        }
        ASSERT(!runtime->IsMethodAbstract(ctx.method));

        auto new_call_inst = call_inst->Clone(GetGraph())->CastToCallVirtual();
        new_call_inst->SetCallMethodId(runtime->GetMethodId(ctx.method));
        new_call_inst->SetCallMethod(ctx.method);
        auto inl_graph = BuildGraph(&ctx, call_inst, new_call_inst);
        if (inl_graph.graph == nullptr) {
            continue;
        }
        vregs_count_ += inl_graph.graph->GetVRegsCount();
        /* before
           call_bb
             call_inst
           succs [call_cont_bb]

       call_cont_bb
     */
        /* after
           call_bb:
              compare_classes
           succs [new_call_bb, call_inlined_block]

           call_inlined_block:
              call_inst.inlined
           succs [inlined_graph]

           inlined graph:
           succs [return_inlined_block]

           return_inlined_block:
              return.inlined
           succs [call_cont_bb]

           new_call_bb:
           succs [call_cont_bb]

           call_cont_bb
               phi(new_call_bb, return_inlined_block)
         */
        if (first_inline) {
            first_inline = false;
            // Split block by call instruction
            call_bb = call_inst->GetBasicBlock();
            call_cont_bb = call_bb->SplitBlockAfterInstruction(call_inst, false);
            call_bb->AppendInst(get_cls_inst);

            if (call_inst->GetType() != DataType::VOID) {
                phi_inst = GetGraph()->CreateInstPhi(call_inst->GetType(), call_inst->GetPc());
                phi_inst->ReserveInputs(recievers_.size() << 1U);
                call_cont_bb->AppendPhi(phi_inst);
            }
        } else {
            auto new_call_bb = GetGraph()->CreateEmptyBlock(call_bb);
            call_bb->GetLoop()->AppendBlock(new_call_bb);
            call_bb->AddSucc(new_call_bb);
            call_bb = new_call_bb;
        }

        CreateCompareClass(call_inst, get_cls_inst, receiver, call_bb);

        // Create call_inlined_block
        auto call_inlined_block = GetGraph()->CreateEmptyBlock(call_bb);
        call_bb->GetLoop()->AppendBlock(call_inlined_block);
        call_bb->AddSucc(call_inlined_block);

        // Create Call.inlined and insert in call_inlined_block
        new_call_inst->AppendInput(call_inst->GetInput(0));
        new_call_inst->AppendInput(call_inst->GetSaveState());
        new_call_inst->SetInlined(true);
        new_call_inst->SetFlag(inst_flags::NO_DST);
        call_inlined_block->PrependInst(new_call_inst);

        // Create return_inlined_block and inster PHI for non void functions
        auto return_inlined_block = GetGraph()->CreateEmptyBlock(call_bb);
        call_bb->GetLoop()->AppendBlock(return_inlined_block);

        if (call_inst->GetType() != DataType::VOID) {
            local_phi_inst = GetGraph()->CreateInstPhi(call_inst->GetType(), call_inst->GetPc());
            local_phi_inst->ReserveInputs(recievers_.size() << 1U);
            return_inlined_block->AppendPhi(local_phi_inst);
        }

        // Inlined graph between call_inlined_block and return_inlined_block
        GetGraph()->SetMaxMarkerIdx(inl_graph.graph->GetCurrentMarkerIdx());
        UpdateParameterDataflow(inl_graph.graph, call_inst);
        UpdateDataflow(inl_graph.graph, call_inst, local_phi_inst, phi_inst);
        MoveConstants(inl_graph.graph);
        UpdateControlflow(inl_graph.graph, call_inlined_block, return_inlined_block);

        if (!return_inlined_block->GetPredsBlocks().empty()) {
            if (inl_graph.has_runtime_calls) {
                auto inlined_return = GetGraph()->CreateInstReturnInlined(DataType::VOID, INVALID_PC);
                return_inlined_block->PrependInst(inlined_return);
                inlined_return->SetInput(0, new_call_inst->GetSaveState());
            }
            if (call_inst->GetType() != DataType::VOID) {
                ASSERT(phi_inst);
                phi_inst->AppendInput(local_phi_inst);
            }
            return_inlined_block->AddSucc(call_cont_bb);
        } else {
            // We need remove return_inlined_block if inlined graph doesn't have Return inst(only Throw or Deoptimize)
            has_unreachable_blocks = true;
        }

        if (inl_graph.has_runtime_calls) {
            has_runtime_calls = true;
        } else {
            new_call_inst->GetBasicBlock()->RemoveInst(new_call_inst);
        }

        GetGraph()->GetPassManager()->GetStatistics()->AddInlinedMethods(1);
        EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), runtime->GetMethodFullName(ctx.method),
                     call_inst->GetId(), events::InlineKind::VIRTUAL_POLYMORPHIC, events::InlineResult::SUCCESS);
        LOG_INLINING(DEBUG) << "Successfully inlined: " << GetMethodFullName(GetGraph(), ctx.method);
        methods_inlined_++;
    }
    // Nothing was inlined
    if (call_bb == nullptr) {
        return false;
    }
    if (call_cont_bb->GetPredsBlocks().empty() || has_unreachable_blocks) {
        GetGraph()->RemoveUnreachableBlocks();
    }

    get_cls_inst->SetInput(0, call_inst->GetInput(0).GetInst());

    InsertDeoptimizeInst(call_inst, call_bb);

    if (call_inst->GetType() != DataType::VOID) {
        call_inst->ReplaceUsers(phi_inst);
    }

    ProcessCallReturnInstructions(call_inst, call_cont_bb, has_runtime_calls);

    if (has_runtime_calls) {
        call_inst->GetBasicBlock()->RemoveInst(call_inst);
    }

    return true;
}

#ifndef NDEBUG
void CheckExternalGraph(Graph *graph)
{
    for (auto bb : graph->GetVectorBlocks()) {
        if (bb == nullptr) {
            continue;
        }

        for (auto inst : bb->AllInstsSafe()) {
            ASSERT(!inst->RequireState());
        }
    }
}
#endif

Inst *Inlining::GetNewDefAndCorrectDF(Inst *call_inst, Inst *old_def)
{
    if (old_def->IsConst()) {
        auto constant = old_def->CastToConstant();
        auto exising_constant = GetGraph()->FindOrAddConstant(constant);
        return exising_constant;
    }
    if (old_def->IsParameter()) {
        auto arg_num = old_def->CastToParameter()->GetArgNumber();
        ASSERT(arg_num < call_inst->GetInputsCount() - 1);
        auto input = call_inst->GetInput(arg_num).GetInst();
        return input;
    }
    ASSERT(old_def->GetOpcode() == Opcode::NullPtr);
    auto exising_nullptr = GetGraph()->GetOrCreateNullPtr();
    return exising_nullptr;
}

bool Inlining::TryInlineExternal(CallInst *call_inst, InlineContext *ctx)
{
    if (TryInlineExternalAot(call_inst, ctx)) {
        return true;
    }
    // Skip external methods
    EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::SKIP_EXTERNAL);
    LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
    return false;
}

/*
 * External methods could be inlined only if there are no instructions requiring state.
 * The only exception are NullChecks that check parameters and used by LoadObject/StoreObject.
 */
bool CheckExternalMethodInstructions(Graph *graph, CallInst *call_inst)
{
    ArenaUnorderedSet<Inst *> suspicious_instructions(graph->GetLocalAllocator()->Adapter());
    for (auto bb : graph->GetVectorBlocks()) {
        if (bb == nullptr) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            bool is_rt_call = inst->RequireState() || inst->IsRuntimeCall();
            auto opcode = inst->GetOpcode();
            if (is_rt_call && opcode == Opcode::NullCheck) {
                suspicious_instructions.insert(inst);
            } else if (is_rt_call && opcode != Opcode::NullCheck) {
                return false;
            }
            if (opcode != Opcode::LoadObject && opcode != Opcode::StoreObject) {
                continue;
            }
            auto nc = inst->GetInput(0).GetInst();
            if (nc->GetOpcode() == Opcode::NullCheck && nc->HasSingleUser()) {
                suspicious_instructions.erase(nc);
            }
            // If LoadObject/StoreObject first input (i.e. object to load data from / store data to)
            // is a method parameter and corresponding call instruction's input is either NullCheck
            // or NewObject then the NullCheck could be removed from external method's body because
            // the parameter is known to be not null at the time load/store will be executed.
            // If we can't prove that the input is not-null then NullCheck could not be eliminated
            // and a method could not be inlined.
            auto obj_input = inst->GetDataFlowInput(0);
            if (obj_input->GetOpcode() != Opcode::Parameter) {
                return false;
            }
            auto param_id = obj_input->CastToParameter()->GetArgNumber();
            if (call_inst->GetInput(param_id).GetInst()->GetOpcode() != Opcode::NullCheck &&
                call_inst->GetDataFlowInput(param_id)->GetOpcode() != Opcode::NewObject) {
                return false;
            }
        }
    }
    return suspicious_instructions.empty();
}

/*
 * We can only inline external methods that don't have runtime calls.
 * The only exception from this rule are methods performing LoadObject/StoreObject
 * to/from a parameter for which we can prove that it can't be null. In that case
 * NullChecks preceding LoadObject/StoreObject are removed from inlined graph.
 */
bool Inlining::TryInlineExternalAot(CallInst *call_inst, InlineContext *ctx)
{
    // We can't guarantee without cha that runtime will use this external file.
    if (!GetGraph()->GetAotData()->GetUseCha()) {
        return false;
    }
    IrBuilderExternalInliningAnalysis bytecode_analysis(GetGraph(), ctx->method);
    if (!GetGraph()->RunPass(&bytecode_analysis)) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::UNSUITABLE);
        LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
        return false;
    }
    auto graph_inl = GetGraph()->CreateChildGraph(ctx->method);
    graph_inl->SetCurrentInstructionId(GetGraph()->GetCurrentInstructionId());

    auto stats = GetGraph()->GetPassManager()->GetStatistics();
    auto saved_pbc_inst_num = stats->GetPbcInstNum();
    if (!TryBuildGraph(*ctx, graph_inl, call_inst, nullptr)) {
        stats->SetPbcInstNum(saved_pbc_inst_num);
        return false;
    }

    graph_inl->RunPass<Cleanup>();

    // External method could be inlined only if there are no instructions requiring state
    // because compiler saves method id into stack map's inline info and there is no way
    // to distinguish id of an external method from id of some method from the same translation unit.
    // Following check ensures that there are no instructions requiring state within parsed
    // external method except NullChecks used by LoadObject/StoreObject that checks nullness
    // of parameters that known to be non-null at the call time. In that case NullChecks
    // will be eliminated and there will no instruction requiring state.
    if (!CheckExternalMethodInstructions(graph_inl, call_inst)) {
        stats->SetPbcInstNum(saved_pbc_inst_num);
        return false;
    }

    vregs_count_ += graph_inl->GetVRegsCount();

    auto method = ctx->method;
    auto runtime = GetGraph()->GetRuntime();
    // Call instruction is already inlined, so change its call id to the resolved method.
    call_inst->SetCallMethodId(runtime->GetMethodId(method));
    call_inst->SetCallMethod(method);

    auto call_bb = call_inst->GetBasicBlock();
    auto call_cont_bb = call_bb->SplitBlockAfterInstruction(call_inst, false);

    GetGraph()->SetMaxMarkerIdx(graph_inl->GetCurrentMarkerIdx());
    // Adjust instruction id counter for parent graph, thereby avoid situation when two instructions have same id.
    GetGraph()->SetCurrentInstructionId(graph_inl->GetCurrentInstructionId());

    UpdateExternalParameterDataflow(graph_inl, call_inst);
    UpdateDataflow(graph_inl, call_inst, call_cont_bb);
    MoveConstants(graph_inl);
    UpdateControlflow(graph_inl, call_bb, call_cont_bb);

    if (call_cont_bb->GetPredsBlocks().empty()) {
        GetGraph()->RemoveUnreachableBlocks();
    } else {
        return_blocks_.push_back(call_cont_bb);
    }

    bool need_barriers = runtime->IsMemoryBarrierRequired(method);
    ProcessCallReturnInstructions(call_inst, call_cont_bb, false, need_barriers);

#ifndef NDEBUG
    CheckExternalGraph(graph_inl);
#endif

    LOG_INLINING(DEBUG) << "Successfully inlined external method: " << GetMethodFullName(GetGraph(), ctx->method);
    methods_inlined_++;
    return true;
}

bool Inlining::DoInline(CallInst *call_inst, InlineContext *ctx)
{
    ASSERT(!call_inst->IsInlined());

    auto method = ctx->method;

    auto runtime = GetGraph()->GetRuntime();

    if (!CheckMethodCanBeInlined<false>(call_inst, ctx)) {
        return false;
    }
    if (GetGraph()->GetRuntime()->IsMethodExternal(GetGraph()->GetMethod(), method)) {
        if (!options.IsCompilerInlineExternalMethods()) {
            // Skip external methods
            EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::SKIP_EXTERNAL);
            LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
            return false;
        }
        if (GetGraph()->IsAotMode()) {
            return TryInlineExternal(call_inst, ctx);
        }
    }

    ASSERT(!runtime->IsMethodAbstract(method));

    // Split block by call instruction
    auto call_bb = call_inst->GetBasicBlock();
    // TODO (a.popov) Support inlining to the catch blocks
    if (call_bb->IsCatch()) {
        return false;
    }

    auto graph_inl = BuildGraph(ctx, call_inst);
    if (graph_inl.graph == nullptr) {
        return false;
    }

    vregs_count_ += graph_inl.graph->GetVRegsCount();

    // Call instruction is already inlined, so change its call id to the resolved method.
    call_inst->SetCallMethodId(runtime->GetMethodId(method));
    call_inst->SetCallMethod(method);

    auto call_cont_bb = call_bb->SplitBlockAfterInstruction(call_inst, false);

    GetGraph()->SetMaxMarkerIdx(graph_inl.graph->GetCurrentMarkerIdx());
    UpdateParameterDataflow(graph_inl.graph, call_inst);
    UpdateDataflow(graph_inl.graph, call_inst, call_cont_bb);

    MoveConstants(graph_inl.graph);

    UpdateControlflow(graph_inl.graph, call_bb, call_cont_bb);

    if (call_cont_bb->GetPredsBlocks().empty()) {
        GetGraph()->RemoveUnreachableBlocks();
    } else {
        return_blocks_.push_back(call_cont_bb);
    }

    if (ctx->cha_devirtualize) {
        InsertChaGuard(call_inst);
        GetCha()->AddDependency(method, GetGraph()->GetOutermostParentGraph()->GetMethod());
        GetGraph()->GetOutermostParentGraph()->AddSingleImplementationMethod(method);
    }

    bool need_barriers = runtime->IsMemoryBarrierRequired(method);
    ProcessCallReturnInstructions(call_inst, call_cont_bb, graph_inl.has_runtime_calls, need_barriers);

    LOG_INLINING(DEBUG) << "Successfully inlined: " << GetMethodFullName(GetGraph(), method);
    GetGraph()->GetPassManager()->GetStatistics()->AddInlinedMethods(1);
    methods_inlined_++;
    return true;
}

void Inlining::ProcessCallReturnInstructions(CallInst *call_inst, BasicBlock *call_cont_bb, bool has_runtime_calls,
                                             bool need_barriers)
{
    if (has_runtime_calls) {
        // In case if inlined graph contains call to runtime we need to preserve call instruction with special `Inlined`
        // flag and create new `ReturnInlined` instruction, hereby codegen can properly handle method frames.
        call_inst->SetInlined(true);
        call_inst->SetFlag(inst_flags::NO_DST);
        // Remove call_inst's all inputs except SaveState and NullCheck(if exist)
        auto save_state = call_inst->GetSaveState();
        ASSERT(save_state->GetOpcode() == Opcode::SaveState);
        auto nullcheck_inst = call_inst->GetInput(0).GetInst();
        call_inst->RemoveInputs();
        if (nullcheck_inst->GetOpcode() == Opcode::NullCheck) {
            call_inst->AppendInput(nullcheck_inst);
        }
        call_inst->AppendInput(save_state);
        call_inst->SetType(DataType::VOID);
        for (auto bb : return_blocks_) {
            auto inlined_return = GetGraph()->CreateInstReturnInlined(DataType::VOID, INVALID_PC);
            auto last_inst = bb->GetLastInst();
            if (bb != call_cont_bb && last_inst != nullptr &&
                (last_inst->GetOpcode() == Opcode::Throw || last_inst->GetOpcode() == Opcode::Deoptimize)) {
                last_inst->InsertBefore(inlined_return);
                inlined_return->SetExtendedLiveness();
            } else {
                bb->PrependInst(inlined_return);
            }
            inlined_return->SetInput(0, call_inst->GetSaveState());
            if (need_barriers) {
                inlined_return->SetFlag(inst_flags::MEM_BARRIER);
            }
        }
    } else {
        // Otherwise we remove call instruction
        auto save_state = call_inst->GetSaveState();
        // Remove SaveState if it has only Call instruction in the users
        if (save_state->GetUsers().Front().GetNext() == nullptr) {
            save_state->GetBasicBlock()->RemoveInst(save_state);
        }
        call_inst->GetBasicBlock()->RemoveInst(call_inst);
    }
    return_blocks_.clear();
}

bool Inlining::CheckBytecode(const InlineContext &ctx, bool *callee_call_runtime, CallInst *call_inst)
{
    auto vregs_num = GetGraph()->GetRuntime()->GetMethodArgumentsCount(ctx.method) +
                     GetGraph()->GetRuntime()->GetMethodRegistersCount(ctx.method) + 1;
    if ((vregs_count_ + vregs_num) >= options.GetCompilerMaxVregsNum()) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::LIMIT);
        LOG_INLINING(DEBUG) << "Reached vregs limit: current=" << vregs_count_ << ", inlined=" << vregs_num;
        return false;
    }
    IrBuilderInliningAnalysis bytecode_analysis(GetGraph(), ctx.method);
    if (!GetGraph()->RunPass(&bytecode_analysis)) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::UNSUITABLE);
        LOG_INLINING(DEBUG) << "Method contains unsuitable bytecode";
        return false;
    }

    if (bytecode_analysis.HasRuntimeCalls() && options.IsCompilerInlineSimpleOnly()) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::UNSUITABLE);
        return false;
    }

    *callee_call_runtime = bytecode_analysis.HasRuntimeCalls();
    return true;
}

bool Inlining::CheckInsrtuctionLimitAndCallInline(InlineContext *ctx, Graph *graph_inl, CallInst *call_inst)
{
    // Don't inline if we reach the limit of instructions and method is big enough.
    auto inlined_insts_count = CalculateInstructionsCount(graph_inl);
    if (inlined_insts_count > SMALL_METHOD_MAX_SIZE &&
        (instructions_count_ + inlined_insts_count) >= options.GetCompilerInliningMaxInsts()) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::LIMIT);
        LOG_INLINING(DEBUG) << "Reached instructions limit: current_size=" << instructions_count_
                            << ", inlined_size=" << inlined_insts_count;
        ctx->replace_to_static = (call_inst->GetOpcode() == Opcode::CallVirtual);
        return false;
    }

    if ((depth_ + 1) < options.GetCompilerInliningMaxDepth()) {
        graph_inl->RunPass<Inlining>(instructions_count_ + inlined_insts_count, depth_ + 1, methods_inlined_ + 1);
    }

    return true;
}

bool Inlining::TryBuildGraph(const InlineContext &ctx, Graph *graph_inl, CallInst *call_inst, CallInst *poly_call_inst)
{
    if (!graph_inl->RunPass<IrBuilder>(ctx.method, poly_call_inst != nullptr ? poly_call_inst : call_inst)) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::FAIL);
        LOG_INLINING(WARNING) << "Graph building failed";
        return false;
    }

    if (graph_inl->HasInfiniteLoop()) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::INF_LOOP);
        COMPILER_LOG(INFO, INLINING) << "Inlining of the methods with infinite loop is not supported";
        return false;
    }

    if (!options.IsCompilerInliningSkipAlwaysThrowMethods()) {
        return true;
    }

    bool always_throw = true;
    // check that end block could be reached only through throw-blocks
    for (auto pred : graph_inl->GetEndBlock()->GetPredsBlocks()) {
        if (pred->GetLastInst()->GetOpcode() != Opcode::Throw) {
            always_throw = false;
            break;
        }
    }
    if (!always_throw) {
        return true;
    }
    EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::UNSUITABLE);
    LOG_INLINING(DEBUG) << "Method always throw an expection, skip inlining: "
                        << GetMethodFullName(GetGraph(), ctx.method);
    return false;
}

void RemoveDeadSafePoints(Graph *graph_inl)
{
    for (auto bb : *graph_inl) {
        if (bb == nullptr || bb->IsStartBlock() || bb->IsEndBlock()) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            if (!inst->IsSaveState()) {
                continue;
            }
            ASSERT(inst->GetOpcode() == Opcode::SafePoint || inst->GetOpcode() == Opcode::SaveStateDeoptimize);
            ASSERT(inst->GetUsers().Empty());
            bb->RemoveInst(inst);
        }
    }
}

bool Inlining::CheckLoops(bool *callee_call_runtime, Graph *graph_inl)
{
    // Check that inlined graph hasn't loops
    graph_inl->RunPass<LoopAnalyzer>();
    if (graph_inl->HasLoop()) {
        if (options.IsCompilerInlineSimpleOnly()) {
            LOG_INLINING(INFO) << "Inlining of the methods with loops is disabled";
            return false;
        }
        *callee_call_runtime = true;
    } else if (!*callee_call_runtime) {
        RemoveDeadSafePoints(graph_inl);
    }
    return true;
}

/* static */
void Inlining::PropagateObjectInfo(Graph *graph_inl, CallInst *call_inst)
{
    // Propagate object type information to the parameters of the inlined graph
    int index = 0;
    // NOLINTNEXTLINE(readability-static-accessed-through-instance)
    for (auto param_inst : graph_inl->GetParameters()) {
        auto input_inst = call_inst->GetDataFlowInput(index);
        param_inst->SetObjectTypeInfo(input_inst->GetObjectTypeInfo());
        index++;
    }
}

InlinedGraph Inlining::BuildGraph(InlineContext *ctx, CallInst *call_inst, CallInst *poly_call_inst)
{
    bool callee_call_runtime = false;
    if (!CheckBytecode(*ctx, &callee_call_runtime, call_inst)) {
        return InlinedGraph();
    }

    auto graph_inl = GetGraph()->CreateChildGraph(ctx->method);

    // Propagate instruction id counter to inlined graph, thereby avoid instructions id duplication
    graph_inl->SetCurrentInstructionId(GetGraph()->GetCurrentInstructionId());

    auto stats = GetGraph()->GetPassManager()->GetStatistics();
    auto saved_pbc_inst_num = stats->GetPbcInstNum();
    if (!TryBuildGraph(*ctx, graph_inl, call_inst, poly_call_inst)) {
        stats->SetPbcInstNum(saved_pbc_inst_num);
        return InlinedGraph();
    }

    PropagateObjectInfo(graph_inl, call_inst);

    // Run basic optimizations
    graph_inl->RunPass<Cleanup>();
    auto peephole_applied = graph_inl->RunPass<Peepholes>();
    auto object_type_applied = graph_inl->RunPass<ObjectTypeCheckElimination>();
    if (peephole_applied || object_type_applied) {
        graph_inl->RunPass<BranchElimination>();
    }
    graph_inl->RunPass<Cleanup>();

    // Don't inline if we reach the limit of instructions and method is big enough.
    if (!CheckInsrtuctionLimitAndCallInline(ctx, graph_inl, call_inst)) {
        stats->SetPbcInstNum(saved_pbc_inst_num);
        return InlinedGraph();
    }

    instructions_count_ += CalculateInstructionsCount(graph_inl);

    GetGraph()->SetMaxMarkerIdx(graph_inl->GetCurrentMarkerIdx());

    // Adjust instruction id counter for parent graph, thereby avoid situation when two instructions have same id.
    GetGraph()->SetCurrentInstructionId(graph_inl->GetCurrentInstructionId());

    if (ctx->cha_devirtualize && !GetCha()->IsSingleImplementation(ctx->method)) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::LOST_SINGLE_IMPL);
        LOG_INLINING(WARNING) << "Method lost single implementation property while we build IR for it";
        stats->SetPbcInstNum(saved_pbc_inst_num);
        return InlinedGraph();
    }

    if (!CheckLoops(&callee_call_runtime, graph_inl)) {
        stats->SetPbcInstNum(saved_pbc_inst_num);
        return InlinedGraph();
    }
    return {graph_inl, callee_call_runtime};
}

template <bool check_external>
bool Inlining::CheckMethodCanBeInlined(const CallInst *call_inst, InlineContext *ctx)
{
    bool is_inline_external_methods = (!GetGraph()->IsAotMode() && options.IsCompilerInlineExternalMethods());
    // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (check_external) {
        if (!is_inline_external_methods &&
            GetGraph()->GetRuntime()->IsMethodExternal(GetGraph()->GetMethod(), ctx->method)) {
            // Skip external methods
            EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::SKIP_EXTERNAL);
            LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
            return false;
        }
    }

    if (!blacklist_.empty()) {
        std::string method_name = GetGraph()->GetRuntime()->GetMethodFullName(ctx->method);
        if (blacklist_.find(method_name) != blacklist_.end()) {
            EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::NOINLINE);
            LOG_INLINING(DEBUG) << "Method is in the blacklist: " << GetMethodFullName(GetGraph(), ctx->method);
            return false;
        }
    }

    if (!GetGraph()->GetRuntime()->IsMethodCanBeInlined(ctx->method)) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::UNSUITABLE);
        return false;
    }

    if (GetGraph()->GetRuntime()->GetMethodName(ctx->method).find("__noinline__") != std::string::npos) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::NOINLINE);
        return false;
    }

    if (GetGraph()->GetRuntime()->GetMethodCodeSize(ctx->method) >= options.GetCompilerInliningMaxSize()) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::LIMIT);
        LOG_INLINING(DEBUG) << "Method is too big: " << GetMethodFullName(GetGraph(), ctx->method);
        ctx->replace_to_static = (call_inst->GetOpcode() == Opcode::CallVirtual);
        // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
        if constexpr (!check_external) {
            if (GetGraph()->GetRuntime()->IsMethodExternal(GetGraph()->GetMethod(), ctx->method)) {
                ctx->replace_to_static &= is_inline_external_methods;
            }
        }
        return false;
    }

    return true;
}

void RemoveReturnVoidInst(BasicBlock *end_block)
{
    for (auto &pred : end_block->GetPredsBlocks()) {
        auto return_inst = pred->GetLastInst();
        if (return_inst->GetOpcode() == Opcode::Throw || return_inst->GetOpcode() == Opcode::Deoptimize) {
            continue;
        }
        ASSERT(return_inst->GetOpcode() == Opcode::ReturnVoid);
        pred->RemoveInst(return_inst);
    }
}

/**
 * Embed inlined dataflow graph into the caller graph.
 */
void Inlining::UpdateDataflow(Graph *graph_inl, Inst *call_inst, std::variant<BasicBlock *, PhiInst *> use,
                              Inst *new_def)
{
    // Replace inlined graph outcoming dataflow edges
    auto end_block = graph_inl->GetEndBlock();
    if (end_block->GetPredsBlocks().size() > 1) {
        if (call_inst->GetType() == DataType::VOID) {
            RemoveReturnVoidInst(end_block);
            return;
        }
        PhiInst *phi_inst = nullptr;
        if (std::holds_alternative<BasicBlock *>(use)) {
            phi_inst = GetGraph()->CreateInstPhi(GetGraph()->GetRuntime()->GetMethodReturnType(graph_inl->GetMethod()),
                                                 INVALID_PC);
            phi_inst->ReserveInputs(end_block->GetPredsBlocks().size());
            std::get<BasicBlock *>(use)->AppendPhi(phi_inst);
        } else {
            phi_inst = std::get<PhiInst *>(use);
            ASSERT(phi_inst != nullptr);
        }
        for (auto &pred : end_block->GetPredsBlocks()) {
            auto return_inst = pred->GetLastInst();
            if (return_inst->GetOpcode() == Opcode::Throw || return_inst->GetOpcode() == Opcode::Deoptimize) {
                continue;
            }
            ASSERT(return_inst->GetOpcode() == Opcode::Return);
            ASSERT(return_inst->GetInputsCount() == 1);
            phi_inst->AppendInput(return_inst->GetInput(0).GetInst());
            pred->RemoveInst(return_inst);
        }
        if (new_def == nullptr) {
            new_def = phi_inst;
        }
        call_inst->ReplaceUsers(new_def);
    } else {
        auto pred_block = end_block->GetPredsBlocks().front();
        auto return_inst = pred_block->GetLastInst();
        ASSERT(return_inst->GetOpcode() == Opcode::Return || return_inst->GetOpcode() == Opcode::ReturnVoid ||
               return_inst->GetOpcode() == Opcode::Throw || return_inst->GetOpcode() == Opcode::Deoptimize);
        if (return_inst->GetOpcode() == Opcode::Return) {
            ASSERT(return_inst->GetInputsCount() == 1);
            auto input_inst = return_inst->GetInput(0).GetInst();
            if (std::holds_alternative<PhiInst *>(use)) {
                auto phi_inst = std::get<PhiInst *>(use);
                phi_inst->AppendInput(input_inst);
            } else {
                call_inst->ReplaceUsers(input_inst);
            }
        }
        if (return_inst->GetOpcode() != Opcode::Throw && return_inst->GetOpcode() != Opcode::Deoptimize) {
            pred_block->RemoveInst(return_inst);
        }
    }
}

/**
 * Embed inlined controlflow graph into the caller graph.
 */
void Inlining::UpdateControlflow(Graph *graph_inl, BasicBlock *call_bb, BasicBlock *call_cont_bb)
{
    auto start_bb = graph_inl->GetStartBlock();

    // Move all blocks from inlined graph to parent
    auto current_loop = call_bb->GetLoop();
    for (auto bb : graph_inl->GetVectorBlocks()) {
        if (bb != nullptr && !bb->IsStartBlock() && !bb->IsEndBlock()) {
            bb->ClearMarkers();
            GetGraph()->AddBlock(bb);
            bb->CopyTryCatchProps(call_bb);
        }
    }
    call_cont_bb->CopyTryCatchProps(call_bb);

    // Fix loop tree
    for (auto loop : graph_inl->GetRootLoop()->GetInnerLoops()) {
        current_loop->AppendInnerLoop(loop);
        loop->SetOuterLoop(current_loop);
    }
    for (auto bb : graph_inl->GetRootLoop()->GetBlocks()) {
        bb->SetLoop(current_loop);
        current_loop->AppendBlock(bb);
    }

    // Connect inlined graph as successor of the first part of call continuation block
    for (auto succ = start_bb->GetSuccsBlocks().back(); !start_bb->GetSuccsBlocks().empty();
         succ = start_bb->GetSuccsBlocks().back()) {
        succ->ReplacePred(start_bb, call_bb);
        start_bb->GetSuccsBlocks().pop_back();
    }

    ASSERT(graph_inl->HasEndBlock());
    auto end_block = graph_inl->GetEndBlock();
    for (auto pred : end_block->GetPredsBlocks()) {
        end_block->RemovePred(pred);
        auto term_inst = pred->GetLastInst();
        if (term_inst != nullptr &&
            (term_inst->GetOpcode() == Opcode::Throw || term_inst->GetOpcode() == Opcode::Deoptimize)) {
            if (!GetGraph()->HasEndBlock()) {
                GetGraph()->CreateEndBlock();
            }
            return_blocks_.push_back(pred);
            pred->ReplaceSucc(end_block, GetGraph()->GetEndBlock());
        } else {
            pred->ReplaceSucc(end_block, call_cont_bb);
        }
    }
}

/**
 * Move constants of the inlined graph to the current one if same constant doesn't already exist.
 * If constant exists just fix callee graph's dataflow to use existing constants.
 */
void Inlining::MoveConstants(Graph *graph_inl)
{
    auto start_bb = graph_inl->GetStartBlock();
    for (ConstantInst *constant = graph_inl->GetFirstConstInst(), *next_constant = nullptr; constant != nullptr;
         constant = next_constant) {
        next_constant = constant->GetNextConst();
        start_bb->EraseInst(constant);
        auto exising_constant = GetGraph()->FindOrAddConstant(constant);
        if (exising_constant != constant) {
            constant->ReplaceUsers(exising_constant);
        }
    }

    // Move NullPtr instruction
    if (graph_inl->HasNullPtrInst()) {
        start_bb->EraseInst(graph_inl->GetNullPtrInst());
        auto exising_nullptr = GetGraph()->GetOrCreateNullPtr();
        graph_inl->GetNullPtrInst()->ReplaceUsers(exising_nullptr);
    }
}

bool Inlining::ResolveTarget(CallInst *call_inst, InlineContext *ctx)
{
    auto runtime = GetGraph()->GetRuntime();
    auto method = call_inst->GetCallMethod();
    if (call_inst->GetOpcode() == Opcode::CallStatic) {
        ctx->method = method;
        return true;
    }

    if (options.IsCompilerNoVirtualInlining()) {
        return false;
    }
    auto object_inst = call_inst->GetDataFlowInput(0);
    auto receiver = object_inst->GetObjectTypeInfo();
    if (receiver) {
        if (runtime->IsInterfaceMethod(method)) {
            ctx->method = runtime->ResolveInterfaceMethod(receiver.GetClass(), call_inst->GetCallMethod());
        } else {
            ctx->method = runtime->ResolveVirtualMethod(receiver.GetClass(), call_inst->GetCallMethod());
        }
        if (ctx->method == nullptr) {
            LOG_INLINING(WARNING) << "Runtime failed to resolve method";
            return false;
        }
        return true;
    }

    // If class or method are final we can resolve the method
    if (runtime->IsMethodFinal(method) || runtime->IsClassFinal(runtime->GetClass(method))) {
        ctx->method = method;
        return true;
    }

    if (ArchTraits<RUNTIME_ARCH>::SUPPORT_DEOPTIMIZATION && !options.IsCompilerNoChaInlining() &&
        !GetGraph()->IsAotMode()) {
        // Try resolve via CHA
        auto cha = GetCha();
        if (cha != nullptr && cha->IsSingleImplementation(method)) {
            auto klass = runtime->GetClass(method);
            ctx->method = runtime->ResolveVirtualMethod(klass, call_inst->GetCallMethod());
            if (ctx->method == nullptr) {
                return false;
            }
            ctx->cha_devirtualize = true;
            return true;
        }
    }

    return false;
}

void Inlining::InsertChaGuard(CallInst *call_inst)
{
    auto save_state = call_inst->GetSaveState();
    auto check_deopt = GetGraph()->CreateInstIsMustDeoptimize(DataType::BOOL, call_inst->GetPc());
    auto deopt = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, call_inst->GetPc());
    deopt->SetDeoptimizeType(DeoptimizeType::INLINE_CHA);
    deopt->SetInput(0, check_deopt);
    deopt->SetInput(1, save_state);
    call_inst->InsertBefore(deopt);
    deopt->InsertBefore(check_deopt);
}

bool Inlining::SkipBlock(const BasicBlock *block) const
{
    if (block == nullptr || block->IsEmpty()) {
        return true;
    }
    if (!options.IsCompilerInliningSkipThrowBlocks()) {
        return false;
    }
    auto last_inst = block->GetLastInst();
    return last_inst->GetOpcode() == Opcode::Throw || last_inst->GetOpcode() == Opcode::Deoptimize;
}
}  // namespace panda::compiler
