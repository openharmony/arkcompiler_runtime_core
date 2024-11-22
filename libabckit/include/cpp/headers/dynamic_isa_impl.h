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

#include "./graph.h"
#include "./instruction.h"
#include "./literal_array.h"
#include "./core/import_descriptor.h"
#include "./dynamic_isa.h"

#include <memory>
#include <type_traits>

namespace abckit {

inline Instruction DynamicIsa::CreateCallArg1(const Instruction &acc, const Instruction &input0) const &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *callArg1 = conf->cDynapi_->iCreateCallarg1(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(callArg1, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallThis0(const Instruction &acc, const Instruction &input0) const &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *callthis0 = conf->cDynapi_->iCreateCallthis0(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(callthis0, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallThis2(const Instruction &acc, const Instruction &input0,
                                               const Instruction &input1, const Instruction &input2) const &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *callthis2 = conf->cDynapi_->iCreateCallthis2(graph_.GetResource(), acc.GetView(), input0.GetView(),
                                                             input1.GetView(), input2.GetView());
    CheckError(conf);
    return Instruction(callthis2, conf, &graph_);
}

inline Instruction DynamicIsa::CreateTryldglobalbyname(std::string_view str) const &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);

    // NOTE(urandon): resolve duplicates for str
    AbckitString *abcStr = conf->cMapi_->createString(file, str.data());
    CheckError(conf);
    AbckitInst *callArg1 = conf->cDynapi_->iCreateTryldglobalbyname(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(callArg1, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdObjByName(const Instruction &acc, std::string_view str) const &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);
    AbckitString *abcStr = conf->cMapi_->createString(file, str.data());
    CheckError(conf);
    AbckitInst *loadobj = conf->cDynapi_->iCreateLdobjbyname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(loadobj, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLoadString(std::string_view str) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);
    AbckitString *abcStr = conf->cMapi_->createString(file, str.data());
    CheckError(conf);
    AbckitInst *abcLoadstring = conf->cDynapi_->iCreateLoadString(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(abcLoadstring, conf, &graph_);
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
inline Instruction DynamicIsa::CreateNewobjrange(const Instruction &instClass, Instructions &&...instArgs) const &&
{
    static_assert((... && std::is_convertible_v<Instructions, Instruction>),
                  "Expected abckit::Instruction instances at `instArgs` for CreateNewobjrange");

    auto *conf = graph_.GetApiConfig();
    AbckitInst *objrange = conf->cDynapi_->iCreateNewobjrange(graph_.GetResource(), 1 + sizeof...(instArgs),
                                                              instClass.GetView(), instArgs.GetView()...);
    CheckError(conf);
    return Instruction(objrange, conf, &graph_);
}

inline Instruction DynamicIsa::CreateSub2(const Instruction &acc, const Instruction &input0) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *subInst = conf->cDynapi_->iCreateSub2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(subInst, conf, &graph_);
}

inline core::ImportDescriptor DynamicIsa::GetImportDescriptor(const Instruction &inst) const
{
    auto *conf = graph_.GetApiConfig();
    AbckitCoreImportDescriptor *id = conf->cDynapi_->iGetImportDescriptor(inst.GetView());
    CheckError(conf);
    return core::ImportDescriptor(id, conf, graph_.GetFile());
}

inline Instruction DynamicIsa::CreateGreatereq(const Instruction &acc, const Instruction &input0) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *createreqInst = conf->cDynapi_->iCreateGreatereq(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(createreqInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateIf(const Instruction &input, enum AbckitIsaApiDynamicConditionCode cc) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *createIfInst = conf->cDynapi_->iCreateIf(graph_.GetResource(), input.GetView(), cc);
    CheckError(conf);
    return Instruction(createIfInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateReturn(const Instruction &acc) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *returnInst = conf->cDynapi_->iCreateReturn(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(returnInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdExternalModuleVar(const core::ImportDescriptor &idesc) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *ldInst = conf->cDynapi_->iCreateLdexternalmodulevar(graph_.GetResource(), idesc.GetView());
    CheckError(conf);
    return Instruction(ldInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowUndefinedIfHoleWithName(const Instruction &acc,
                                                                  std::string_view string) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitFile *file = conf->cGapi_->gGetFile(graph_.GetResource());
    CheckError(conf);
    AbckitString *abcStr = conf->cMapi_->createString(file, string.data());
    CheckError(conf);
    AbckitInst *ldInst =
        conf->cDynapi_->iCreateThrowUndefinedifholewithname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(ldInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdTrue() const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *returnInst = conf->cDynapi_->iCreateLdtrue(graph_.GetResource());
    CheckError(conf);
    return Instruction(returnInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdFalse() const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *returnInst = conf->cDynapi_->iCreateLdfalse(graph_.GetResource());
    CheckError(conf);
    return Instruction(returnInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreateArrayWithBuffer(const abckit::LiteralArray &literalArray) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *returnInst = conf->cDynapi_->iCreateCreatearraywithbuffer(graph_.GetResource(), literalArray.GetView());
    CheckError(conf);
    return Instruction(returnInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStownByIndex(const Instruction &acc, const Instruction &input0,
                                                  uint64_t imm0) const &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *returnInst =
        conf->cDynapi_->iCreateStownbyindex(graph_.GetResource(), acc.GetView(), input0.GetView(), imm0);
    CheckError(conf);
    return Instruction(returnInst, conf, &graph_);
}

}  // namespace abckit

#endif  // CPP_ABCKIT_DYNAMIC_ISA_IMPL_H
