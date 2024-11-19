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

#include "./dynamic_isa.h"
#include "./instruction.h"
#include "./graph.h"
#include "./core/import_descriptor.h"

#include <memory>
#include <type_traits>

namespace abckit {

inline Instruction DynamicIsa::CreateCallArg1(const Instruction &acc, const Instruction &input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *callArg1 = conf->cDynapi_->iCreateCallarg1(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(callArg1, conf);
}

inline Instruction DynamicIsa::CreateCallThis0(const Instruction &acc, const Instruction &input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *callthis0 = conf->cDynapi_->iCreateCallthis0(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(callthis0, conf);
}

inline Instruction DynamicIsa::CreateTryldglobalbyname(std::string_view str) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);

    // NOTE(urandon): resolve duplicates for str
    AbckitString *abcStr = conf->cMapi_->createString(file, str.data());
    CheckError(conf);
    AbckitInst *callArg1 = conf->cDynapi_->iCreateTryldglobalbyname(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(callArg1, conf);
}

inline Instruction DynamicIsa::CreateLdObjByName(const Instruction &acc, std::string_view str) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);
    AbckitString *abcStr = conf->cMapi_->createString(file, str.data());
    CheckError(conf);
    AbckitInst *loadobj = conf->cDynapi_->iCreateLdobjbyname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(loadobj, conf);
}

inline Instruction DynamicIsa::CreateLoadString(std::string_view str) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);
    AbckitString *abcStr = conf->cMapi_->createString(file, str.data());
    CheckError(conf);
    AbckitInst *abcLoadstring = conf->cDynapi_->iCreateLoadString(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(abcLoadstring, conf);
}

/**
 * @brief Creates instruction with opcode NEWOBJRANGE. This instruction invokes the constructor of `instClass` with
 * arguments `instArgs...` to create a class instance, and stores the instance in returned instruction`.
 * @tparam Instructions - a parameter pack for abckit::Instruction
 * @param [ in ] instClass - Inst containing class object
 * @param [ in ] instArgs - Insts containing constructor arguments
 * @return `Instruction`
 */
template <class... Instructions>
inline Instruction DynamicIsa::CreateNewobjrange(const Instruction &instClass, Instructions &&...instArgs) &&
{
    static_assert((... && std::is_convertible_v<Instructions, Instruction>),
                  "Expected abckit::Instruction instances at `instArgs` for CreateNewobjrange");

    auto *conf = graph_.GetApiConfig();
    AbckitInst *objrange = conf->cDynapi_->iCreateNewobjrange(graph_.GetResource(), 1 + sizeof...(instArgs),
                                                              instClass.GetView(), instArgs.GetView()...);
    CheckError(conf);
    return Instruction(objrange, conf);
}

inline Instruction DynamicIsa::CreateSub2(const Instruction &acc, const Instruction &input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *subInst = conf->cDynapi_->iCreateSub2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(subInst, conf);
}

inline core::ImportDescriptor DynamicIsa::GetImportDescriptor(const Instruction &inst)
{
    auto *conf = graph_.GetApiConfig();
    AbckitCoreImportDescriptor *id = conf->cDynapi_->iGetImportDescriptor(inst.GetView());
    CheckError(conf);
    return core::ImportDescriptor(id, conf);
}

}  // namespace abckit

#endif  // CPP_ABCKIT_DYNAMIC_ISA_IMPL_H
