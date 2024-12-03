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

#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/wrappers/pandasm_wrapper.h"

#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"
#include "static_core/bytecode_optimizer/reg_acc_alloc.h"
#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir/basicblock.h"
#include "static_core/compiler/optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "static_core/compiler/optimizer/optimizations/regalloc/reg_alloc_resolver.h"

#include "abckit_intrinsics_opcodes.inc"

static constexpr uint32_t IC_SLOT_VALUE = 0xF;

namespace libabckit {

std::tuple<std::string, std::string> ClassGetNames(const std::string &fullName)
{
    static const std::string GLOBAL_CLASS_NAME = "ETSGLOBAL";
    std::string::size_type pos = fullName.rfind('.');
    if (pos == std::string::npos) {
        return {".", fullName};
    }
    if (pos < GLOBAL_CLASS_NAME.size()) {
        return {fullName.substr(0, pos), fullName.substr(pos + 1)};
    }
    auto rawModuleName = fullName.substr(pos - GLOBAL_CLASS_NAME.size(), GLOBAL_CLASS_NAME.size());
    if (rawModuleName == GLOBAL_CLASS_NAME) {
        std::string::size_type pos2 = fullName.substr(0, pos).rfind('.');
        return {fullName.substr(0, pos2), fullName.substr(pos2 + 1)};
    }
    return {fullName.substr(0, pos), fullName.substr(pos + 1)};
}

std::tuple<std::string, std::string> FuncGetNames(const std::string &fullSig)
{
    auto fullName = fullSig.substr(0, fullSig.find(':'));
    std::string::size_type pos = fullName.rfind('.');
    if (pos == std::string::npos) {
        return {".", "ETSGLOBAL"};
    }
    return ClassGetNames(fullName.substr(0, pos));
}

std::string FuncNameCropModule(const std::string &fullSig)
{
    auto fullName = fullSig.substr(0, fullSig.find(':'));
    std::string::size_type dotPos = fullName.rfind('.');
    if (dotPos != std::string::npos) {
        return fullSig.substr(dotPos + 1);
    }
    return fullSig;
}

void CheckInvalidOpcodes([[maybe_unused]] compiler::Graph *graph, [[maybe_unused]] bool isDynamic)
{
#ifndef NDEBUG
    for (auto *bb : graph->GetBlocksRPO()) {
        for (auto *inst : bb->AllInsts()) {
            bool isInvalid = isDynamic ? (GetDynamicOpcode(inst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID)
                                       : (GetStaticOpcode(inst) == ABCKIT_ISA_API_STATIC_OPCODE_INVALID);
            if (isInvalid) {
                std::ostringstream out;
                LIBABCKIT_LOG_DUMP(inst->DumpOpcode(&out), DEBUG);
                LIBABCKIT_LOG(DEBUG) << "ASSERTION FAILED: Invalid opcode encountered: " << out.str() << std::endl;
                ASSERT(false);
            }
        }
    }
#endif
}

// CC-OFFNXT(G.FUD.05) huge function, big switch-case
// CC-OFFNXT(G.FUN.01-CPP) huge function, big switch-case
AbckitIsaApiStaticOpcode GetStaticOpcode(compiler::Inst *inst)
{
    auto opcode = inst->GetOpcode();
    switch (opcode) {
        case compiler::Opcode::CallStatic:
            return ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC;
        case compiler::Opcode::CallVirtual:
            return ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL;
        case compiler::Opcode::LoadStatic:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC;
        case compiler::Opcode::LoadString:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING;
        case compiler::Opcode::LoadObject:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT;
        case compiler::Opcode::Sub:
            return ABCKIT_ISA_API_STATIC_OPCODE_SUB;
        case compiler::Opcode::ReturnVoid:
            return ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID;
        case compiler::Opcode::Parameter:
            return ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER;
        case compiler::Opcode::Constant:
            return ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT;
        case compiler::Opcode::Cmp:
            return ABCKIT_ISA_API_STATIC_OPCODE_CMP;
        case compiler::Opcode::Cast:
            return ABCKIT_ISA_API_STATIC_OPCODE_CAST;
        case compiler::Opcode::Return:
            return ABCKIT_ISA_API_STATIC_OPCODE_RETURN;
        case compiler::Opcode::Add:
            return ABCKIT_ISA_API_STATIC_OPCODE_ADD;
        case compiler::Opcode::Mul:
            return ABCKIT_ISA_API_STATIC_OPCODE_MUL;
        case compiler::Opcode::Mod:
            return ABCKIT_ISA_API_STATIC_OPCODE_MOD;
        case compiler::Opcode::Div:
            return ABCKIT_ISA_API_STATIC_OPCODE_DIV;
        case compiler::Opcode::Neg:
            return ABCKIT_ISA_API_STATIC_OPCODE_NEG;
        case compiler::Opcode::AddI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ADDI;
        case compiler::Opcode::DivI:
            return ABCKIT_ISA_API_STATIC_OPCODE_DIVI;
        case compiler::Opcode::SubI:
            return ABCKIT_ISA_API_STATIC_OPCODE_SUBI;
        case compiler::Opcode::MulI:
            return ABCKIT_ISA_API_STATIC_OPCODE_MULI;
        case compiler::Opcode::ModI:
            return ABCKIT_ISA_API_STATIC_OPCODE_MODI;
        case compiler::Opcode::Shl:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHL;
        case compiler::Opcode::Shr:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHR;
        case compiler::Opcode::AShr:
            return ABCKIT_ISA_API_STATIC_OPCODE_ASHR;
        case compiler::Opcode::ShlI:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHLI;
        case compiler::Opcode::ShrI:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHRI;
        case compiler::Opcode::AShrI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ASHRI;
        case compiler::Opcode::And:
            return ABCKIT_ISA_API_STATIC_OPCODE_AND;
        case compiler::Opcode::Or:
            return ABCKIT_ISA_API_STATIC_OPCODE_OR;
        case compiler::Opcode::Xor:
            return ABCKIT_ISA_API_STATIC_OPCODE_XOR;
        case compiler::Opcode::AndI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ANDI;
        case compiler::Opcode::OrI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ORI;
        case compiler::Opcode::XorI:
            return ABCKIT_ISA_API_STATIC_OPCODE_XORI;
        case compiler::Opcode::Not:
            return ABCKIT_ISA_API_STATIC_OPCODE_NOT;
        case compiler::Opcode::LenArray:
            return ABCKIT_ISA_API_STATIC_OPCODE_LENARRAY;
        case compiler::Opcode::If:
            return ABCKIT_ISA_API_STATIC_OPCODE_IF;
        case compiler::Opcode::NullPtr:
            return ABCKIT_ISA_API_STATIC_OPCODE_NULLPTR;
        case compiler::Opcode::Phi:
            return ABCKIT_ISA_API_STATIC_OPCODE_PHI;
        case compiler::Opcode::LoadUndefined:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADUNDEFINED;
        case compiler::Opcode::Try:
            return ABCKIT_ISA_API_STATIC_OPCODE_TRY;
        case compiler::Opcode::CatchPhi:
            return ABCKIT_ISA_API_STATIC_OPCODE_CATCHPHI;
        case compiler::Opcode::Intrinsic:
            return GetStaticIntrinsicOpcode(inst->CastToIntrinsic());
        default:
            LIBABCKIT_LOG(DEBUG) << "compiler->abckit INVALID\n";
            return ABCKIT_ISA_API_STATIC_OPCODE_INVALID;
    }
    LIBABCKIT_UNREACHABLE;
}

AbckitIsaApiDynamicOpcode GetDynamicOpcode(compiler::Inst *inst)
{
    auto opcode = inst->GetOpcode();
    switch (opcode) {
        case compiler::Opcode::LoadString:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING;
        case compiler::Opcode::Parameter:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER;
        case compiler::Opcode::Constant:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT;
        case compiler::Opcode::If:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_IF;
        case compiler::Opcode::Phi:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_PHI;
        case compiler::Opcode::Try:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_TRY;
        case compiler::Opcode::CatchPhi:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_CATCHPHI;
        case compiler::Opcode::Intrinsic:
            switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
                case compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_STRING:
                    return ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING;
                default:
                    break;
            }
            return GetDynamicIntrinsicOpcode(inst->CastToIntrinsic());
        default:
            LIBABCKIT_LOG(DEBUG) << "compiler->abckit INVALID\n";
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID;
    }
    LIBABCKIT_UNREACHABLE;
}

size_t GetIntrinicMaxInputsCount(AbckitInst *inst)
{
    ASSERT(inst->impl->IsIntrinsic());
    if (IsDynamic(inst->graph->function->owningModule->target)) {
        return GetIntrinicMaxInputsCountDyn(inst->impl->CastToIntrinsic());
    }
    return GetIntrinicMaxInputsCountStatic(inst->impl->CastToIntrinsic());
}

bool IsCallInst(AbckitInst *inst)
{
    if (IsDynamic(inst->graph->function->owningModule->target)) {
        return IsCallInstDyn(inst->impl);
    }
    return IsCallInstStatic(inst->impl);
}

AbckitTypeId TypeToTypeId(compiler::DataType::Type type)
{
    LIBABCKIT_LOG(DEBUG) << "compiler->abckit\n";
    switch (type) {
        case compiler::DataType::Type::REFERENCE:
            return AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE;
        case compiler::DataType::Type::BOOL:
            return AbckitTypeId::ABCKIT_TYPE_ID_U1;
        case compiler::DataType::Type::UINT8:
            return AbckitTypeId::ABCKIT_TYPE_ID_U8;
        case compiler::DataType::Type::INT8:
            return AbckitTypeId::ABCKIT_TYPE_ID_I8;
        case compiler::DataType::Type::UINT16:
            return AbckitTypeId::ABCKIT_TYPE_ID_U16;
        case compiler::DataType::Type::INT16:
            return AbckitTypeId::ABCKIT_TYPE_ID_I16;
        case compiler::DataType::Type::UINT32:
            return AbckitTypeId::ABCKIT_TYPE_ID_U32;
        case compiler::DataType::Type::INT32:
            return AbckitTypeId::ABCKIT_TYPE_ID_I32;
        case compiler::DataType::Type::UINT64:
            return AbckitTypeId::ABCKIT_TYPE_ID_U64;
        case compiler::DataType::Type::INT64:
            return AbckitTypeId::ABCKIT_TYPE_ID_I64;
        case compiler::DataType::Type::FLOAT32:
            return AbckitTypeId::ABCKIT_TYPE_ID_F32;
        case compiler::DataType::Type::FLOAT64:
            return AbckitTypeId::ABCKIT_TYPE_ID_F64;
        case compiler::DataType::Type::ANY:
            return AbckitTypeId::ABCKIT_TYPE_ID_ANY;
        case compiler::DataType::Type::VOID:
            return AbckitTypeId::ABCKIT_TYPE_ID_VOID;
        case compiler::DataType::Type::POINTER:
        case compiler::DataType::Type::NO_TYPE:
        default:
            LIBABCKIT_LOG(DEBUG) << "compiler->abckit INVALID\n";
            return AbckitTypeId::ABCKIT_TYPE_ID_INVALID;
    }
    LIBABCKIT_UNREACHABLE;
}

compiler::DataType::Type TypeIdToType(AbckitTypeId typeId)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (typeId) {
        case AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE:
            return compiler::DataType::Type::REFERENCE;
        case AbckitTypeId::ABCKIT_TYPE_ID_U1:
            return compiler::DataType::Type::BOOL;
        case AbckitTypeId::ABCKIT_TYPE_ID_U8:
            return compiler::DataType::Type::UINT8;
        case AbckitTypeId::ABCKIT_TYPE_ID_I8:
            return compiler::DataType::Type::INT8;
        case AbckitTypeId::ABCKIT_TYPE_ID_U16:
            return compiler::DataType::Type::UINT16;
        case AbckitTypeId::ABCKIT_TYPE_ID_I16:
            return compiler::DataType::Type::INT16;
        case AbckitTypeId::ABCKIT_TYPE_ID_U32:
            return compiler::DataType::Type::UINT32;
        case AbckitTypeId::ABCKIT_TYPE_ID_I32:
            return compiler::DataType::Type::INT32;
        case AbckitTypeId::ABCKIT_TYPE_ID_U64:
            return compiler::DataType::Type::UINT64;
        case AbckitTypeId::ABCKIT_TYPE_ID_I64:
            return compiler::DataType::Type::INT64;
        case AbckitTypeId::ABCKIT_TYPE_ID_F32:
            return compiler::DataType::Type::FLOAT32;
        case AbckitTypeId::ABCKIT_TYPE_ID_F64:
            return compiler::DataType::Type::FLOAT64;
        case AbckitTypeId::ABCKIT_TYPE_ID_ANY:
            return compiler::DataType::Type::ANY;
        case AbckitTypeId::ABCKIT_TYPE_ID_VOID:
            return compiler::DataType::Type::VOID;
        case AbckitTypeId::ABCKIT_TYPE_ID_INVALID:
        default:
            LIBABCKIT_LOG(DEBUG) << "abckit->compiler INVALID\n";
            return compiler::DataType::Type::NO_TYPE;
    }
    LIBABCKIT_UNREACHABLE;
}

