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

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../include/c/isa/isa_dynamic.h"

#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock {

// NOLINTBEGIN(readability-identifier-naming)

inline AbckitCoreModule *IgetModule(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_MODULE;
}

inline void IsetModule(AbckitInst *inst, AbckitCoreModule *md)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(md == DEFAULT_CORE_MODULE);
}

inline enum AbckitIsaApiDynamicConditionCode IgetConditionCode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE;
}

inline void IsetConditionCode(AbckitInst *inst, enum AbckitIsaApiDynamicConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE);
}

inline enum AbckitIsaApiDynamicOpcode IgetOpcode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_API_DYNAMIC_OPCODE;
}

inline AbckitCoreImportDescriptor *IgetImportDescriptor(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_IMPORT_DESCRIPTOR;
}

inline void IsetImportDescriptor(AbckitInst *inst, AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
}

inline AbckitCoreExportDescriptor *IgetExportDescriptor(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_EXPORT_DESCRIPTOR;
}

inline void IsetExportDescriptor(AbckitInst *inst, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
}

inline AbckitInst *IcreateLoadString(AbckitGraph *graph, AbckitString *str)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(str == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdnan(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdinfinity(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdundefined(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdnull(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdsymbol(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdglobal(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdtrue(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdfalse(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdhole(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdnewtarget(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdthis(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreatePoplexenv(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGetunmappedargs(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAsyncfunctionenter(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdfunction(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDebugger(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGetpropiterator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGetiterator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGetasynciterator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1,
                                            AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateTestin(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDefinefieldbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                            AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDefinepropertybyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                               AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreateemptyobject(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreateemptyarray(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreategeneratorobj(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreateiterresultobj(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                       uint64_t imm0, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                           uint64_t imm0, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreatearraywithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreateobjectwithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNewobjapply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNewobjrange(AbckitGraph *graph, size_t argCount, ...)
{
    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideNewobjrange(AbckitGraph *graph, size_t argCount, ...)
{
    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNewlexenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideNewlexenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNewlexenvwithname(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideNewlexenvwithname(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCreateasyncgeneratorobj(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAsyncgeneratorresolve(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                AbckitInst *input2)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAdd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSub2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateMul2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDiv2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateMod2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateEq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLess(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLesseq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGreater(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGreatereq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateShl2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateShr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAshr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAnd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateOr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateXor2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateExp(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateTypeof(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateTonumber(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateTonumeric(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNeg(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNot(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateInc(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDec(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateIstrue(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateIsfalse(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateIsin(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateInstanceof(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStrictnoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStricteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeNotifyconcurrentresult(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeDefinefieldbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                        AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeDefinefieldbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                        AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeTopropertykey(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeCreateprivateproperty(AbckitGraph *graph, uint64_t imm0,
                                                           AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeDefineprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                           uint64_t imm1, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    EXPECT_TRUE(imm1 == DEFAULT_I64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeCallinit(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeDefinesendableclass(AbckitGraph *graph, AbckitCoreFunction *function,
                                                         AbckitLiteralArray *literalArray, uint64_t imm0,
                                                         AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeLdsendableclass(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeLdsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeWideldsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeNewsendableenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeWidenewsendableenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeStsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    EXPECT_TRUE(imm1 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeWidestsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                       uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    EXPECT_TRUE(imm1 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeLdsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    EXPECT_TRUE(imm1 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeWideldsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_I64);
    EXPECT_TRUE(imm1 == DEFAULT_I64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeIstrue(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallruntimeIsfalse(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrow(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowNotexists(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowPatternnoncoercible(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowDeletesuperproperty(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowConstassignment(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowIfnotobject(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowUndefinedifhole(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowIfsupernotcorrectcall(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrowUndefinedifholewithname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallarg0(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallarg1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallargs2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallargs3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                    AbckitInst *input2)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSupercallspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateApply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallthis0(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallthis1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallthis2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                    AbckitInst *input2)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    return DEFAULT_INST;
}

// CC-OFFNXT(G.FUN.01) function args are necessary
inline AbckitInst *IcreateCallthis3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                    AbckitInst *input2, AbckitInst *input3)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    EXPECT_TRUE(input3 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSupercallthisrange(AbckitGraph *graph, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideSupercallthisrange(AbckitGraph *graph, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

// CC-OFFNXT(G.FUN.01) function args are necessary
inline AbckitInst *IcreateDefinegettersetterbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                    AbckitInst *input1, AbckitInst *input2, AbckitInst *input3)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    EXPECT_TRUE(input3 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDefinefunc(AbckitGraph *graph, AbckitCoreFunction *function, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDefinemethod(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *function, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDefineclasswithbuffer(AbckitGraph *graph, AbckitCoreFunction *function,
                                                AbckitLiteralArray *literalArray, uint64_t imm0, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateResumegenerator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGetresumemode(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGettemplateobject(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGetnextpropname(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDelobjprop(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSuspendgenerator(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAsyncfunctionawaituncaught(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCopydataproperties(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStarrayspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSetobjectwithproto(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStownbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAsyncfunctionresolve(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAsyncfunctionreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCopyrestargs(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideCopyrestargs(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(md == DEFAULT_CORE_MODULE);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(md == DEFAULT_CORE_MODULE);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStmodulevar(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideStmodulevar(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateTryldglobalbyname(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateTrystglobalbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdglobalvar(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStglobalvar(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStownbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStownbyvaluewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStownbynamewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                 AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdbigint(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdthisbyname(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStthisbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLdthisbyvalue(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStthisbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideLdpatchvar(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateWideStpatchvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDynamicimport(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAsyncgeneratorreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSetgeneratorstate(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateReturn(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateReturnundefined(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateIf(AbckitGraph *graph, AbckitInst *input, enum AbckitIsaApiDynamicConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE);
    return DEFAULT_INST;
}

AbckitIsaApiDynamic g_isaApiDynamicImpl = {
    IgetModule,
    IsetModule,
    IgetConditionCode,
    IsetConditionCode,
    IgetOpcode,
    IgetImportDescriptor,
    IsetImportDescriptor,
    IgetExportDescriptor,
    IsetExportDescriptor,
    IcreateLoadString,
    IcreateLdnan,
    IcreateLdinfinity,
    IcreateLdundefined,
    IcreateLdnull,
    IcreateLdsymbol,
    IcreateLdglobal,
    IcreateLdtrue,
    IcreateLdfalse,
    IcreateLdhole,
    IcreateLdnewtarget,
    IcreateLdthis,
    IcreatePoplexenv,
    IcreateGetunmappedargs,
    IcreateAsyncfunctionenter,
    IcreateLdfunction,
    IcreateDebugger,
    IcreateGetpropiterator,
    IcreateGetiterator,
    IcreateGetasynciterator,
    IcreateLdprivateproperty,
    IcreateStprivateproperty,
    IcreateTestin,
    IcreateDefinefieldbyname,
    IcreateDefinepropertybyname,
    IcreateCreateemptyobject,
    IcreateCreateemptyarray,
    IcreateCreategeneratorobj,
    IcreateCreateiterresultobj,
    IcreateCreateobjectwithexcludedkeys,
    IcreateWideCreateobjectwithexcludedkeys,
    IcreateCreatearraywithbuffer,
    IcreateCreateobjectwithbuffer,
    IcreateNewobjapply,
    IcreateNewobjrange,
    IcreateWideNewobjrange,
    IcreateNewlexenv,
    IcreateWideNewlexenv,
    IcreateNewlexenvwithname,
    IcreateWideNewlexenvwithname,
    IcreateCreateasyncgeneratorobj,
    IcreateAsyncgeneratorresolve,
    IcreateAdd2,
    IcreateSub2,
    IcreateMul2,
    IcreateDiv2,
    IcreateMod2,
    IcreateEq,
    IcreateNoteq,
    IcreateLess,
    IcreateLesseq,
    IcreateGreater,
    IcreateGreatereq,
    IcreateShl2,
    IcreateShr2,
    IcreateAshr2,
    IcreateAnd2,
    IcreateOr2,
    IcreateXor2,
    IcreateExp,
    IcreateTypeof,
    IcreateTonumber,
    IcreateTonumeric,
    IcreateNeg,
    IcreateNot,
    IcreateInc,
    IcreateDec,
    IcreateIstrue,
    IcreateIsfalse,
    IcreateIsin,
    IcreateInstanceof,
    IcreateStrictnoteq,
    IcreateStricteq,
    IcreateCallruntimeNotifyconcurrentresult,
    IcreateCallruntimeDefinefieldbyvalue,
    IcreateCallruntimeDefinefieldbyindex,
    IcreateCallruntimeTopropertykey,
    IcreateCallruntimeCreateprivateproperty,
    IcreateCallruntimeDefineprivateproperty,
    IcreateCallruntimeCallinit,
    IcreateCallruntimeDefinesendableclass,
    IcreateCallruntimeLdsendableclass,
    IcreateCallruntimeLdsendableexternalmodulevar,
    IcreateCallruntimeWideldsendableexternalmodulevar,
    IcreateCallruntimeNewsendableenv,
    IcreateCallruntimeWidenewsendableenv,
    IcreateCallruntimeStsendablevar,
    IcreateCallruntimeWidestsendablevar,
    IcreateCallruntimeLdsendablevar,
    IcreateCallruntimeWideldsendablevar,
    IcreateCallruntimeIstrue,
    IcreateCallruntimeIsfalse,
    IcreateThrow,
    IcreateThrowNotexists,
    IcreateThrowPatternnoncoercible,
    IcreateThrowDeletesuperproperty,
    IcreateThrowConstassignment,
    IcreateThrowIfnotobject,
    IcreateThrowUndefinedifhole,
    IcreateThrowIfsupernotcorrectcall,
    IcreateThrowUndefinedifholewithname,
    IcreateCallarg0,
    IcreateCallarg1,
    IcreateCallargs2,
    IcreateCallargs3,
    IcreateCallrange,
    IcreateWideCallrange,
    IcreateSupercallspread,
    IcreateApply,
    IcreateCallthis0,
    IcreateCallthis1,
    IcreateCallthis2,
    IcreateCallthis3,
    IcreateCallthisrange,
    IcreateWideCallthisrange,
    IcreateSupercallthisrange,
    IcreateWideSupercallthisrange,
    IcreateSupercallarrowrange,
    IcreateWideSupercallarrowrange,
    IcreateDefinegettersetterbyvalue,
    IcreateDefinefunc,
    IcreateDefinemethod,
    IcreateDefineclasswithbuffer,
    IcreateResumegenerator,
    IcreateGetresumemode,
    IcreateGettemplateobject,
    IcreateGetnextpropname,
    IcreateDelobjprop,
    IcreateSuspendgenerator,
    IcreateAsyncfunctionawaituncaught,
    IcreateCopydataproperties,
    IcreateStarrayspread,
    IcreateSetobjectwithproto,
    IcreateLdobjbyvalue,
    IcreateStobjbyvalue,
    IcreateStownbyvalue,
    IcreateLdsuperbyvalue,
    IcreateStsuperbyvalue,
    IcreateLdobjbyindex,
    IcreateWideLdobjbyindex,
    IcreateStobjbyindex,
    IcreateWideStobjbyindex,
    IcreateStownbyindex,
    IcreateWideStownbyindex,
    IcreateAsyncfunctionresolve,
    IcreateAsyncfunctionreject,
    IcreateCopyrestargs,
    IcreateWideCopyrestargs,
    IcreateLdlexvar,
    IcreateWideLdlexvar,
    IcreateStlexvar,
    IcreateWideStlexvar,
    IcreateGetmodulenamespace,
    IcreateWideGetmodulenamespace,
    IcreateStmodulevar,
    IcreateWideStmodulevar,
    IcreateTryldglobalbyname,
    IcreateTrystglobalbyname,
    IcreateLdglobalvar,
    IcreateStglobalvar,
    IcreateLdobjbyname,
    IcreateStobjbyname,
    IcreateStownbyname,
    IcreateLdsuperbyname,
    IcreateStsuperbyname,
    IcreateLdlocalmodulevar,
    IcreateWideLdlocalmodulevar,
    IcreateLdexternalmodulevar,
    IcreateWideLdexternalmodulevar,
    IcreateStownbyvaluewithnameset,
    IcreateStownbynamewithnameset,
    IcreateLdbigint,
    IcreateLdthisbyname,
    IcreateStthisbyname,
    IcreateLdthisbyvalue,
    IcreateStthisbyvalue,
    IcreateWideLdpatchvar,
    IcreateWideStpatchvar,
    IcreateDynamicimport,
    IcreateAsyncgeneratorreject,
    IcreateSetgeneratorstate,
    IcreateReturn,
    IcreateReturnundefined,
    IcreateIf,
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock

AbckitIsaApiDynamic const *AbckitGetMockIsaApiDynamicImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::g_isaApiDynamicImpl;
}
