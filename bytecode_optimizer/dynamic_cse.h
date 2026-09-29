/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef BYTECODE_OPTIMIZER_DYNAMIC_CSE_H
#define BYTECODE_OPTIMIZER_DYNAMIC_CSE_H

#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/pass.h"

namespace panda::bytecodeopt {

class DynamicCse final : public compiler::Optimization {
public:
    explicit DynamicCse(compiler::Graph *graph) : Optimization(graph) {}
    NO_COPY_SEMANTIC(DynamicCse);
    NO_MOVE_SEMANTIC(DynamicCse);
    ~DynamicCse() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "DynamicCse";
    }

    bool IsEnable() const override
    {
        return true;
    }
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_DYNAMIC_CSE_H