ark::compiler::ConditionCode CcToCc(AbckitIsaApiDynamicConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ:
            return ark::compiler::ConditionCode::CC_EQ;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE:
            return ark::compiler::ConditionCode::CC_NE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LT:
            return ark::compiler::ConditionCode::CC_LT;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LE:
            return ark::compiler::ConditionCode::CC_LE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT:
            return ark::compiler::ConditionCode::CC_GT;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GE:
            return ark::compiler::ConditionCode::CC_GE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_B:
            return ark::compiler::ConditionCode::CC_B;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_BE:
            return ark::compiler::ConditionCode::CC_BE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_A:
            return ark::compiler::ConditionCode::CC_A;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_AE:
            return ark::compiler::ConditionCode::CC_AE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_EQ:
            return ark::compiler::ConditionCode::CC_TST_EQ;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_NE:
            return ark::compiler::ConditionCode::CC_TST_NE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler CcToCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

ark::compiler::ConditionCode CcToCc(AbckitIsaApiStaticConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ:
            return ark::compiler::ConditionCode::CC_EQ;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE:
            return ark::compiler::ConditionCode::CC_NE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT:
            return ark::compiler::ConditionCode::CC_LT;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE:
            return ark::compiler::ConditionCode::CC_LE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT:
            return ark::compiler::ConditionCode::CC_GT;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE:
            return ark::compiler::ConditionCode::CC_GE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B:
            return ark::compiler::ConditionCode::CC_B;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE:
            return ark::compiler::ConditionCode::CC_BE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A:
            return ark::compiler::ConditionCode::CC_A;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE:
            return ark::compiler::ConditionCode::CC_AE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ:
            return ark::compiler::ConditionCode::CC_TST_EQ;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE:
            return ark::compiler::ConditionCode::CC_TST_NE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler CcToCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

AbckitIsaApiDynamicConditionCode CcToDynamicCc(ark::compiler::ConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case ark::compiler::ConditionCode::CC_EQ:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ;
        case ark::compiler::ConditionCode::CC_NE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE;
        case ark::compiler::ConditionCode::CC_LT:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LT;
        case ark::compiler::ConditionCode::CC_LE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LE;
        case ark::compiler::ConditionCode::CC_GT:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT;
        case ark::compiler::ConditionCode::CC_GE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GE;
        case ark::compiler::ConditionCode::CC_B:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_B;
        case ark::compiler::ConditionCode::CC_BE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_BE;
        case ark::compiler::ConditionCode::CC_A:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_A;
        case ark::compiler::ConditionCode::CC_AE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_AE;
        case ark::compiler::ConditionCode::CC_TST_EQ:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_EQ;
        case ark::compiler::ConditionCode::CC_TST_NE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_NE;
        default:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "compiler->abckit CcToDynamicCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

AbckitIsaApiStaticConditionCode CcToStaticCc(ark::compiler::ConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case ark::compiler::ConditionCode::CC_EQ:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ;
        case ark::compiler::ConditionCode::CC_NE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE;
        case ark::compiler::ConditionCode::CC_LT:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT;
        case ark::compiler::ConditionCode::CC_LE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE;
        case ark::compiler::ConditionCode::CC_GT:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT;
        case ark::compiler::ConditionCode::CC_GE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE;
        case ark::compiler::ConditionCode::CC_B:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B;
        case ark::compiler::ConditionCode::CC_BE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE;
        case ark::compiler::ConditionCode::CC_A:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A;
        case ark::compiler::ConditionCode::CC_AE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE;
        case ark::compiler::ConditionCode::CC_TST_EQ:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ;
        case ark::compiler::ConditionCode::CC_TST_NE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE;
        default:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "compiler->abckit CcToStaticCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

void SetLastError(AbckitStatus err)
{
    LIBABCKIT_LOG(DEBUG) << "error code: " << err << std::endl;
    statuses::SetLastError(err);
}

uint32_t GetClassOffset(AbckitGraph *graph, AbckitCoreClass *klass)
{
    ASSERT(!IsDynamic(graph->function->owningModule->target));

    auto *rec = klass->GetArkTSImpl()->impl.GetStaticClass();
    LIBABCKIT_LOG(DEBUG) << "className: " << rec->name << "\n";

    uint32_t classOffset = 0;
    for (auto &[id, s] : graph->irInterface->classes) {
        if (s == rec->name) {
            classOffset = id;
        }
    }
    if (classOffset == 0) {
        LIBABCKIT_LOG(DEBUG) << "classOffset == 0\n";
        LIBABCKIT_UNREACHABLE;
    }
    LIBABCKIT_LOG(DEBUG) << "classOffset: " << classOffset << "\n";
    return classOffset;
}

std::string GetFuncName(AbckitCoreFunction *function)
{
    std::string funcName = "__ABCKIT_INVALID__";

    if (IsDynamic(function->owningModule->target)) {
        auto *func = PandasmWrapper::GetWrappedFunction(GetDynFunction(function));
        funcName = func->name;
        delete func;
    } else {
        auto *func = reinterpret_cast<const pandasm::Function *>(function->GetArkTSImpl()->GetStaticImpl());
        funcName = func->name;
    }

    return funcName;
}

std::string GetMangleFuncName(AbckitCoreFunction *function)
{
    std::string funcName = "__ABCKIT_INVALID__";

    if (IsDynamic(function->owningModule->target)) {
        auto *func = PandasmWrapper::GetWrappedFunction(GetDynFunction(function));
        funcName = func->name;
        delete func;
    } else {
        auto *func = reinterpret_cast<const pandasm::Function *>(function->GetArkTSImpl()->GetStaticImpl());
        funcName = MangleFunctionName(pandasm::DeMangleName(func->name), func->params, func->returnType);
    }
    return funcName;
}

uint32_t GetMethodOffset(AbckitGraph *graph, AbckitCoreFunction *function)
{
    std::string funcName = GetMangleFuncName(function);
    ASSERT(funcName != "__ABCKIT_INVALID__");
    LIBABCKIT_LOG(DEBUG) << "functionName: " << funcName << "\n";

    uint32_t methodOffset = 0;
    for (auto &[id, s] : graph->irInterface->methods) {
        if (s == funcName) {
            methodOffset = id;
            break;
        }
    }
    if (methodOffset == 0) {
        LIBABCKIT_LOG(DEBUG) << "methodOffset == 0\n";
        LIBABCKIT_UNREACHABLE;
    }
    LIBABCKIT_LOG(DEBUG) << "methodOffset: " << methodOffset << "\n";
    return methodOffset;
}

uint32_t GetStringOffset(AbckitGraph *graph, AbckitString *string)
{
    uint32_t stringOffset = 0;
    for (auto &[id, s] : graph->irInterface->strings) {
        if (s == string->impl) {
            stringOffset = id;
        }
    }

    if (stringOffset == 0) {
        // Newly created string
        // NOLINTNEXTLINE(cert-msc51-cpp)
        do {
            LIBABCKIT_LOG(DEBUG) << "generating new stringOffset\n";
            // NOLINTNEXTLINE(cert-msc50-cpp)
            stringOffset++;
        } while (graph->irInterface->strings.find(stringOffset) != graph->irInterface->strings.end());
        // insert new string id
        graph->irInterface->strings.insert({stringOffset, string->impl.data()});
    }

    LIBABCKIT_LOG(DEBUG) << "stringOffset: " << stringOffset << "\n";
    return stringOffset;
}

uint32_t GetLiteralArrayOffset(AbckitGraph *graph, AbckitLiteralArray *arr)
{
    std::string arrName = "__ABCKIT_INVALID__";
    if (IsDynamic(graph->function->owningModule->target)) {
        auto litarrTable = PandasmWrapper::GetUnwrappedLiteralArrayTable(graph->file->GetDynamicProgram());
        for (auto &[id, s] : litarrTable) {
            if (s == arr->GetDynamicImpl()) {
                arrName = id;
            }
        }
    } else {
        auto *prog = graph->file->GetStaticProgram();
        for (auto &[id, s] : prog->literalarrayTable) {
            if (&s == arr->GetStaticImpl()) {
                arrName = id;
            }
        }
    }
    ASSERT(arrName != "__ABCKIT_INVALID__");

    uint32_t arrayOffset = 0;
    for (auto &[id, s] : graph->irInterface->literalarrays) {
        if (s == arrName) {
            arrayOffset = id;
        }
    }

    if (arrayOffset == 0) {
        // Newly created literal array
        // NOLINTNEXTLINE(cert-msc51-cpp)
        do {
            LIBABCKIT_LOG(DEBUG) << "generating new arrayOffset\n";
            // NOLINTNEXTLINE(cert-msc50-cpp)
            arrayOffset++;
        } while (graph->irInterface->literalarrays.find(arrayOffset) != graph->irInterface->literalarrays.end());
        // insert new literal array
        graph->irInterface->literalarrays.insert({arrayOffset, arrName});
    }
    return arrayOffset;
}

AbckitInst *CreateInstFromImpl(AbckitGraph *graph, compiler::Inst *impl)
{
    AbckitInst *newInst = graph->impl->GetLocalAllocator()->New<AbckitInst>(graph, impl);
    graph->implToInst.insert({impl, newInst});
    return newInst;
}

AbckitInst *CreateDynInst(AbckitGraph *graph, compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {3U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input2->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, compiler::DataType::Type retType,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(retType, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0, uint64_t imm1,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, uint64_t imm1,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01-CPP) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {4U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input2->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1, uint64_t imm2, AbckitInst *input0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm2);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, AbckitInst *input3, compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {5U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input2->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input3->impl, compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, std::va_list args,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {imm0 + 2U};
    intrImpl->ReserveInputs(imm0 + 2U);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, compiler::DataType::ANY);
    for (size_t index = 0; index < imm0; index++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, compiler::DataType::ANY);
    }
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {argCount + 1};
    intrImpl->ReserveInputs(argCount + 1);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, compiler::DataType::ANY);
    for (size_t index = 0; index < argCount; ++index) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, compiler::DataType::ANY);
    }
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), argCount);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, size_t argCount, std::va_list args,
                          compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {argCount};
    intrImpl->ReserveInputs(argCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    for (size_t index = 0; index < argCount; ++index) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, compiler::DataType::ANY);
    }
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), argCount);
    return CreateInstFromImpl(graph, intrImpl);
}

