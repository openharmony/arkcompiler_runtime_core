/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_INSTRUCTION_IMPL_H
#define CPP_ABCKIT_INSTRUCTION_IMPL_H

#include "./instruction.h"
#include "./core/function.h"
#include "./core/import_descriptor.h"

namespace abckit {

inline const Instruction &Instruction::InsertAfter(const Instruction &inst) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iInsertAfter(GetView(), inst.GetView());
    CheckError(conf);
    return *this;
}

inline const Instruction &Instruction::InsertBefore(const Instruction &inst) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iInsertBefore(GetView(), inst.GetView());
    CheckError(conf);
    return *this;
}

inline void Instruction::Remove() const
{
    GetApiConfig()->cGapi_->iRemove(GetView());
    CheckError(GetApiConfig());
}

inline AbckitIsaApiDynamicOpcode Instruction::GetOpcodeDyn() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitIsaApiDynamicOpcode opc = conf->cDynapi_->iGetOpcode(GetView());
    CheckError(conf);
    return opc;
}

inline AbckitIsaApiStaticOpcode Instruction::GetOpcodeStat() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitIsaApiStaticOpcode opc = conf->cStatapi_->iGetOpcode(GetView());
    CheckError(conf);
    return opc;
}

inline AbckitIsaApiDynamicConditionCode Instruction::GetConditionCodeDyn() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitIsaApiDynamicConditionCode dcc = conf->cDynapi_->iGetConditionCode(GetView());
    CheckError(conf);
    return dcc;
}

inline int64_t Instruction::GetConstantValueI64() const
{
    int64_t ret = GetApiConfig()->cGapi_->iGetConstantValueI64(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline std::string_view Instruction::GetString() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cGapi_->iGetString(GetView());
    CheckError(conf);
    std::string_view view = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return view;
}

inline Instruction Instruction::GetNext() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitInst *inst = conf->cGapi_->iGetNext(GetView());
    CheckError(conf);
    return Instruction(inst, conf, GetResource());
}

inline Instruction Instruction::GetPrev() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitInst *inst = conf->cGapi_->iGetPrev(GetView());
    CheckError(conf);
    return Instruction(inst, conf, GetResource());
}

inline core::Function Instruction::GetFunction() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreFunction *func = conf->cGapi_->iGetFunction(GetView());
    CheckError(conf);
    return core::Function(func, conf, GetResource()->GetFile());
}

inline BasicBlock Instruction::GetBasicBlock() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->iGetBasicBlock(GetView());
    CheckError(conf);
    return BasicBlock(bb, conf, GetResource());
}

inline uint32_t Instruction::GetInputCount() const
{
    const ApiConfig *conf = GetApiConfig();
    uint32_t count = conf->cGapi_->iGetInputCount(GetView());
    CheckError(conf);
    return count;
}

inline Instruction Instruction::GetInput(uint32_t index) const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitInst *inInst = conf->cGapi_->iGetInput(GetView(), index);
    CheckError(conf);
    return Instruction(inInst, conf, GetResource());
}

inline void Instruction::SetInput(uint32_t index, const Instruction &input) const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iSetInput(GetView(), input.GetView(), index);
    CheckError(conf);
}

inline bool Instruction::VisitUsers(const std::function<bool(Instruction)> &cb) const
{
    Payload<const std::function<bool(Instruction)> &> payload {cb, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cGapi_->iVisitUsers(GetView(), &payload, [](AbckitInst *user, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<bool(Instruction)> &> *>(data);
        if (!payload.data(Instruction(user, payload.config, payload.resource))) {
            return false;
        };
        return true;
    });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline uint32_t Instruction::GetUserCount() const
{
    uint32_t count = GetApiConfig()->cGapi_->iGetUserCount(GetView());
    CheckError(GetApiConfig());
    return count;
}

inline core::ImportDescriptor Instruction::GetImportDescriptorDyn() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreImportDescriptor *id = conf->cDynapi_->iGetImportDescriptor(GetView());
    CheckError(conf);
    return core::ImportDescriptor(id, conf, GetResource()->GetFile());
}

}  // namespace abckit

#endif  // CPP_ABCKIT_INSTRUCTION_IMPL_H
