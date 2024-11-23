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

namespace abckit {

inline Instruction &Instruction::InsertAfter(const Instruction &inst)
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iInsertAfter(GetView(), inst.GetView());
    CheckError(conf);
    return *this;
}

inline Instruction &Instruction::InsertBefore(const Instruction &inst)
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iInsertBefore(GetView(), inst.GetView());
    CheckError(conf);
    return *this;
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
    return Instruction(inst, conf);
}

inline Instruction Instruction::GetPrev() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitInst *inst = conf->cGapi_->iGetPrev(GetView());
    CheckError(conf);
    return Instruction(inst, conf);
}

inline core::Function Instruction::GetFunction() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreFunction *func = conf->cGapi_->iGetFunction(GetView());
    CheckError(conf);
    return core::Function(func, conf);
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
    return Instruction(inInst, conf);
}

inline void Instruction::SetInput(uint32_t index, const Instruction &input)
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->iSetInput(GetView(), input.GetView(), index);
    CheckError(conf);
}

inline void Instruction::VisitUsers(const std::function<void(Instruction)> &cb) const
{
    struct Payload {
        const std::function<void(Instruction)> &callback;
        const ApiConfig *config;
    };
    Payload payload {cb, GetApiConfig()};

    GetApiConfig()->cGapi_->iVisitUsers(GetView(), &payload, [](AbckitInst *user, void *data) {
        const auto &payload = *static_cast<Payload *>(data);
        return payload.callback(Instruction(user, payload.config));
    });
    CheckError(GetApiConfig());
}

}  // namespace abckit

#endif  // CPP_ABCKIT_INSTRUCTION_IMPL_H