static void LiteralDeleter(pandasm_Literal *p)
{
    delete reinterpret_cast<pandasm::LiteralArray::Literal *>(p);
}

template <class T>
static AbckitLiteral *GetOrCreateLit(AbckitFile *file, std::unordered_map<T, std::unique_ptr<AbckitLiteral>> &cache,
                                     T value, panda_file::LiteralTag tagImpl)
{
    if (cache.count(value) == 1) {
        return cache[value].get();
    }

    auto *literal = new pandasm::LiteralArray::Literal();
    literal->tag = tagImpl;
    literal->value = value;
    auto abcLit = std::make_unique<AbckitLiteral>(
        file, AbckitLiteralImplT(reinterpret_cast<pandasm_Literal *>(literal), LiteralDeleter));
    cache.insert({value, std::move(abcLit)});
    return cache[value].get();
}

AbckitLiteral *GetOrCreateLiteralBoolStatic(AbckitFile *file, bool value)
{
    return GetOrCreateLit(file, file->literals.boolLits, value, panda_file::LiteralTag::ARRAY_U1);
}

AbckitLiteral *GetOrCreateLiteralU8Static(AbckitFile *file, uint8_t value)
{
    return GetOrCreateLit(file, file->literals.u8Lits, value, panda_file::LiteralTag::ARRAY_U8);
}

