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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_TYPES_RESOLVING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_TYPES_RESOLVING_H

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace panda::compiler {
/*
 * TypesResolving(based on the results of the TypesAnalysis) tries to replace dynamic intrinsics to static instructions
 * TypesResolving collects assumed types(dynamic type) of intrinsics inputs:
 *  - If all inputs have undefined assumes type, the optimization isn't applied.
 *  - If some inputs have undefined assumed type they are assigned the assumed type of another input.
 * Thus each input has an assumed type.
 * Next, optimization tries to replace the intrinsic with static instructions, taking into account the types of inputs.
 * In case of success, the corresponding types are put in AnyType heck, which are the inputs of the intrinsic
 */
class TypesResolving : public Optimization {
public:
    explicit TypesResolving(Graph *graph);
    NO_MOVE_SEMANTIC(TypesResolving);
    NO_COPY_SEMANTIC(TypesResolving);
    ~TypesResolving() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "TypesResolving";
    }
    void InvalidateAnalyses() override;
#include "intrinsics_types_resolving.inl.h"

private:
    AnyBaseType GetAssumedAnyType(Inst *inst);
    bool TryInline(IntrinsicInst *intrinsic);
    bool TryResolvePhi();
    bool CheckInputsAnyTypesRec(Inst *phi);
    void PropagateTypeToPhi();
    bool DoInline(IntrinsicInst *intrinsic);
    ArenaVector<AnyBaseType> types_;
    ArenaVector<Inst *> phis_;
    AnyBaseType any_type_ {AnyBaseType::UNDEFINED_TYPE};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_TYPES_RESOLVING_H
