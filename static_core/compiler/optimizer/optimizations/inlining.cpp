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

namespace panda::compiler {
using MethodPtr = RuntimeInterface::MethodPtr;

// Explicitly instantiate both versions of CheckMethodCanBeInlined since they can be used in subclasses
template bool Inlining::CheckMethodCanBeInlined<false, true>(const CallInst *, InlineContext *);
template bool Inlining::CheckMethodCanBeInlined<true, true>(const CallInst *, InlineContext *);
template bool Inlining::CheckMethodCanBeInlined<false, false>(const CallInst *, InlineContext *);
template bool Inlining::CheckMethodCanBeInlined<true, false>(const CallInst *, InlineContext *);

inline bool CanReplaceWithCallStatic(Opcode opcode)
{
    switch (opcode) {
        case Opcode::CallResolvedVirtual:
        case Opcode::CallVirtual:
            return true;
        default:
            return false;
    }
}

size_t Inlining::CalculateInstructionsCount(Graph *graph)
{
    size_t count = 0;
    for (auto bb : *graph) {
        if (bb != nullptr && !bb->IsStartBlock() && !bb->IsEndBlock()) {
            for (auto inst : bb->Insts()) {
                if (inst->IsSaveState()) {
                    continue;
                }
                switch (inst->GetOpcode()) {
                    case Opcode::Return:
                    case Opcode::ReturnI:
                    case Opcode::ReturnVoid:
                    case Opcode::Phi:
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
      depth_(depth),
      methods_inlined_(methods_inlined),
      instructions_count_(instructions_count != 0 ? instructions_count : CalculateInstructionsCount(graph)),
      instructions_limit_(OPTIONS.GetCompilerInliningMaxInsts()),
      return_blocks_(graph->GetLocalAllocator()->Adapter()),
      blacklist_(graph->GetLocalAllocator()->Adapter()),
      vregs_count_(graph->GetVRegsCount()),
      cha_(graph->GetRuntime()->GetCha())
{
}

bool Inlining::IsInlineCachesEnabled() const
{
    return DoesArchSupportDeoptimization(GetGraph()->GetArch()) && !OPTIONS.IsCompilerNoPicInlining();
}

#ifdef PANDA_EVENTS_ENABLED
static void EmitEvent(const Graph *graph, const CallInst *call_inst, const InlineContext &ctx, events::InlineResult res)
{
    auto runtime = graph->GetRuntime();
    events::InlineKind kind;
    if (ctx.cha_devirtualize) {
        kind = events::InlineKind::VIRTUAL_CHA;
    } else if (CanReplaceWithCallStatic(call_inst->GetOpcode()) || ctx.replace_to_static) {
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
    GetGraph()->RunPass<LoopAnalyzer>();

    auto blacklist_names = OPTIONS.GetCompilerInliningBlacklist();
    blacklist_.reserve(blacklist_names.size());

    for (const auto &method_name : blacklist_names) {
        blacklist_.insert(method_name);
    }
    return Do();
}

void Inlining::RunOptimizations() const
{
    if (GetGraph()->GetParentGraph() == nullptr && GetGraph()->RunPass<ObjectTypeCheckElimination>() &&
        GetGraph()->RunPass<Peepholes>()) {
        GetGraph()->RunPass<BranchElimination>();
    }
}

bool Inlining::Do()
{
    bool inlined = false;
    RunOptimizations();

    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (SkipBlock(bb)) {
            continue;
        }
        for (auto inst : bb->InstsSafe()) {
            if (GetGraph()->GetVectorBlocks()[inst->GetBasicBlock()->GetId()] == nullptr) {
                break;
            }

            if (!IsInstSuitableForInline(inst)) {
                continue;
            }

            inlined |= TryInline(static_cast<CallInst *>(inst));
        }
    }

#ifndef NDEBUG
    GetGraph()->SetInliningComplete();
#endif  // NDEBUG

    return inlined;
}

bool Inlining::IsInstSuitableForInline(Inst *inst) const
{
    if (!inst->IsCall()) {
        return false;
    }
    auto call_inst = static_cast<CallInst *>(inst);
    ASSERT(!call_inst->IsDynamicCall());
    if (call_inst->IsInlined() || call_inst->IsLaunchCall()) {
        return false;
    }
    if (call_inst->IsUnresolved() || call_inst->GetCallMethod() == nullptr) {
        LOG_INLINING(DEBUG) << "Unknown method " << call_inst->GetCallMethodId();
        return false;
    }
    ASSERT(call_inst->GetCallMethod() != nullptr);
    return true;
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
        if (IsIntrinsic(&ctx)) {
            call_inst->GetBasicBlock()->RemoveInst(call_inst);
        }
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::SUCCESS);
        return true;
    }
    if (ctx.replace_to_static && !ctx.cha_devirtualize) {
        ASSERT(ctx.method != nullptr);
        if (call_inst->GetCallMethod() != ctx.method) {
            // Replace method id only if the methods are different.
            // Otherwise, leave the method id as is because:
            // 1. In aot mode the new method id can refer to a method from a different file
            // 2. In jit mode the method id does not matter
            call_inst->SetCallMethodId(GetGraph()->GetRuntime()->GetMethodId(ctx.method));
        }
        call_inst->SetCallMethod(ctx.method);
        if (call_inst->GetOpcode() == Opcode::CallResolvedVirtual) {
            // Drop the first argument - the resolved method
            ASSERT(!call_inst->GetInputs().empty());
            for (size_t i = 0; i < call_inst->GetInputsCount() - 1; i++) {
                call_inst->SetInput(i, call_inst->GetInput(i + 1).GetInst());
            }
            call_inst->RemoveInput(call_inst->GetInputsCount() - 1);
            ASSERT(!call_inst->GetInputTypes()->empty());
            call_inst->GetInputTypes()->erase(call_inst->GetInputTypes()->begin());
        }
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

    ArenaVector<RuntimeInterface::ClassPtr> receivers(GetGraph()->GetLocalAllocator()->Adapter());
    auto call_kind = pic->GetClasses(GetGraph()->GetMethod(), call_inst->GetPc(), &receivers);
    switch (call_kind) {
        case InlineCachesInterface::CallKind::MEGAMORPHIC:
            EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), "-", call_inst->GetId(),
                         events::InlineKind::VIRTUAL_POLYMORPHIC, events::InlineResult::FAIL_MEGAMORPHIC);
            return false;
        case InlineCachesInterface::CallKind::UNKNOWN:
            return false;
        case InlineCachesInterface::CallKind::MONOMORPHIC:
            return DoInlineMonomorphic(call_inst, receivers[0]);
        case InlineCachesInterface::CallKind::POLYMORPHIC:
            ASSERT(call_kind == InlineCachesInterface::CallKind::POLYMORPHIC);
            return DoInlinePolymorphic(call_inst, &receivers);
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
    auto obj_inst = call_inst->GetObjectInst();

    LOG_INLINING(DEBUG) << "Try to inline monomorphic(size=" << runtime->GetMethodCodeSize(ctx.method)
                        << "): " << GetMethodFullName(GetGraph(), ctx.method);

    if (!DoInline(call_inst, &ctx)) {
        return false;
    }

    // Add type guard
    auto get_cls_inst = GetGraph()->CreateInstGetInstanceClass(DataType::REFERENCE, call_inst->GetPc(), obj_inst);
    auto load_cls_inst = GetGraph()->CreateInstLoadImmediate(DataType::REFERENCE, call_inst->GetPc(), receiver);
    auto cmp_inst = GetGraph()->CreateInstCompare(DataType::BOOL, call_inst->GetPc(), get_cls_inst, load_cls_inst,
                                                  DataType::REFERENCE, ConditionCode::CC_NE);
    auto deopt_inst = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, call_inst->GetPc(), cmp_inst, save_state,
                                                         DeoptimizeType::INLINE_IC);
    if (IsIntrinsic(&ctx)) {
        call_inst->InsertBefore(load_cls_inst);
        call_inst->InsertBefore(get_cls_inst);
        call_inst->InsertBefore(cmp_inst);
        call_inst->InsertBefore(deopt_inst);
        call_inst->GetBasicBlock()->RemoveInst(call_inst);
    } else {
        call_bb->AppendInst(load_cls_inst);
        call_bb->AppendInst(get_cls_inst);
        call_bb->AppendInst(cmp_inst);
        call_bb->AppendInst(deopt_inst);
    }

    EVENT_INLINE(runtime->GetMethodFullName(GetGraph()->GetMethod()), runtime->GetMethodFullName(ctx.method),
                 call_inst->GetId(), events::InlineKind::VIRTUAL_MONOMORPHIC, events::InlineResult::SUCCESS);
    return true;
}

void Inlining::CreateCompareClass(CallInst *call_inst, Inst *get_cls_inst, RuntimeInterface::ClassPtr receiver,
                                  BasicBlock *call_bb)
{
    auto load_cls_inst = GetGraph()->CreateInstLoadImmediate(DataType::REFERENCE, call_inst->GetPc(), receiver);
    auto cmp_inst = GetGraph()->CreateInstCompare(DataType::BOOL, call_inst->GetPc(), load_cls_inst, get_cls_inst,
                                                  DataType::REFERENCE, ConditionCode::CC_EQ);
    auto if_inst = GetGraph()->CreateInstIfImm(DataType::BOOL, call_inst->GetPc(), cmp_inst, 0, DataType::BOOL,
                                               ConditionCode::CC_NE);
    call_bb->AppendInst(load_cls_inst);
    call_bb->AppendInst(cmp_inst);
    call_bb->AppendInst(if_inst);
}

void Inlining::InsertDeoptimizeInst(CallInst *call_inst, BasicBlock *call_bb, DeoptimizeType deopt_type)
{
    // If last class compare returns false we need to deoptimize the method.
    // So we construct instruction DeoptimizeIf and insert instead of IfImm inst.
    auto if_inst = call_bb->GetLastInst();
    ASSERT(if_inst != nullptr && if_inst->GetOpcode() == Opcode::IfImm);
    ASSERT(if_inst->CastToIfImm()->GetImm() == 0 && if_inst->CastToIfImm()->GetCc() == ConditionCode::CC_NE);

    auto compare_inst = if_inst->GetInput(0).GetInst()->CastToCompare();
    ASSERT(compare_inst != nullptr && compare_inst->GetCc() == ConditionCode::CC_EQ);
    compare_inst->SetCc(ConditionCode::CC_NE);

    auto deopt_inst = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, call_inst->GetPc(), compare_inst,
                                                         call_inst->GetSaveState(), deopt_type);

    call_bb->RemoveInst(if_inst);
    call_bb->AppendInst(deopt_inst);
}

void Inlining::InsertCallInst(CallInst *call_inst, BasicBlock *call_bb, BasicBlock *ret_bb, Inst *phi_inst)
{
    ASSERT(phi_inst == nullptr || phi_inst->GetBasicBlock() == ret_bb);
    // Insert new BB
    auto new_call_bb = GetGraph()->CreateEmptyBlock(call_bb);
    call_bb->GetLoop()->AppendBlock(new_call_bb);
    call_bb->AddSucc(new_call_bb);
    new_call_bb->AddSucc(ret_bb);

    // Copy SaveState inst
    auto ss = call_inst->GetSaveState();
    auto clone_ss = static_cast<SaveStateInst *>(ss->Clone(GetGraph()));
    for (size_t input_idx = 0; input_idx < ss->GetInputsCount(); input_idx++) {
        clone_ss->AppendInput(ss->GetInput(input_idx));
        clone_ss->SetVirtualRegister(input_idx, ss->GetVirtualRegister(input_idx));
    }
    new_call_bb->AppendInst(clone_ss);

    // Copy Call inst
    auto clone_call = call_inst->Clone(GetGraph());
    for (auto input : call_inst->GetInputs()) {
        clone_call->AppendInput(input.GetInst());
    }
    clone_call->SetSaveState(clone_ss);
    new_call_bb->AppendInst(clone_call);

    // Set return value in phi inst
    if (phi_inst != nullptr) {
        phi_inst->AppendInput(clone_call);
    }
}

void Inlining::UpdateParameterDataflow(Graph *graph_inl, Inst *call_inst)
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
            if (call_inst->GetOpcode() == Opcode::CallResolvedVirtual ||
                call_inst->GetOpcode() == Opcode::CallResolvedStatic) {
                ASSERT(arg_num != ParameterInst::DYNAMIC_NUM_ARGS);
                arg_num += 1;  // skip method_reg
            }
            Inst *input = nullptr;
            if (arg_num < call_inst->GetInputsCount() - 1) {
                input = call_inst->GetInput(arg_num).GetInst();
            } else if (arg_num == ParameterInst::DYNAMIC_NUM_ARGS) {
                input = GetGraph()->FindOrCreateConstant(call_inst->GetInputsCount() - 1);
            } else {
                input = GetGraph()->FindOrCreateConstant(DataType::Any(coretypes::TaggedValue::VALUE_UNDEFINED));
            }
            param_inst->ReplaceUsers(input);
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
        ASSERT(arg_num != ParameterInst::DYNAMIC_NUM_ARGS);
        if (call_inst->GetOpcode() == Opcode::CallResolvedVirtual ||
            call_inst->GetOpcode() == Opcode::CallResolvedStatic) {
            arg_num += 1;  // skip method_reg
        }
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

static inline CallInst *CloneVirtualCallInst(CallInst *call, Graph *graph)
{
    if (call->GetOpcode() == Opcode::CallVirtual) {
        return call->Clone(graph)->CastToCallVirtual();
    }
    if (call->GetOpcode() == Opcode::CallResolvedVirtual) {
        return call->Clone(graph)->CastToCallResolvedVirtual();
    }
    UNREACHABLE();
}

bool Inlining::DoInlinePolymorphic(CallInst *call_inst, ArenaVector<RuntimeInterface::ClassPtr> *receivers)
{
    LOG_INLINING(DEBUG) << "Try inline polymorphic call(" << receivers->size() << " receivers):";
    LOG_INLINING(DEBUG) << "  instruction: " << *call_inst;

    bool has_unreachable_blocks = false;
    bool has_runtime_calls = false;
    auto runtime = GetGraph()->GetRuntime();
    auto get_cls_inst = GetGraph()->CreateInstGetInstanceClass(DataType::REFERENCE, call_inst->GetPc());
    PhiInst *phi_inst = nullptr;
    BasicBlock *call_bb = nullptr;
    BasicBlock *call_cont_bb = nullptr;
    auto inlined_methods = methods_inlined_;

    // For each receiver we construct BB for CallVirtual inlined, and BB for Return.Inlined
    // Inlined graph we inserts between the blocks:
    // BEFORE:
    //     call_bb:
    //         call_inst
    //         succs [call_cont_bb]
    //     call_cont_bb:
    //
    // AFTER:
    //     call_bb:
    //         compare_classes
    //     succs [new_call_bb, call_inlined_block]
    //
    //     call_inlined_block:
    //         call_inst.inlined
    //     succs [inlined_graph]
    //
    //     inlined graph:
    //     succs [return_inlined_block]
    //
    //     return_inlined_block:
    //         return.inlined
    //     succs [call_cont_bb]
    //
    //     new_call_bb:
    //     succs [call_cont_bb]
    //
    //     call_cont_bb
    //         phi(new_call_bb, return_inlined_block)
    for (auto receiver : *receivers) {
        InlineContext ctx;
        ctx.method = runtime->ResolveVirtualMethod(receiver, call_inst->GetCallMethod());
        ASSERT(ctx.method != nullptr && !runtime->IsMethodAbstract(ctx.method));
        LOG_INLINING(DEBUG) << "Inline receiver " << runtime->GetMethodFullName(ctx.method);
        if (!CheckMethodCanBeInlined<true, false>(call_inst, &ctx)) {
            continue;
        }
        ASSERT(ctx.intrinsic_id == RuntimeInterface::IntrinsicId::INVALID);

        // Create Call.inlined
        CallInst *new_call_inst = CloneVirtualCallInst(call_inst, GetGraph());
        new_call_inst->SetCallMethodId(runtime->GetMethodId(ctx.method));
        new_call_inst->SetCallMethod(ctx.method);
        auto inl_graph = BuildGraph(&ctx, call_inst, new_call_inst);
        if (inl_graph.graph == nullptr) {
            continue;
        }
        vregs_count_ += inl_graph.graph->GetVRegsCount();
        if (call_bb == nullptr) {
            // Split block by call instruction
            call_bb = call_inst->GetBasicBlock();
            call_cont_bb = call_bb->SplitBlockAfterInstruction(call_inst, false);
            call_bb->AppendInst(get_cls_inst);
            if (call_inst->GetType() != DataType::VOID) {
                phi_inst = GetGraph()->CreateInstPhi(call_inst->GetType(), call_inst->GetPc());
                phi_inst->ReserveInputs(receivers->size() << 1U);
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

        // Insert Call.inlined in call_inlined_block
        new_call_inst->AppendInput(call_inst->GetObjectInst());
        new_call_inst->AppendInput(call_inst->GetSaveState());
        new_call_inst->SetInlined(true);
        new_call_inst->SetFlag(inst_flags::NO_DST);
        call_inlined_block->PrependInst(new_call_inst);

        // Create return_inlined_block and inster PHI for non void functions
        auto return_inlined_block = GetGraph()->CreateEmptyBlock(call_bb);
        call_bb->GetLoop()->AppendBlock(return_inlined_block);
        PhiInst *local_phi_inst = nullptr;
        if (call_inst->GetType() != DataType::VOID) {
            local_phi_inst = GetGraph()->CreateInstPhi(call_inst->GetType(), call_inst->GetPc());
            local_phi_inst->ReserveInputs(receivers->size() << 1U);
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
                auto inlined_return =
                    GetGraph()->CreateInstReturnInlined(DataType::VOID, INVALID_PC, new_call_inst->GetSaveState());
                return_inlined_block->PrependInst(inlined_return);
            }
            if (call_inst->GetType() != DataType::VOID) {
                ASSERT(phi_inst);
                // clang-tidy think that phi_inst can be nullptr
                phi_inst->AppendInput(local_phi_inst);  // NOLINT
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
    if (call_bb == nullptr) {
        // Nothing was inlined
        return false;
    }
    if (call_cont_bb->GetPredsBlocks().empty() || has_unreachable_blocks) {
        GetGraph()->RemoveUnreachableBlocks();
    }

    get_cls_inst->SetInput(0, call_inst->GetObjectInst());

    if (methods_inlined_ - inlined_methods == receivers->size()) {
        InsertDeoptimizeInst(call_inst, call_bb);
    } else {
        InsertCallInst(call_inst, call_bb, call_cont_bb, phi_inst);
    }
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
        if (bb != nullptr) {
            for (auto inst : bb->AllInstsSafe()) {
                ASSERT(!inst->RequireState());
            }
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
            auto param_id = obj_input->CastToParameter()->GetArgNumber() + call_inst->GetObjectIndex();
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

bool Inlining::DoInlineIntrinsic(CallInst *call_inst, InlineContext *ctx)
{
    auto intrinsic_id = ctx->intrinsic_id;
    ASSERT(intrinsic_id != RuntimeInterface::IntrinsicId::INVALID);
    ASSERT(call_inst != nullptr);
    if (!EncodesBuiltin(GetGraph()->GetRuntime(), intrinsic_id, GetGraph()->GetArch())) {
        return false;
    }
    IntrinsicInst *inst = GetGraph()->CreateInstIntrinsic(call_inst->GetType(), call_inst->GetPc(), intrinsic_id);
    bool need_save_state = inst->RequireState();

    size_t inputs_count = call_inst->GetInputsCount() - (need_save_state ? 0 : 1);

    inst->ReserveInputs(inputs_count);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), inputs_count);

    auto inputs = call_inst->GetInputs();
    for (size_t i = 0; i < inputs_count; ++i) {
        inst->AppendInput(inputs[i].GetInst());
        inst->AddInputType(call_inst->GetInputType(i));
    }

    auto method = ctx->method;
    if (ctx->cha_devirtualize) {
        InsertChaGuard(call_inst);
        GetCha()->AddDependency(method, GetGraph()->GetOutermostParentGraph()->GetMethod());
        GetGraph()->GetOutermostParentGraph()->AddSingleImplementationMethod(method);
    }

    call_inst->InsertAfter(inst);
    call_inst->ReplaceUsers(inst);
    LOG_INLINING(DEBUG) << "The method: " << GetMethodFullName(GetGraph(), method) << "replaced to the intrinsic"
                        << GetIntrinsicName(intrinsic_id);

    return true;
}

bool Inlining::DoInlineMethod(CallInst *call_inst, InlineContext *ctx)
{
    ASSERT(ctx->intrinsic_id == RuntimeInterface::IntrinsicId::INVALID);
    auto method = ctx->method;

    auto runtime = GetGraph()->GetRuntime();

    if (resolve_wo_inline_) {
        // Return, don't inline anything
        // At this point we:
        // 1. Gave a chance to inline external method
        // 2. Set replace_to_static to true where possible
        return false;
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

bool Inlining::DoInline(CallInst *call_inst, InlineContext *ctx)
{
    ASSERT(!call_inst->IsInlined());

    auto method = ctx->method;

    auto runtime = GetGraph()->GetRuntime();

    if (!CheckMethodCanBeInlined<false, true>(call_inst, ctx)) {
        return false;
    }
    if (runtime->IsMethodExternal(GetGraph()->GetMethod(), method) && !IsIntrinsic(ctx)) {
        if (!OPTIONS.IsCompilerInlineExternalMethods()) {
            // Skip external methods
            EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::SKIP_EXTERNAL);
            LOG_INLINING(DEBUG) << "We can't inline external method: " << GetMethodFullName(GetGraph(), ctx->method);
            return false;
        }
        if (GetGraph()->IsAotMode()) {
            return TryInlineExternal(call_inst, ctx);
        }
    }

    if (IsIntrinsic(ctx)) {
        return DoInlineIntrinsic(call_inst, ctx);
    }
    return DoInlineMethod(call_inst, ctx);
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
        // Do not remove function (first) input for dynamic calls
        auto save_state = call_inst->GetSaveState();
        ASSERT(save_state->GetOpcode() == Opcode::SaveState);
        if (call_inst->GetOpcode() != Opcode::CallDynamic) {
            auto nullcheck_inst = call_inst->GetObjectInst();
            call_inst->RemoveInputs();
            if (nullcheck_inst->GetOpcode() == Opcode::NullCheck) {
                call_inst->AppendInput(nullcheck_inst);
            }
        } else {
            auto func = call_inst->GetInput(0).GetInst();
            call_inst->RemoveInputs();
            call_inst->AppendInput(func);
        }
        call_inst->AppendInput(save_state);
        call_inst->SetType(DataType::VOID);
        for (auto bb : return_blocks_) {
            auto inlined_return =
                GetGraph()->CreateInstReturnInlined(DataType::VOID, INVALID_PC, call_inst->GetSaveState());
            if (bb != call_cont_bb && (bb->IsEndWithThrowOrDeoptimize() ||
                                       (bb->IsEmpty() && bb->GetPredsBlocks()[0]->IsEndWithThrowOrDeoptimize()))) {
                auto last_inst = !bb->IsEmpty() ? bb->GetLastInst() : bb->GetPredsBlocks()[0]->GetLastInst();
                last_inst->InsertBefore(inlined_return);
                inlined_return->SetExtendedLiveness();
            } else {
                bb->PrependInst(inlined_return);
            }
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

bool Inlining::CheckBytecode(CallInst *call_inst, const InlineContext &ctx, bool *callee_call_runtime)
{
    auto vregs_num = GetGraph()->GetRuntime()->GetMethodArgumentsCount(ctx.method) +
                     GetGraph()->GetRuntime()->GetMethodRegistersCount(ctx.method) + 1;
    if ((vregs_count_ + vregs_num) >= OPTIONS.GetCompilerMaxVregsNum()) {
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

    if (bytecode_analysis.HasRuntimeCalls() && OPTIONS.IsCompilerInlineSimpleOnly()) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::UNSUITABLE);
        return false;
    }
    if (callee_call_runtime != nullptr) {
        *callee_call_runtime = bytecode_analysis.HasRuntimeCalls();
    }
    return true;
}

bool Inlining::CheckInstructionLimit(CallInst *call_inst, InlineContext *ctx, size_t inlined_insts_count)
{
    // Don't inline if we reach the limit of instructions and method is big enough.
    if (inlined_insts_count > SMALL_METHOD_MAX_SIZE &&
        (instructions_count_ + inlined_insts_count) >= instructions_limit_) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::LIMIT);
        LOG_INLINING(DEBUG) << "Reached instructions limit: current_size=" << instructions_count_
                            << ", inlined_size=" << inlined_insts_count;
        ctx->replace_to_static = CanReplaceWithCallStatic(call_inst->GetOpcode());
        return false;
    }
    return true;
}

bool Inlining::TryBuildGraph(const InlineContext &ctx, Graph *graph_inl, CallInst *call_inst, CallInst *poly_call_inst)
{
    if (!graph_inl->RunPass<IrBuilder>(ctx.method, poly_call_inst != nullptr ? poly_call_inst : call_inst,
                                       depth_ + 1)) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::FAIL);
        LOG_INLINING(WARNING) << "Graph building failed";
        return false;
    }

    if (graph_inl->HasInfiniteLoop()) {
        EmitEvent(GetGraph(), call_inst, ctx, events::InlineResult::INF_LOOP);
        COMPILER_LOG(INFO, INLINING) << "Inlining of the methods with infinite loop is not supported";
        return false;
    }

    if (!OPTIONS.IsCompilerInliningSkipAlwaysThrowMethods()) {
        return true;
    }

    bool always_throw = true;
    // check that end block could be reached only through throw-blocks
    for (auto pred : graph_inl->GetEndBlock()->GetPredsBlocks()) {
        auto return_inst = pred->GetLastInst();
        if (return_inst == nullptr) {
            ASSERT(pred->IsTryEnd());
            ASSERT(pred->GetPredsBlocks().size() == 1);
            pred = pred->GetPredBlockByIndex(0);
        }
        if (!pred->IsEndWithThrowOrDeoptimize()) {
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
        if (OPTIONS.IsCompilerInlineSimpleOnly()) {
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
    auto index = call_inst->GetObjectIndex();
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
    if (!CheckBytecode(call_inst, *ctx, &callee_call_runtime)) {
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
    graph_inl->RunPass<Cleanup>(false);
    auto peephole_applied = graph_inl->RunPass<Peepholes>();
    auto object_type_applied = graph_inl->RunPass<ObjectTypeCheckElimination>();
    if (peephole_applied || object_type_applied) {
        graph_inl->RunPass<BranchElimination>();
        graph_inl->RunPass<Cleanup>();
    }

    // Don't inline if we reach the limit of instructions and method is big enough.
    auto inlined_insts_count = CalculateInstructionsCount(graph_inl);
    if (!CheckInstructionLimit(call_inst, ctx, inlined_insts_count)) {
        stats->SetPbcInstNum(saved_pbc_inst_num);
        return InlinedGraph();
    }

    if ((depth_ + 1) < OPTIONS.GetCompilerInliningMaxDepth()) {
        graph_inl->RunPass<Inlining>(instructions_count_ + inlined_insts_count, depth_ + 1, methods_inlined_ + 1);
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

template <bool CHECK_EXTERNAL, bool CHECK_INTRINSICS>
bool Inlining::CheckMethodCanBeInlined(const CallInst *call_inst, InlineContext *ctx)
{
    if (ctx->method == nullptr) {
        return false;
    }
    if constexpr (CHECK_EXTERNAL) {
        ASSERT(!GetGraph()->IsAotMode());
        if (!OPTIONS.IsCompilerInlineExternalMethods() &&
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
        if constexpr (CHECK_INTRINSICS) {
            if (GetGraph()->GetRuntime()->IsMethodIntrinsic(ctx->method)) {
                ctx->intrinsic_id = GetGraph()->GetRuntime()->GetIntrinsicId(ctx->method);
                return true;
            }
        }
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::UNSUITABLE);
        return false;
    }

    if (GetGraph()->GetRuntime()->GetMethodName(ctx->method).find("__noinline__") != std::string::npos) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::NOINLINE);
        return false;
    }

    bool method_is_too_big =
        GetGraph()->GetRuntime()->GetMethodCodeSize(ctx->method) >= OPTIONS.GetCompilerInliningMaxSize();
    if (method_is_too_big) {
        EmitEvent(GetGraph(), call_inst, *ctx, events::InlineResult::LIMIT);
        LOG_INLINING(DEBUG) << "Method is too big: " << GetMethodFullName(GetGraph(), ctx->method);
    }

    if (method_is_too_big || resolve_wo_inline_) {
        ctx->replace_to_static = CanReplaceWithCallStatic(call_inst->GetOpcode());
        if constexpr (!CHECK_EXTERNAL) {
            if (GetGraph()->GetRuntime()->IsMethodExternal(GetGraph()->GetMethod(), ctx->method)) {
                // Do not replace to call static if --compiler-inline-external-methods=false
                ctx->replace_to_static &= OPTIONS.IsCompilerInlineExternalMethods();
                ASSERT(ctx->method != nullptr);
                // Allow to replace CallVirtual with CallStatic if the resolved method is same as the called method
                // In AOT mode the resolved method id can be different from the method id in the call_inst,
                // but we'll keep the method id from the call_inst because the resolved method id can be not correct
                // for aot compiled method
                ctx->replace_to_static &= ctx->method == call_inst->GetCallMethod()
                                          // Or if it's not aot mode. That is, just replace in other modes
                                          || !GetGraph()->IsAotMode();
            }
        }
        if (method_is_too_big) {
            return false;
        }
        ASSERT(resolve_wo_inline_);
        // Continue and return true to give a change to TryInlineExternalAot
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

/// Embed inlined dataflow graph into the caller graph.
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
        for (auto pred : end_block->GetPredsBlocks()) {
            auto return_inst = pred->GetLastInst();
            if (return_inst == nullptr) {
                ASSERT(pred->IsTryEnd());
                ASSERT(pred->GetPredsBlocks().size() == 1);
                pred = pred->GetPredBlockByIndex(0);
                return_inst = pred->GetLastInst();
            }
            if (pred->IsEndWithThrowOrDeoptimize()) {
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
               pred_block->IsEndWithThrowOrDeoptimize());
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
        if (!pred_block->IsEndWithThrowOrDeoptimize()) {
            pred_block->RemoveInst(return_inst);
        }
    }
}

/// Embed inlined controlflow graph into the caller graph.
void Inlining::UpdateControlflow(Graph *graph_inl, BasicBlock *call_bb, BasicBlock *call_cont_bb)
{
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
    auto start_bb = graph_inl->GetStartBlock();
    for (auto succ = start_bb->GetSuccsBlocks().back(); !start_bb->GetSuccsBlocks().empty();
         succ = start_bb->GetSuccsBlocks().back()) {
        succ->ReplacePred(start_bb, call_bb);
        start_bb->GetSuccsBlocks().pop_back();
    }

    ASSERT(graph_inl->HasEndBlock());
    auto end_block = graph_inl->GetEndBlock();
    for (auto pred : end_block->GetPredsBlocks()) {
        end_block->RemovePred(pred);
        if (pred->IsEndWithThrowOrDeoptimize() ||
            (pred->IsEmpty() && pred->GetPredsBlocks()[0]->IsEndWithThrowOrDeoptimize())) {
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

    if (OPTIONS.IsCompilerNoVirtualInlining()) {
        return false;
    }

    // If class or method are final we can resolve the method
    if (runtime->IsMethodFinal(method) || runtime->IsClassFinal(runtime->GetClass(method))) {
        ctx->method = method;
        return true;
    }

    auto object_inst = call_inst->GetDataFlowInput(call_inst->GetObjectIndex());
    auto type_info = object_inst->GetObjectTypeInfo();
    if (CanUseTypeInfo(type_info, method)) {
        auto receiver = type_info.GetClass();
        MethodPtr resolved_method;
        if (runtime->IsInterfaceMethod(method)) {
            resolved_method = runtime->ResolveInterfaceMethod(receiver, method);
        } else {
            resolved_method = runtime->ResolveVirtualMethod(receiver, method);
        }
        if (resolved_method != nullptr && (type_info.IsExact() || runtime->IsMethodFinal(resolved_method))) {
            ctx->method = resolved_method;
            return true;
        }
        if (type_info.IsExact()) {
            LOG_INLINING(WARNING) << "Runtime failed to resolve method";
            return false;
        }
    }

    if (ArchTraits<RUNTIME_ARCH>::SUPPORT_DEOPTIMIZATION && !OPTIONS.IsCompilerNoChaInlining() &&
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

bool Inlining::CanUseTypeInfo(ObjectTypeInfo type_info, RuntimeInterface::MethodPtr method)
{
    auto runtime = GetGraph()->GetRuntime();
    if (!type_info || runtime->IsInterface(type_info.GetClass())) {
        return false;
    }
    if (type_info.IsExact()) {
        return true;
    }
    return runtime->IsAssignableFrom(runtime->GetClass(method), type_info.GetClass());
}

void Inlining::InsertChaGuard(CallInst *call_inst)
{
    auto save_state = call_inst->GetSaveState();
    auto check_deopt = GetGraph()->CreateInstIsMustDeoptimize(DataType::BOOL, call_inst->GetPc());
    auto deopt = GetGraph()->CreateInstDeoptimizeIf(DataType::NO_TYPE, call_inst->GetPc(), check_deopt, save_state,
                                                    DeoptimizeType::INLINE_CHA);
    call_inst->InsertBefore(deopt);
    deopt->InsertBefore(check_deopt);
}

bool Inlining::SkipBlock(const BasicBlock *block) const
{
    if (block == nullptr || block->IsEmpty()) {
        return true;
    }
    if (!OPTIONS.IsCompilerInliningSkipThrowBlocks() || (GetGraph()->GetThrowCounter(block) > 0)) {
        return false;
    }
    return block->IsEndWithThrowOrDeoptimize();
}
}  // namespace panda::compiler
