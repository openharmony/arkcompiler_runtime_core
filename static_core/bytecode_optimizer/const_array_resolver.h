/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_CONST_ARRAY_RESOLVER_H
#define PANDA_CONST_ARRAY_RESOLVER_H

#include "assembler/assembly-function.h"
#include "bytecodeopt_options.h"
#include "ir_interface.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/optimizations/const_folding.h"

namespace panda::bytecodeopt {
using panda::compiler::Inst;
using panda::compiler::Opcode;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda::compiler::DataType;
class ConstArrayResolver : public compiler::Optimization {
public:
    explicit ConstArrayResolver(compiler::Graph *graph, BytecodeOptIrInterface *iface)
        : compiler::Optimization(graph),
          ir_interface_(iface),
          const_arrays_init_(graph->GetLocalAllocator()->Adapter()),
          const_arrays_fill_(graph->GetLocalAllocator()->Adapter())
    {
    }

    ~ConstArrayResolver() override = default;
    NO_COPY_SEMANTIC(ConstArrayResolver);
    NO_MOVE_SEMANTIC(ConstArrayResolver);

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "ConstArrayResolver";
    }

    bool IsEnable() const override
    {
        return OPTIONS.IsConstArrayResolver();
    }

private:
    bool FindConstantArrays();
    void RemoveArraysFill();
    void InsertLoadConstArrayInsts();
    std::optional<std::vector<pandasm::LiteralArray::Literal>> FillLiteralArray(Inst *inst, size_t size);
    bool FillLiteral(compiler::StoreInst *store_array_inst, pandasm::LiteralArray::Literal *literal);
    void AddIntroLiterals(pandasm::LiteralArray *lt_ar);
    bool IsMultidimensionalArray(compiler::NewArrayInst *inst);

private:
    BytecodeOptIrInterface *ir_interface_ {nullptr};
    ArenaMap<uint32_t, compiler::NewArrayInst *> const_arrays_init_;  // const_arrays_[literalarray_id] = new_array_inst
    ArenaMap<Inst *, std::vector<Inst *>>
        const_arrays_fill_;  // const_arrays_[new_array_inst] = {store_array_inst_1, ... , store_array_inst_n}
};
}  // namespace panda::bytecodeopt

#endif  // PANDA_CONST_ARRAY_RESOLVER_H
