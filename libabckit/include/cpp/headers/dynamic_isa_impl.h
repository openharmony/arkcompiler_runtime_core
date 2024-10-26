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

#ifndef CPP_ABCKIT_DYNAMIC_ISA_IMPL_H
#define CPP_ABCKIT_DYNAMIC_ISA_IMPL_H

#include "cpp/headers/dynamic_isa.h"
#include "cpp/headers/instruction.h"
#include "cpp/headers/graph.h"

#include <memory>

namespace abckit {

inline Instruction DynamicIsa::CreateCallArg1(const Instruction &acc, const Instruction &input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *callArg1 = conf->cDynapi_->iCreateCallarg1(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(callArg1, conf);
}

inline Instruction DynamicIsa::CreateTryldglobalbyname(const std::string &str) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);

    AbckitInst *callArg1 =
        conf->cDynapi_->iCreateTryldglobalbyname(graph_.GetResource(), conf->cMapi_->createString(file, str.c_str()));
    CheckError(conf);
    return Instruction(callArg1, conf);
}

inline Instruction DynamicIsa::CreateLoadString(const std::string &str) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);
    AbckitString *abcStr = conf->cMapi_->createString(file, str.c_str());
    CheckError(conf);
    AbckitInst *abcLoadstring = conf->cDynapi_->iCreateLoadString(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(abcLoadstring, conf);
}

}  // namespace abckit

#endif  // CPP_ABCKIT_DYNAMIC_ISA_IMPL_H
