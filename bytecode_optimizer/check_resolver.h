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

#ifndef PANDA_CHECK_RESOLVER_H
#define PANDA_CHECK_RESOLVER_H

#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/pass.h"
#include "utils/arena_containers.h"

/*
 * Check Resolver.
 *
 * Check Resolver is a bytecodeopt-specific pass. In bytecode optimizer, we do
 * not need all check-instructions, such as BoundsCheck, NullCheck, NegativeCheck
 * and ZeroCheck, because the throws of these checks will be embedded in their
 * underlying instructions during runtime.
 * For the sake of saving ROM size, we can delete these check-instructions. Besides,
 * when bytecode optimizer optimizes the code in which there exist operations on the
 * element of an array, in the generated code the redundant asm lenarr will be generated
 * with ldarr and starr. Here lenarr is generated from IR LenArray and LenArray is an
 * input of BoundsCheck. CodeGen will encode LenArray but ignore BoundsCheck. That is
 * why dead lenarr remains. So we can also benefit from the size of generated bytecode
 * by deleting check-instructions.
 * However, these LenArray that are generated from asm lenarr should be keeped, as they
 * may throw in the original code logic. For LoadArray, Div, Mod, LoadStatic and LoadObject,
 * since they can throw but they are not generated as inputs of check-instructions, We
 * should keep all such insts.
 *
 * For every check-instruction, we replace the corresponding input of its users by the data
 * flow input. Then we clear its NO_DCE flag such that it can be removed by DCE pass. We set
 * the NO_DCE flag for the insts that should be keeped.
 */
namespace panda::bytecodeopt {

class CheckResolver : public compiler::Optimization {
public:
    explicit CheckResolver(compiler::Graph *graph) : compiler::Optimization(graph) {}
    ~CheckResolver() override = default;
    bool RunImpl() override;
    static bool IsCheck(const compiler::Inst *inst);
    static bool NeedSetNoDCE(const compiler::Inst *inst);
    const char *GetPassName() const override
    {
        return "CheckResolver";
    }
};

}  // namespace panda::bytecodeopt

#endif  // PANDA_CHECK_RESOLVER_H
