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

#include "../../include/c/metadata_core.h"
#include "../../src/include_v2/c/isa/isa_static.h"

#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock {

// NOLINTBEGIN(readability-identifier-naming)

inline AbckitCoreClass *IgetClass(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_CLASS;
}

inline void IsetClass(AbckitInst *inst, AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
}

inline enum AbckitIsaApiStaticConditionCode IgetConditionCode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_STATIC_CONDITION_CODE;
}

inline void IsetConditionCode(AbckitInst *inst, enum AbckitIsaApiStaticConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_STATIC_CONDITION_CODE);
}

inline enum AbckitIsaApiStaticOpcode IgetOpcode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_API_STATIC_OPCODE;
}

inline void IsetTargetType(AbckitInst *inst, AbckitTypeId t)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(t == DEFAULT_TYPE_ID);
}

inline enum AbckitTypeId IgetTargetType(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_TYPE_ID;
}

inline AbckitInst *IcreateCmp(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLoadString(AbckitGraph *graph, AbckitString *str)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(str == DEFAULT_STRING);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateReturn(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateIf(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                             enum AbckitIsaApiStaticConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_STATIC_CONDITION_CODE);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNeg(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNot(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAdd(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSub(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateMul(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDiv(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateMod(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateShl(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateShr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAShr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAnd(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateOr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateXor(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCast(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetType)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(targetType == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

inline AbckitInst *GcreateNullPtr(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNewArray(AbckitGraph *graph, AbckitCoreClass *inputClass, AbckitInst *inputSize)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputClass == DEFAULT_CORE_CLASS);
    EXPECT_TRUE(inputSize == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateNewObject(AbckitGraph *graph, AbckitCoreClass *inputClass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputClass == DEFAULT_CORE_CLASS);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateInitObject(AbckitGraph *graph, AbckitCoreFunction *function, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLoadArray(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                    AbckitTypeId returnTypeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arrayRef == DEFAULT_INST);
    EXPECT_TRUE(idx == DEFAULT_INST);
    EXPECT_TRUE(returnTypeId == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStoreArray(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                     AbckitTypeId returnTypeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arrayRef == DEFAULT_INST);
    EXPECT_TRUE(idx == DEFAULT_INST);
    EXPECT_TRUE(value == DEFAULT_INST);
    EXPECT_TRUE(returnTypeId == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateStoreArrayWide(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                         AbckitTypeId returnTypeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arrayRef == DEFAULT_INST);
    EXPECT_TRUE(idx == DEFAULT_INST);
    EXPECT_TRUE(value == DEFAULT_INST);
    EXPECT_TRUE(returnTypeId == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLenArray(AbckitGraph *graph, AbckitInst *arr)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arr == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLoadConstArray(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCheckCast(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(targetType == DEFAULT_TYPE);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateIsInstance(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(targetType == DEFAULT_TYPE);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateLoadUndefined(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateReturnVoid(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateEquals(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallStatic(AbckitGraph *graph, AbckitCoreFunction *function, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateCallVirtual(AbckitGraph *graph, AbckitInst *inputObj, AbckitCoreFunction *function,
                                      size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAddI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateSubI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateMulI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateDivI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateModI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateShlI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateShrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAShrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateAndI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateOrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateXorI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateThrow(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

inline AbckitInst *IcreateIsUndefined(AbckitGraph *graph, AbckitInst *inputObj)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitIsaApiStatic g_isaApiStaticImpl = {
    IgetClass,
    IsetClass,
    IgetConditionCode,
    IsetConditionCode,
    IgetOpcode,
    IsetTargetType,
    IgetTargetType,
    IcreateCmp,
    IcreateLoadString,
    IcreateReturn,
    IcreateIf,
    IcreateNeg,
    IcreateNot,
    IcreateAdd,
    IcreateSub,
    IcreateMul,
    IcreateDiv,
    IcreateMod,
    IcreateShl,
    IcreateShr,
    IcreateAShr,
    IcreateAnd,
    IcreateOr,
    IcreateXor,
    IcreateCast,
    GcreateNullPtr,
    IcreateNewArray,
    IcreateNewObject,
    IcreateInitObject,
    IcreateLoadArray,
    IcreateStoreArray,
    IcreateStoreArrayWide,
    IcreateLenArray,
    IcreateLoadConstArray,
    IcreateCheckCast,
    IcreateIsInstance,
    IcreateLoadUndefined,
    IcreateReturnVoid,
    IcreateEquals,
    IcreateCallStatic,
    IcreateCallVirtual,
    IcreateAddI,
    IcreateSubI,
    IcreateMulI,
    IcreateDivI,
    IcreateModI,
    IcreateShlI,
    IcreateShrI,
    IcreateAShrI,
    IcreateAndI,
    IcreateOrI,
    IcreateXorI,
    IcreateThrow,
    IcreateIsUndefined,
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock

AbckitIsaApiStatic const *AbckitGetMockIsaApiStaticImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::g_isaApiStaticImpl;
}