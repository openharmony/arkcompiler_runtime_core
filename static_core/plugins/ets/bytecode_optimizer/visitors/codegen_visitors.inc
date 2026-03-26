/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

static void VisitLoadUniqueObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    if (inst->GetDstReg() == compiler::GetAccReg()) {
        enc->result_.emplace_back(pandasm::Create_ETS_LDNULLVALUE());
    } else {
        enc->result_.emplace_back(pandasm::Create_ETS_MOVNULLVALUE(inst->GetDstReg()));
    }
}

template <bool IS_STRICT>
static void VisitEtsEquals(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto v0 = inst->GetSrcReg(0);
    auto v1 = inst->GetSrcReg(1);
    if constexpr (IS_STRICT) {
        enc->result_.emplace_back(pandasm::Create_ETS_STRICTEQUALS(v0, v1));
    } else {
        enc->result_.emplace_back(pandasm::Create_ETS_EQUALS(v0, v1));
    }
    DoSta(inst->GetDstReg(), enc->result_);
}

static void VisitEtsTypeof(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto v0 = inst->GetSrcReg(0);
    enc->result_.emplace_back(pandasm::Create_ETS_TYPEOF(v0));
    DoStaObj(inst->GetDstReg(), enc->result_);
}

static void VisitEtsIstrue(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto v = inst->GetSrcReg(0);
    enc->result_.emplace_back(pandasm::Create_ETS_ISTRUE(v));
    DoSta(inst->GetDstReg(), enc->result_);
}

static void VisitEtsNullcheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    DoLdaObj(inst->GetSrcReg(0), enc->result_);
    enc->result_.emplace_back(pandasm::Create_ETS_NULLCHECK());
}