AbckitLiteral *GetOrCreateLiteralU16Static(AbckitFile *file, uint16_t value)
{
    return GetOrCreateLit(file, file->literals.u16Lits, value, panda_file::LiteralTag::ARRAY_U16);
}

AbckitLiteral *GetOrCreateLiteralMethodAffiliateStatic(AbckitFile *file, uint16_t value)
{
    return GetOrCreateLit(file, file->literals.methodAffilateLits, value, panda_file::LiteralTag::METHODAFFILIATE);
}

AbckitLiteral *GetOrCreateLiteralU32Static(AbckitFile *file, uint32_t value)
{
    return GetOrCreateLit(file, file->literals.u32Lits, value, panda_file::LiteralTag::ARRAY_U32);
}

AbckitLiteral *GetOrCreateLiteralU64Static(AbckitFile *file, uint64_t value)
{
    return GetOrCreateLit(file, file->literals.u64Lits, value, panda_file::LiteralTag::ARRAY_U64);
}

AbckitLiteral *GetOrCreateLiteralFloatStatic(AbckitFile *file, float value)
{
    return GetOrCreateLit(file, file->literals.floatLits, value, panda_file::LiteralTag::FLOAT);
}

AbckitLiteral *GetOrCreateLiteralDoubleStatic(AbckitFile *file, double value)
{
    return GetOrCreateLit(file, file->literals.doubleLits, value, panda_file::LiteralTag::DOUBLE);
}

