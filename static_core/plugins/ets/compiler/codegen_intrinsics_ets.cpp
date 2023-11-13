/**
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
#include "operands.h"
#include "codegen.h"

namespace panda::compiler {

void Codegen::CreateMathTrunc([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeTrunc(dst, src[0]);
}

void Codegen::CreateMathRoundAway([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeRoundAway(dst, src[0]);
}

void Codegen::EtsLdundefined([[maybe_unused]] IntrinsicInst *inst, Reg dst, [[maybe_unused]] SRCREGS src)
{
    if (GetGraph()->IsJitOrOsrMode()) {
        GetEncoder()->EncodeMov(dst, Imm(GetRuntime()->GetUndefinedObject()));
    } else {
        auto ref = MemRef(ThreadReg(), cross_values::GetEtsCoroutineUndefinedObjectOffset(GetArch()));
        GetEncoder()->EncodeLdr(dst, false, ref);
    }
}

}  // namespace panda::compiler
