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

#include "cpp/headers/instruction.h"

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

}  // namespace abckit

#endif  // CPP_ABCKIT_INSTRUCTION_IMPL_H
