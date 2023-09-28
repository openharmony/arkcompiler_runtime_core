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

#include "compiler/optimizer/code_generator/codegen.h"

namespace panda::compiler {

bool Codegen::LaunchCallCodegen(CallInst *call_inst)
{
    SCOPED_DISASM_STR(this, "Create Launch Call");

    auto dst_reg = ConvertRegister(call_inst->GetDstReg(), call_inst->GetType());

    Reg param_0 = GetTarget().GetParamReg(0);
    ScopedTmpRegLazy tmp(GetEncoder());

    RegMask live_regs {GetLiveRegisters(call_inst).first};
    if (live_regs.Test(param_0.GetId())) {
        tmp.Acquire();
        param_0 = tmp;
    }

    Reg obj_reg;
    Reg this_reg;
    if (call_inst->GetOpcode() == Opcode::CallResolvedLaunchStatic ||
        call_inst->GetOpcode() == Opcode::CallResolvedLaunchVirtual) {
        auto location = call_inst->GetLocation(0);
        ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

        param_0 = ConvertRegister(location.GetValue(), DataType::POINTER);
        auto location1 = call_inst->GetLocation(1);
        ASSERT(location1.IsFixedRegister() && location1.IsRegisterValid());

        obj_reg = ConvertRegister(location1.GetValue(), DataType::REFERENCE);
        if (call_inst->GetOpcode() == Opcode::CallResolvedLaunchVirtual) {
            this_reg = ConvertRegister(call_inst->GetLocation(2).GetValue(), DataType::REFERENCE);
        }
    } else {
        auto location = call_inst->GetLocation(0);
        ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

        obj_reg = ConvertRegister(location.GetValue(), DataType::REFERENCE);

        auto method = call_inst->GetCallMethod();
        if (call_inst->GetOpcode() == Opcode::CallLaunchStatic) {
            ASSERT(!GetGraph()->IsAotMode());
            GetEncoder()->EncodeMov(param_0, Imm(reinterpret_cast<size_t>(method)));
        } else {
            ASSERT(call_inst->GetOpcode() == Opcode::CallLaunchVirtual);
            this_reg = ConvertRegister(call_inst->GetLocation(1).GetValue(), DataType::REFERENCE);
            LoadClassFromObject(param_0, this_reg);
            // Get index
            auto vtable_index = GetRuntime()->GetVTableIndex(method);
            // Load from VTable, address = klass + ((index << shift) + vtable_offset)
            auto total_offset = GetRuntime()->GetVTableOffset(GetArch()) + (vtable_index << GetVtableShift());
            // Class ref was loaded to method_reg
            GetEncoder()->EncodeLdr(param_0, false, MemRef(param_0, total_offset));
        }
    }

    if (call_inst->IsStaticLaunchCall()) {
        CallRuntime(call_inst, EntrypointId::CREATE_LAUNCH_STATIC_COROUTINE, dst_reg, RegMask::GetZeroMask(), param_0,
                    obj_reg, SpReg());
    } else {
        CallRuntime(call_inst, EntrypointId::CREATE_LAUNCH_VIRTUAL_COROUTINE, dst_reg, RegMask::GetZeroMask(), param_0,
                    obj_reg, SpReg(), this_reg);
    }
    if (call_inst->GetFlag(inst_flags::MEM_BARRIER)) {
        GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
    return true;
}
}  // namespace panda::compiler
