/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_CODEGEN_LIB_CALL_INST_H
#define COMPILER_OPTIMIZER_CODEGEN_LIB_CALL_INST_H

#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/inst.h"

namespace panda::compiler {
inline bool HasLibCall(Inst *inst, Arch arch)
{
    auto opcode = inst->GetOpcode();
    auto type = inst->GetType();
    switch (arch) {
        case Arch::X86_64:
        case Arch::AARCH64: {
            if (opcode == Opcode::Mod) {
                return DataType::IsFloatType(type);
            }
            return false;
        }
        case Arch::AARCH32: {
            if (opcode == Opcode::Mod) {
                return true;
            }
            if (opcode == Opcode::Div) {
                return type == DataType::INT64 || type == DataType::UINT64;
            }
            if (opcode == Opcode::Cast) {
                auto src_type = inst->GetInputType(0);
                if (DataType::IsFloatType(type)) {
                    return src_type == DataType::INT64 || src_type == DataType::UINT64;
                }
                if (DataType::IsFloatType(src_type)) {
                    return type == DataType::INT64 || type == DataType::UINT64;
                }
            }
            return false;
        }
        default:
            UNREACHABLE();
    }
}
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_LIB_CALL_INST_H
