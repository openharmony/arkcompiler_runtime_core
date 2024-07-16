/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::compiler {

inline void Codegen::PrepareForCallLaunchVirtual(CallInst *callInst, RuntimeInterface::MethodPtr method, Reg &thisReg,
                                                 Reg &param0)
{
    thisReg = ConvertRegister(callInst->GetLocation(1).GetValue(), DataType::REFERENCE);
    LoadClassFromObject(param0, thisReg);
    // Get index
    auto vtableIndex = GetRuntime()->GetVTableIndex(method);
    // Load from VTable, address = klass + ((index << shift) + vtable_offset)
    auto totalOffset = GetRuntime()->GetVTableOffset(GetArch()) + (vtableIndex << GetVtableShift());
    // Class ref was loaded to method_reg
    GetEncoder()->EncodeLdr(param0, false, MemRef(param0, totalOffset));
}

bool Codegen::LaunchCallCodegen(CallInst *callInst)
{
    SCOPED_DISASM_STR(this, "Create Launch Call");

    auto dstReg = ConvertRegister(callInst->GetDstReg(), callInst->GetType());

    Reg param0 = GetTarget().GetParamReg(0);
    ScopedTmpRegLazy tmp(GetEncoder());

    RegMask liveRegs {GetLiveRegisters(callInst).first};
    if (liveRegs.Test(param0.GetId())) {
        tmp.Acquire();
        param0 = tmp;
    }

    Reg objReg;
    Reg thisReg;
    if (callInst->GetOpcode() == Opcode::CallResolvedLaunchStatic ||
        callInst->GetOpcode() == Opcode::CallResolvedLaunchVirtual) {
        auto location = callInst->GetLocation(0);
        ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

        param0 = ConvertRegister(location.GetValue(), DataType::POINTER);
        auto location1 = callInst->GetLocation(1);
        ASSERT(location1.IsFixedRegister() && location1.IsRegisterValid());

        objReg = ConvertRegister(location1.GetValue(), DataType::REFERENCE);
        if (callInst->GetOpcode() == Opcode::CallResolvedLaunchVirtual) {
            thisReg = ConvertRegister(callInst->GetLocation(2U).GetValue(), DataType::REFERENCE);
        }
    } else {
        auto location = callInst->GetLocation(0);
        ASSERT(location.IsFixedRegister() && location.IsRegisterValid());

        objReg = ConvertRegister(location.GetValue(), DataType::REFERENCE);

        auto method = callInst->GetCallMethod();
        if (callInst->GetOpcode() == Opcode::CallLaunchStatic) {
            ASSERT(!GetGraph()->IsAotMode());
            GetEncoder()->EncodeMov(param0, Imm(reinterpret_cast<size_t>(method)));
        } else {
            ASSERT(callInst->GetOpcode() == Opcode::CallLaunchVirtual);
            PrepareForCallLaunchVirtual(callInst, method, thisReg, param0);
        }
    }

    if (callInst->IsStaticLaunchCall()) {
        CallRuntime(callInst, EntrypointId::CREATE_LAUNCH_STATIC_COROUTINE, dstReg, RegMask::GetZeroMask(), param0,
                    objReg, SpReg());
    } else {
        CallRuntime(callInst, EntrypointId::CREATE_LAUNCH_VIRTUAL_COROUTINE, dstReg, RegMask::GetZeroMask(), param0,
                    objReg, SpReg(), thisReg);
    }
    if (callInst->GetFlag(inst_flags::MEM_BARRIER)) {
        GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
    return true;
}
}  // namespace ark::compiler