AbckitLiteral *GetOrCreateLiteralStringStatic(AbckitFile *file, const std::string &value)
{
    return GetOrCreateLit(file, file->literals.stringLits, value, panda_file::LiteralTag::STRING);
}

AbckitLiteral *GetOrCreateLiteralMethodStatic(AbckitFile *file, const std::string &value)
{
    return GetOrCreateLit(file, file->literals.methodLits, value, panda_file::LiteralTag::METHOD);
}

void FixPreassignedRegisters(compiler::Inst *inst, compiler::Register reg, compiler::Register regLarge)
{
    for (size_t idx = 0; idx < inst->GetInputsCount(); idx++) {
        if (inst->GetSrcReg(idx) == reg) {
            inst->SetSrcReg(idx, regLarge);
        }
    }
    if (inst->GetDstReg() == reg) {
        inst->SetDstReg(regLarge);
    }
}

void FixPreassignedRegisters(compiler::Graph *graph)
{
    for (auto bb : graph->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            FixPreassignedRegisters(inst, compiler::INVALID_REG, compiler::INVALID_REG_LARGE);
            FixPreassignedRegisters(inst, compiler::INVALID_REG - 1U, compiler::INVALID_REG_LARGE - 1U);  // ACC_REG
        }
    }
}

bool AllocateRegisters(compiler::Graph *graph, uint8_t reservedReg)
{
    graph->RunPass<bytecodeopt::RegAccAlloc>();
    compiler::RegAllocResolver(graph).ResolveCatchPhis();
    if (!compiler::IsFrameSizeLarge()) {
        return graph->RunPass<compiler::RegAllocGraphColoring>(compiler::GetFrameSize());
    }
    compiler::LocationMask regMask(graph->GetLocalAllocator());
    regMask.Resize(compiler::GetFrameSize());
    regMask.Set(reservedReg);
    FixPreassignedRegisters(graph);
    return graph->RunPass<compiler::RegAllocGraphColoring>(regMask);
}

}  // namespace libabckit
