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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H_

#include <string>
#include "optimizer/pass.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/runtime_interface.h"
#include "compiler_options.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
struct InlineContext {
    RuntimeInterface::MethodPtr method {};
    bool cha_devirtualize {false};
    bool replace_to_static {false};
};

struct InlinedGraph {
    Graph *graph {nullptr};
    bool has_runtime_calls {false};
};

class Inlining : public Optimization {
    static constexpr size_t SMALL_METHOD_MAX_SIZE = 5;

public:
    explicit Inlining(Graph *graph) : Inlining(graph, 0, 0, 0) {}

    Inlining(Graph *graph, uint32_t instructions_count, uint32_t inline_depth, uint32_t methods_inlined);

    NO_MOVE_SEMANTIC(Inlining);
    NO_COPY_SEMANTIC(Inlining);
    ~Inlining() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return options.IsCompilerInlining();
    }

    const char *GetPassName() const override
    {
        return "Inline";
    }

    void InvalidateAnalyses() override;

private:
    bool TryInline(CallInst *call_inst);
    bool TryInlineWithInlineCaches(CallInst *call_inst);
    bool TryInlineExternal(CallInst *call_inst, InlineContext *ctx);
    bool TryInlineExternalAot(CallInst *call_inst, InlineContext *ctx);

    Inst *GetNewDefAndCorrectDF(Inst *call_inst, Inst *old_def);

    bool DoInline(CallInst *call_inst, InlineContext *ctx);
    bool DoInlineMonomorphic(CallInst *call_inst, RuntimeInterface::ClassPtr receiver);
    bool DoInlinePolymorphic(CallInst *call_inst);
    void CreateCompareClass(CallInst *call_inst, Inst *get_cls_inst, RuntimeInterface::ClassPtr receiver,
                            BasicBlock *call_bb);
    void InsertDeoptimizeInst(CallInst *call_inst, BasicBlock *call_bb);
    void InsertPolymorphicGraph(InlinedGraph inl_graph, BasicBlock *call_bb, PhiInst *phi_inst, CallInst *call_inst,
                                CallInst *new_call_inst);

    void UpdateDataflow(Graph *graph_inl, Inst *call_inst, std::variant<BasicBlock *, PhiInst *> use,
                        Inst *new_def = nullptr);
    void UpdateControlflow(Graph *graph_inl, BasicBlock *call_bb, BasicBlock *call_cont_bb);
    void MoveConstants(Graph *graph_inl);

    template <bool check_external>
    bool CheckMethodCanBeInlined(const CallInst *call_inst, InlineContext *ctx);
    bool ResolveTarget(CallInst *call_inst, InlineContext *ctx);
    void InsertChaGuard(CallInst *call_inst);

    InlinedGraph BuildGraph(InlineContext *ctx, CallInst *call_inst, CallInst *poly_call_inst = nullptr);
    bool CheckBytecode(const InlineContext &ctx, bool *callee_call_runtime, CallInst *call_inst);
    bool CheckInsrtuctionLimitAndCallInline(InlineContext *ctx, Graph *graph_inl, CallInst *call_inst);
    bool TryBuildGraph(const InlineContext &ctx, Graph *graph_inl, CallInst *call_inst, CallInst *poly_call_inst);
    bool CheckLoops(bool *callee_call_runtime, Graph *graph_inl);
    static void PropagateObjectInfo(Graph *graph_inl, CallInst *call_inst);

    void ProcessCallReturnInstructions(CallInst *call_inst, BasicBlock *call_cont_bb, bool has_runtime_calls,
                                       bool need_barriers = false);

    IClassHierarchyAnalysis *GetCha()
    {
        return cha_;
    }

    bool IsInlineCachesEnabled() const;

    std::string GetLogIndent() const
    {
        return std::string(depth_ * 2, ' ');
    }

    bool SkipBlock(const BasicBlock *block) const;

private:
    IClassHierarchyAnalysis *cha_ {nullptr};
    ArenaUnorderedSet<std::string> blacklist_;
    ArenaVector<BasicBlock *> return_blocks_;
    ArenaVector<RuntimeInterface::ClassPtr> recievers_;
    uint32_t instructions_count_ {0};
    uint32_t depth_ {0};
    uint32_t methods_inlined_ {0};
    uint32_t vregs_count_ {0};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_INLINING_H_
