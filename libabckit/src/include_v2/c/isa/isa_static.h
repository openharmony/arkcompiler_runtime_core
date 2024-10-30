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

#ifndef LIBABCKIT_ISA_ISA_STATIC_H
#define LIBABCKIT_ISA_ISA_STATIC_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif

#include "../../../../include/c/metadata_core.h"
#include "../../../../include/c/ir_core.h"
#include "../../../../include/c/abckit.h"

#ifdef __cplusplus
extern "C" {
#endif

enum AbckitIsaApiStaticOpcode {
    ABCKIT_ISA_API_STATIC_OPCODE_INVALID,
    ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT,
    ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING,
    ABCKIT_ISA_API_STATIC_OPCODE_RETURN,
    ABCKIT_ISA_API_STATIC_OPCODE_TRY,
    ABCKIT_ISA_API_STATIC_OPCODE_CATCHPHI,
    ABCKIT_ISA_API_STATIC_OPCODE_PHI,

    /* Unary operations: */
    ABCKIT_ISA_API_STATIC_OPCODE_NEG,
    ABCKIT_ISA_API_STATIC_OPCODE_NOT,

    /* BinaryOperation: */
    ABCKIT_ISA_API_STATIC_OPCODE_CMP,
    ABCKIT_ISA_API_STATIC_OPCODE_ADD,
    ABCKIT_ISA_API_STATIC_OPCODE_SUB,
    ABCKIT_ISA_API_STATIC_OPCODE_MUL,
    ABCKIT_ISA_API_STATIC_OPCODE_DIV,
    ABCKIT_ISA_API_STATIC_OPCODE_MOD,
    ABCKIT_ISA_API_STATIC_OPCODE_SHL,
    ABCKIT_ISA_API_STATIC_OPCODE_SHR,
    ABCKIT_ISA_API_STATIC_OPCODE_ASHR,
    ABCKIT_ISA_API_STATIC_OPCODE_AND,
    ABCKIT_ISA_API_STATIC_OPCODE_OR,
    ABCKIT_ISA_API_STATIC_OPCODE_XOR,

    /* Cast: */
    ABCKIT_ISA_API_STATIC_OPCODE_CAST,

    /* Object manipulation: */
    ABCKIT_ISA_API_STATIC_OPCODE_NULLPTR,
    ABCKIT_ISA_API_STATIC_OPCODE_NEWARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_NEWOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_STOREARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC,
    ABCKIT_ISA_API_STATIC_OPCODE_STORESTATIC,
    ABCKIT_ISA_API_STATIC_OPCODE_LENARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADCONSTARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_CHECKCAST,
    ABCKIT_ISA_API_STATIC_OPCODE_ISINSTANCE,
    ABCKIT_ISA_API_STATIC_OPCODE_ISUNDEFINED,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADUNDEFINED,
    ABCKIT_ISA_API_STATIC_OPCODE_EQUALS,

    /* Return: */
    ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID,

    /* Calls: */
    ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC,
    ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL,

    /* BinaryImmOperation: */
    ABCKIT_ISA_API_STATIC_OPCODE_ADDI,
    ABCKIT_ISA_API_STATIC_OPCODE_SUBI,
    ABCKIT_ISA_API_STATIC_OPCODE_MULI,
    ABCKIT_ISA_API_STATIC_OPCODE_DIVI,
    ABCKIT_ISA_API_STATIC_OPCODE_MODI,
    ABCKIT_ISA_API_STATIC_OPCODE_SHLI,
    ABCKIT_ISA_API_STATIC_OPCODE_SHRI,
    ABCKIT_ISA_API_STATIC_OPCODE_ASHRI,
    ABCKIT_ISA_API_STATIC_OPCODE_ANDI,
    ABCKIT_ISA_API_STATIC_OPCODE_ORI,
    ABCKIT_ISA_API_STATIC_OPCODE_XORI,

    /* Exceptions */
    ABCKIT_ISA_API_STATIC_OPCODE_THROW,

    /* Condition */
    ABCKIT_ISA_API_STATIC_OPCODE_IF,
};

enum AbckitIsaApiStaticConditionCode {
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE = 0,
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ, /* == */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE, /* != */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT, /* < */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE, /* <= */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT, /* > */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE, /* >= */
    // Unsigned integers.
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B,  /* < */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE, /* <= */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A,  /* > */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE, /* >= */
    // Compare result of bitwise AND with zero
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ, /* (lhs AND rhs) == 0 */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE, /* (lhs AND rhs) != 0 */
};

/**
 * @brief Struct that holds the pointers to the API used to work with static ISA.
 * @note Valid targets: `ABCKIT_TARGET_ARK_TS_V2`.
 */
struct AbckitIsaApiStatic {
    /**
     * @brief iGetClass.
     * @return AbckitCoreClass *.
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    AbckitCoreClass *(*iGetClass)(AbckitInst *inst /* in */);

    /**
     * @brief iSetClass.
     * @return void .
     * @param [ in ] AbckitInst *inst .
     * @param [ in ]  AbckitCoreClass *klass .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreClass *klass  is NULL.
     */
    void (*iSetClass)(AbckitInst *inst /* in */, AbckitCoreClass *klass /* in */);

    /**
     * @brief iGetConditionCode.
     * @return enum AbckitIsaApiStaticConditionCode .
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    enum AbckitIsaApiStaticConditionCode (*iGetConditionCode)(AbckitInst *inst /* in */);

    /**
     * @brief iSetConditionCode.
     * @return void .
     * @param [ in ] AbckitInst *inst .
     * @param [ in ]  enum AbckitIsaApiStaticConditionCode cc .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    void (*iSetConditionCode)(AbckitInst *inst /* in */, enum AbckitIsaApiStaticConditionCode cc /* in */);

    /**
     * @brief iGetOpcode.
     * @return enum AbckitIsaApiStaticOpcode .
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    enum AbckitIsaApiStaticOpcode (*iGetOpcode)(AbckitInst *inst /* in */);

    /* For Cast instructions */
    /**
     * @brief iSetTargetType.
     * @return void .
     * @param [ in ] AbckitInst *inst .
     * @param [ in ]  enum AbckitTypeId t .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    void (*iSetTargetType)(AbckitInst *inst /* in */, enum AbckitTypeId t /* in */);

    /* For Cast instructions */
    /**
     * @brief iGetTargetType.
     * @return enum AbckitTypeId .
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    enum AbckitTypeId (*iGetTargetType)(AbckitInst *inst /* in */);

    /**
     * @brief iCreateCmp.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateCmp)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateLoadString.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitString *str .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitString *str  is NULL.
     */
    AbckitInst *(*iCreateLoadString)(AbckitGraph *graph /* in */, AbckitString *str /* in */);

    /**
     * @brief iCreateReturn.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateReturn)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */);

    /**
     * @brief iCreateIf.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @param [ in ] enum AbckitIsaApiStaticConditionCode cc .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateIf)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */,
                             enum AbckitIsaApiStaticConditionCode cc /* in */);

    /**
     * @brief iCreateNeg.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateNeg)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */);

    /**
     * @brief iCreateNot.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateNot)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */);

    /**
     * @brief iCreateAdd.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateAdd)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateSub.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateSub)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateMul.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateMul)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateDiv.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateDiv)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateMod.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateMod)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateShl.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateShl)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateShr.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateShr)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateAShr.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateAShr)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateAnd.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateAnd)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateOr.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateOr)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateXor.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateXor)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateCast.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ] enum AbckitTypeId targetType .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateCast)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */,
                               enum AbckitTypeId targetType /* in */);

    /**
     * @brief gCreateNullPtr.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     */
    AbckitInst *(*gCreateNullPtr)(AbckitGraph *graph /* in */);

    /**
     * @brief iCreateNewArray.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreClass *inputClass .
     * @param [ in ] AbckitInst *inputSize .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreClass *inputClass  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputSize  is NULL.
     */
    AbckitInst *(*iCreateNewArray)(AbckitGraph *graph /* in */, AbckitCoreClass *inputClass /* in */,
                                   AbckitInst *inputSize /* in */);

    /**
     * @brief iCreateNewObject.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreClass *inputClass .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreClass *inputClass  is NULL.
     */
    AbckitInst *(*iCreateNewObject)(AbckitGraph *graph /* in */, AbckitCoreClass *inputClass /* in */);

    /**
     * @brief iCreateInitObject.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreFunction *function .
     * @param [ in ] size_t argCount .
     * @param  ... .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreFunction *function  is NULL.
     */
    AbckitInst *(*iCreateInitObject)(AbckitGraph *graph /* in */, AbckitCoreFunction *function /* in */,
                                     size_t argCount /* in */, ... /* function params */);

    /**
     * @brief iCreateLoadArray.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arrayRef .
     * @param [ in ] AbckitInst *idx .
     * @param [ in ]  enum AbckitTypeId returnTypeId .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arrayRef  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *idx  is NULL.
     */
    AbckitInst *(*iCreateLoadArray)(AbckitGraph *graph /* in */, AbckitInst *arrayRef /* in */,
                                    AbckitInst *idx /* in */, enum AbckitTypeId returnTypeId /* in */);

    /**
     * @brief iCreateStoreArray.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arrayRef .
     * @param [ in ] AbckitInst *idx .
     * @param [ in ]  AbckitInst *value .
     * @param [ in ] enum AbckitTypeId valueTypeId .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arrayRef  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *idx  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *value  is NULL.
     */
    AbckitInst *(*iCreateStoreArray)(AbckitGraph *graph /* in */, AbckitInst *arrayRef /* in */,
                                     AbckitInst *idx /* in */, AbckitInst *value /* in */,
                                     enum AbckitTypeId valueTypeId /* in */);

    /**
     * @brief iCreateStoreArrayWide.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arrayRef .
     * @param [ in ] AbckitInst *idx .
     * @param [ in ]  AbckitInst *value .
     * @param [ in ] enum AbckitTypeId valueTypeId .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arrayRef  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *idx  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *value  is NULL.
     */
    AbckitInst *(*iCreateStoreArrayWide)(AbckitGraph *graph /* in */, AbckitInst *arrayRef /* in */,
                                         AbckitInst *idx /* in */, AbckitInst *value /* in */,
                                         enum AbckitTypeId valueTypeId /* in */);

    /**
     * @brief iCreateLenArray.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arr .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arr  is NULL.
     */
    AbckitInst *(*iCreateLenArray)(AbckitGraph *graph /* in */, AbckitInst *arr /* in */);

    /**
     * @brief iCreateLoadConstArray.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitLiteralArray *literalArray .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitLiteralArray *literalArray  is NULL.
     */
    AbckitInst *(*iCreateLoadConstArray)(AbckitGraph *graph /* in */, AbckitLiteralArray *literalArray /* in */);

    /**
     * @brief iCreateCheckCast.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @param [ in ] AbckitType *targetType .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitType *targetType  is NULL.
     */
    AbckitInst *(*iCreateCheckCast)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                    AbckitType *targetType /* in */);

    /**
     * @brief iCreateIsInstance.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @param [ in ] AbckitType *targetType .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitType *targetType  is NULL.
     */
    AbckitInst *(*iCreateIsInstance)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                     AbckitType *targetType /* in */);

    /**
     * @brief iCreateLoadUndefined.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     */
    AbckitInst *(*iCreateLoadUndefined)(AbckitGraph *graph /* in */);

    /**
     * @brief iCreateReturnVoid.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     */
    AbckitInst *(*iCreateReturnVoid)(AbckitGraph *graph /* in */);

    /**
     * @brief iCreateEquals.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateEquals)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateCallStatic.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreFunction *function .
     * @param [ in ] size_t argCount .
     * @param  ... .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreFunction *function  is NULL.
     */
    AbckitInst *(*iCreateCallStatic)(AbckitGraph *graph /* in */, AbckitCoreFunction *function /* in */,
                                     size_t argCount /* in */, ... /* function params */);

    /**
     * @brief iCreateCallVirtual.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @param [ in ] AbckitCoreFunction *function .
     * @param [ in ]  size_t argCount .
     * @param ... .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreFunction *function  is NULL.
     */
    AbckitInst *(*iCreateCallVirtual)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                      AbckitCoreFunction *function /* in */, size_t argCount /* in */,
                                      ... /* function params */);

    /**
     * @brief iCreateAddI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateAddI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateSubI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateSubI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateMulI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateMulI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateDivI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateDivI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateModI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateModI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateShlI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateShlI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateShrI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateShrI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateAShrI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateAShrI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateAndI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateAndI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateOrI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateOrI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateXorI.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateXorI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief iCreateThrow.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *acc .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *acc  is NULL.
     */
    AbckitInst *(*iCreateThrow)(AbckitGraph *graph /* in */, AbckitInst *acc /* in */);

    /**
     * @brief iCreateIsUndefined.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     */
    AbckitInst *(*iCreateIsUndefined)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */);
};

/**
 * @brief Instantiates API for for working with static ISA.
 * @return Instance of the `AbckitIsaApiStatic` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
struct AbckitIsaApiStatic const *AbckitGetIsaApiStaticImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif

#endif  // LIBABCKIT_ISA_ISA_STATIC_H
