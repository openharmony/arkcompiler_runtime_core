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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H

#include <string>
#include "optimizer/pass.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/runtime_interface.h"
#include "compiler_options.h"
#include "utils/arena_containers.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage,-warnings-as-errors)
#define LOG_INLINING(level) COMPILER_LOG(level, INLINING) << GetLogIndent()

namespace panda::compiler {
struct InlineContext {
    RuntimeInterface::MethodPtr method {};
    bool cha_devirtualize {false};
    bool replace_to_static {false};
    RuntimeInterface::IntrinsicId intrinsic_id {RuntimeInterface::IntrinsicId::INVALID};
};

struct InlinedGraph {
    Graph *graph {nullptr};
    bool has_runtime_calls {false};
};

class Inlining : public Optimization {
    static constexpr size_t SMALL_METHOD_MAX_SIZE = 5;

public:
    explicit Inlining(Graph *graph) : Inlining(graph, 0, 0, 0) {}
    Inlining(Graph *graph, bool resolve_wo_inline) : Inlining(graph)
    {
        resolve_wo_inline_ = resolve_wo_inline;
    }

    Inlining(Graph *graph, uint32_t instructions_count, uint32_t inline_depth, uint32_t methods_inlined);

    NO_MOVE_SEMANTIC(Inlining);
    NO_COPY_SEMANTIC(Inlining);
    ~Inlining() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return OPTIONS.IsCompilerInlining();
    }

    const char *GetPassName() const override
    {
        return "Inline";
    }

    void InvalidateAnalyses() override;

protected:
    virtual void RunOptimizations() const;
    virtual bool IsInstSuitableForInline(Inst *inst) const;
    virtual bool TryInline(CallInst *call_inst);
    bool TryInlineWithInlineCaches(CallInst *call_inst);
    bool TryInlineExternal(CallInst *call_inst, InlineContext *ctx);
    bool TryInlineExternalAot(CallInst *call_inst, InlineContext *ctx);

    Inst *GetNewDefAndCorrectDF(Inst *call_inst, Inst *old_def);

    bool Do();
    bool DoInline(CallInst *call_inst, InlineContext *ctx);
    bool DoInlineMethod(CallInst *call_inst, InlineContext *ctx);
    bool DoInlineIntrinsic(CallInst *call_inst, InlineContext *ctx);
    bool DoInlineMonomorphic(CallInst *call_inst, RuntimeInterface::ClassPtr receiver);
    bool DoInlinePolymorphic(CallInst *call_inst, ArenaVector<RuntimeInterface::ClassPtr> *receivers);
    void CreateCompareClass(CallInst *call_inst, Inst *get_cls_inst, RuntimeInterface::ClassPtr receiver,
                            BasicBlock *call_bb);
    void InsertDeoptimizeInst(CallInst *call_inst, BasicBlock *call_bb,
                              DeoptimizeType deopt_type = DeoptimizeType::INLINE_IC);
    void InsertCallInst(CallInst *call_inst, BasicBlock *call_bb, BasicBlock *ret_bb, Inst *phi_inst);

    void UpdateDataflow(Graph *graph_inl, Inst *call_inst, std::variant<BasicBlock *, PhiInst *> use,
                        Inst *new_def = nullptr);
    void UpdateDataflowForEmptyGraph(Inst *call_inst, std::variant<BasicBlock *, PhiInst *> use, BasicBlock *end_block);
    void UpdateParameterDataflow(Graph *graph_inl, Inst *call_inst);
    void UpdateControlflow(Graph *graph_inl, BasicBlock *call_bb, BasicBlock *call_cont_bb);
    void MoveConstants(Graph *graph_inl);

    template <bool CHECK_EXTERNAL, bool CHECK_INTRINSICS = false>
    bool CheckMethodCanBeInlined(const CallInst *call_inst, InlineContext *ctx);
    template <bool CHECK_EXTERNAL>
    bool CheckTooBigMethodCanBeInlined(const CallInst *call_inst, InlineContext *ctx, bool method_is_too_big);
    bool ResolveTarget(CallInst *call_inst, InlineContext *ctx);
    bool CanUseTypeInfo(ObjectTypeInfo type_info, RuntimeInterface::MethodPtr method);
    void InsertChaGuard(CallInst *call_inst);

    InlinedGraph BuildGraph(InlineContext *ctx, CallInst *call_inst, CallInst *poly_call_inst = nullptr);
    bool CheckBytecode(CallInst *call_inst, const InlineContext &ctx, bool *callee_call_runtime);
    bool CheckInstructionLimit(CallInst *call_inst, InlineContext *ctx, size_t inlined_insts_count);
    bool TryBuildGraph(const InlineContext &ctx, Graph *graph_inl, CallInst *call_inst, CallInst *poly_call_inst);
    bool CheckLoops(bool *callee_call_runtime, Graph *graph_inl);
    static void PropagateObjectInfo(Graph *graph_inl, CallInst *call_inst);

    void ProcessCallReturnInstructions(CallInst *call_inst, BasicBlock *call_cont_bb, bool has_runtime_calls,
                                       bool need_barriers = false);
    size_t CalculateInstructionsCount(Graph *graph);

    IClassHierarchyAnalysis *GetCha()
    {
        return cha_;
    }

    bool IsInlineCachesEnabled() const;

    std::string GetLogIndent() const
    {
        return std::string(depth_ * 2, ' ');
    }

    bool IsIntrinsic(const InlineContext *ctx) const
    {
        return ctx->intrinsic_id != RuntimeInterface::IntrinsicId::INVALID;
    }

    virtual bool SkipBlock(const BasicBlock *block) const;

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    uint32_t depth_ {0};
    uint32_t methods_inlined_ {0};
    uint32_t instructions_count_ {0};
    uint32_t instructions_limit_ {0};
    ArenaVector<BasicBlock *> return_blocks_;
    ArenaUnorderedSet<std::string> blacklist_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    uint32_t vregs_count_ {0};
    bool resolve_wo_inline_ {false};
    IClassHierarchyAnalysis *cha_ {nullptr};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H
