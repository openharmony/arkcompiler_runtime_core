/**
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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_HELPERS_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_HELPERS_STATIC_H

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/ir_core.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"

#include "static_core/compiler/optimizer/ir/inst.h"
#include "static_core/libpandabase/mem/arena_allocator.h"

#include <string>
#include <tuple>

namespace libabckit {

struct CtxGInternal {
    ark::ArenaAllocator *allocator;
    ark::ArenaAllocator *localAllocator;
    const AbckitIrInterface *irInterface;
    AbckitRuntimeAdapterStatic *runtimeAdapter;
};

std::tuple<std::string, std::string> ClassGetNames(const std::string &fullName);
std::tuple<std::string, std::string> FuncGetNames(const std::string &fullSig);
std::string FuncNameCropModule(const std::string &fullSig);

void CheckInvalidOpcodes(ark::compiler::Graph *graph, bool isDynamic);
size_t GetIntrinicMaxInputsCount(AbckitInst *inst);
bool IsCallInst(AbckitInst *inst);

AbckitIsaApiStaticOpcode GetStaticOpcode(ark::compiler::Inst *inst);
AbckitIsaApiDynamicOpcode GetDynamicOpcode(ark::compiler::Inst *inst);
AbckitTypeId TypeToTypeId(ark::compiler::DataType::Type type);
ark::compiler::DataType::Type TypeIdToType(AbckitTypeId typeId);
ark::compiler::ConditionCode CcToCc(AbckitIsaApiDynamicConditionCode cc);
ark::compiler::ConditionCode CcToCc(AbckitIsaApiStaticConditionCode cc);
AbckitIsaApiDynamicConditionCode CcToDynamicCc(ark::compiler::ConditionCode cc);
AbckitIsaApiStaticConditionCode CcToStaticCc(ark::compiler::ConditionCode cc);
void SetLastError(AbckitStatus err);

std::string GetFuncName(AbckitCoreFunction *function);
std::string GetMangleFuncName(AbckitCoreFunction *function);

uint32_t GetClassOffset(AbckitGraph *graph, AbckitCoreClass *klass);
uint32_t GetMethodOffset(AbckitGraph *graph, AbckitCoreFunction *function);
uint32_t GetStringOffset(AbckitGraph *graph, AbckitString *string);
uint32_t GetLiteralArrayOffset(AbckitGraph *graph, AbckitLiteralArray *arr);

AbckitInst *CreateInstFromImpl(AbckitGraph *graph, ark::compiler::Inst *impl);

AbckitInst *CreateDynInst(AbckitGraph *graph, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, ark::compiler::DataType::Type retType,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC = true);

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1, uint64_t imm2, AbckitInst *input0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, AbckitInst *input3, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, size_t argCount, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// Forward declaration of functions which implemented in helpers_dynamic.cpp
panda::pandasm::Function *GetDynFunction(AbckitCoreFunction *function);
AbckitModulePayloadDyn *GetDynModulePayload(AbckitCoreModule *mod);
AbckitDynamicImportDescriptorPayload *GetDynImportDescriptorPayload(AbckitCoreImportDescriptor *id);
AbckitDynamicExportDescriptorPayload *GetDynExportDescriptorPayload(AbckitCoreExportDescriptor *ed);

AbckitLiteral *GetOrCreateLiteralBoolStatic(AbckitFile *file, bool value);
AbckitLiteral *GetOrCreateLiteralU8Static(AbckitFile *file, uint8_t value);
AbckitLiteral *GetOrCreateLiteralU16Static(AbckitFile *file, uint16_t value);
AbckitLiteral *GetOrCreateLiteralMethodAffiliateStatic(AbckitFile *file, uint16_t value);
AbckitLiteral *GetOrCreateLiteralU32Static(AbckitFile *file, uint32_t value);
AbckitLiteral *GetOrCreateLiteralU64Static(AbckitFile *file, uint64_t value);
AbckitLiteral *GetOrCreateLiteralFloatStatic(AbckitFile *file, float value);
AbckitLiteral *GetOrCreateLiteralDoubleStatic(AbckitFile *file, double value);
AbckitLiteral *GetOrCreateLiteralStringStatic(AbckitFile *file, const std::string &value);
AbckitLiteral *GetOrCreateLiteralMethodStatic(AbckitFile *file, const std::string &value);

}  // namespace libabckit

#endif
