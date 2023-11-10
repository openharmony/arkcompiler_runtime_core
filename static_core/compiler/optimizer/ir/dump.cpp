/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "basicblock.h"
#include "compiler_options.h"
#include "inst.h"
#include "graph.h"
#include "dump.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/code_generator/target_info.h"

namespace panda::compiler {

// indent constants for dump instructions
static const int INDENT_ID = 6;
static const int INDENT_TYPE = 5;
static const int INDENT_OPCODE = 27;
static const int HEX_PTR_SIZE = sizeof(void *);

template <class T>
std::enable_if_t<std::is_integral_v<T>, ArenaString> ToArenaString(T value, ArenaAllocator *allocator)
{
    ArenaString res(std::to_string(value), allocator->Adapter());
    return res;
}

ArenaString GetId(uint32_t id, ArenaAllocator *allocator)
{
    return (id == INVALID_ID ? ArenaString("XX", allocator->Adapter()) : ToArenaString(id, allocator));
}

ArenaString IdToString(uint32_t id, ArenaAllocator *allocator, bool v_reg, bool is_phi)
{
    ArenaString reg(v_reg ? "v" : "", allocator->Adapter());
    ArenaString phi(is_phi ? "p" : "", allocator->Adapter());
    return reg + GetId(id, allocator) + phi;
}

// If print without brackets, then we print with space.
void PrintIfValidLocation(Location location, Arch arch, std::ostream *out, bool with_brackets = false)
{
    if (!location.IsInvalid() && !location.IsUnallocatedRegister()) {
        auto string = location.ToString(arch);
        if (with_brackets) {
            (*out) << "(" << string << ")";
        } else {
            (*out) << string << " ";
        }
    }
}

ArenaString InstId(const Inst *inst, ArenaAllocator *allocator)
{
    if (inst != nullptr) {
        if (inst->IsSaveState() && OPTIONS.IsCompilerDumpCompact()) {
            return ArenaString("ss", allocator->Adapter()) +
                   ArenaString(std::to_string(inst->GetId()), allocator->Adapter());
        }
        return IdToString(static_cast<uint32_t>(inst->GetId()), allocator, true, inst->IsPhi());
    }
    ArenaString null("null", allocator->Adapter());
    return null;
}

ArenaString BBId(const BasicBlock *block, ArenaAllocator *allocator)
{
    if (block != nullptr) {
        return IdToString(static_cast<uint32_t>(block->GetId()), allocator);
    }
    ArenaString null("null", allocator->Adapter());
    return null;
}

void DumpUsers(const Inst *inst, std::ostream *out)
{
    auto allocator = inst->GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto arch = inst->GetBasicBlock()->GetGraph()->GetArch();
    for (size_t i = 0; i < inst->GetDstCount(); ++i) {
        PrintIfValidLocation(inst->GetDstLocation(i), arch, out);
    }
    bool fl_first = true;
    for (auto &node_inst : inst->GetUsers()) {
        auto user = node_inst.GetInst();
        (*out) << (fl_first ? "(" : ", ") << InstId(user, allocator);
        if (fl_first) {
            fl_first = false;
        }
    }
    if (!fl_first) {
        (*out) << ')';
    }
}

ArenaString GetCondCodeToString(ConditionCode cc, ArenaAllocator *allocator)
{
    switch (cc) {
        case ConditionCode::CC_EQ:
            return ArenaString("EQ", allocator->Adapter());
        case ConditionCode::CC_NE:
            return ArenaString("NE", allocator->Adapter());

        case ConditionCode::CC_LT:
            return ArenaString("LT", allocator->Adapter());
        case ConditionCode::CC_LE:
            return ArenaString("LE", allocator->Adapter());
        case ConditionCode::CC_GT:
            return ArenaString("GT", allocator->Adapter());
        case ConditionCode::CC_GE:
            return ArenaString("GE", allocator->Adapter());

        case ConditionCode::CC_B:
            return ArenaString("B", allocator->Adapter());
        case ConditionCode::CC_BE:
            return ArenaString("BE", allocator->Adapter());
        case ConditionCode::CC_A:
            return ArenaString("A", allocator->Adapter());
        case ConditionCode::CC_AE:
            return ArenaString("AE", allocator->Adapter());

        case ConditionCode::CC_TST_EQ:
            return ArenaString("TST_EQ", allocator->Adapter());
        case ConditionCode::CC_TST_NE:
            return ArenaString("TST_NE", allocator->Adapter());
        default:
            UNREACHABLE();
    }
}

ArenaString PcToString(uint32_t pc, ArenaAllocator *allocator)
{
    std::ostringstream out_string;
    out_string << "bc: 0x" << std::setfill('0') << std::setw(HEX_PTR_SIZE) << std::hex << pc;
    return ArenaString(out_string.str(), allocator->Adapter());
}

void BBDependence(const char *type, const ArenaVector<BasicBlock *> &bb_vector, std::ostream *out,
                  ArenaAllocator *allocator)
{
    bool fl_first = true;
    (*out) << type << ": [";
    for (auto block_it : bb_vector) {
        (*out) << (fl_first ? "" : ", ") << "bb " << BBId(block_it, allocator);
        if (fl_first) {
            fl_first = false;
        }
    }
    (*out) << ']';
}

ArenaString FieldToString(RuntimeInterface *runtime, ObjectType type, RuntimeInterface::FieldPtr field,
                          ArenaAllocator *allocator)
{
    const auto &adapter = allocator->Adapter();
    if (type != ObjectType::MEM_STATIC && type != ObjectType::MEM_OBJECT) {
        return ArenaString(ObjectTypeToString(type), adapter);
    }

    if (!runtime->HasFieldMetadata(field)) {
        auto offset = runtime->GetFieldOffset(field);
        return ArenaString("Unknown.Unknown", adapter) + ArenaString(std::to_string(offset), adapter);
    }

    ArenaString dot(".", adapter);
    ArenaString cls_name(runtime->GetClassName(runtime->GetClassForField(field)), adapter);
    ArenaString field_name(runtime->GetFieldName(field), adapter);
    return cls_name + dot + field_name;
}

void DumpTypedFieldOpcode(std::ostream *out, Opcode opcode, uint32_t type_id, const ArenaString &field_name,
                          ArenaAllocator *allocator)
{
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opc(GetOpcodeString(opcode), adapter);
    ArenaString id(IdToString(type_id, allocator), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opc + space + id + space + field_name + space;
}

void DumpTypedOpcode(std::ostream *out, Opcode opcode, uint32_t type_id, ArenaAllocator *allocator)
{
    ArenaString space(" ", allocator->Adapter());
    ArenaString opc(GetOpcodeString(opcode), allocator->Adapter());
    ArenaString id(IdToString(type_id, allocator), allocator->Adapter());
    (*out) << std::setw(INDENT_OPCODE) << opc + space + id + space;
}

bool Inst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    auto arch = GetBasicBlock()->GetGraph()->GetArch();
    bool fl_first = true;
    unsigned i = 0;
    for (auto node_inst : GetInputs()) {
        Inst *input = node_inst.GetInst();
        (*out) << (fl_first ? "" : ", ") << InstId(input, allocator);
        PrintIfValidLocation(GetLocation(i), arch, out, true);
        i++;
        fl_first = false;
    }

    if (!GetTmpLocation().IsInvalid()) {
        (*out) << (fl_first ? "" : ", ") << "Tmp(" << GetTmpLocation().ToString(arch) << ")";
    }

    return !fl_first;
}

bool SaveStateInst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const char *sep = "";
    for (size_t i = 0; i < GetInputsCount(); i++) {
        (*out) << sep << std::dec << InstId(GetInput(i).GetInst(), allocator);
        if (GetVirtualRegister(i).IsSpecialReg()) {
            (*out) << "(" << VRegInfo::VRegTypeToString(GetVirtualRegister(i).GetVRegType()) << ")";
        } else if (GetVirtualRegister(i).IsBridge()) {
            (*out) << "(bridge)";
        } else {
            (*out) << "(vr" << GetVirtualRegister(i).Value() << ")";
        }
        sep = ", ";
    }
    if (GetImmediatesCount() > 0) {
        for (auto imm : *GetImmediates()) {
            (*out) << sep << std::hex << "0x" << imm.value;
            if (imm.vreg_type == VRegType::VREG) {
                (*out) << std::dec << "(vr" << imm.vreg << ")";
            } else {
                (*out) << "(" << VRegInfo::VRegTypeToString(imm.vreg_type) << ")";
            }
            sep = ", ";
        }
    }
    if (GetCallerInst() != nullptr) {
        (*out) << sep << "caller=" << GetCallerInst()->GetId();
    }
    (*out) << sep << "inlining_depth=" << GetInliningDepth();
    return true;
}

bool BinaryImmOperation::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool BinaryShiftedRegisterOperation::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", " << GetShiftTypeStr(GetShiftType()) << " 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool UnaryShiftedRegisterOperation::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", " << GetShiftTypeStr(GetShiftType()) << " 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool SelectImmInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool IfImmInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool PhiInst::DumpInputs(std::ostream *out) const
{
    const auto &allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    bool fl_first = true;
    for (size_t idx = 0; idx < GetInputsCount(); ++idx) {
        Inst *input = GetInput(idx).GetInst();
        auto block = GetPhiInputBb(idx);
        (*out) << (fl_first ? "" : ", ") << InstId(input, allocator) << "(bb" << BBId(block, allocator) << ")";
        if (fl_first) {
            fl_first = false;
        }
    }
    return !fl_first;
}

bool ConstantInst::DumpInputs(std::ostream *out) const
{
    switch (GetType()) {
        case DataType::Type::REFERENCE:
        case DataType::Type::BOOL:
        case DataType::Type::UINT8:
        case DataType::Type::INT8:
        case DataType::Type::UINT16:
        case DataType::Type::INT16:
        case DataType::Type::UINT32:
        case DataType::Type::INT32:
        case DataType::Type::UINT64:
        case DataType::Type::INT64:
            (*out) << "0x" << std::hex << GetIntValue() << std::dec;
            break;
        case DataType::Type::FLOAT32:
            (*out) << GetFloatValue();
            break;
        case DataType::Type::FLOAT64:
            (*out) << GetDoubleValue();
            break;
        case DataType::Type::ANY:
            (*out) << "0x" << std::hex << GetRawValue() << std::dec;
            break;
        default:
            UNREACHABLE();
    }
    return true;
}

bool SpillFillInst::DumpInputs(std::ostream *out) const
{
    bool first = true;
    for (auto spill_fill : GetSpillFills()) {
        if (!first) {
            (*out) << ", ";
        }
        first = false;
        (*out) << sf_data::ToString(spill_fill, GetBasicBlock()->GetGraph()->GetArch());
    }
    return true;
}

bool ParameterInst::DumpInputs(std::ostream *out) const
{
    auto arg_num = GetArgNumber();
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    ArenaString nums("nums", allocator->Adapter());
    (*out) << "arg " << ((arg_num == ParameterInst::DYNAMIC_NUM_ARGS) ? nums : IdToString(arg_num, allocator));
    return true;
}

void CompareInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

static void DumpOpcodeAnyTypeMixin(std::ostream &out, const Inst *inst)
{
    const auto *mixin_inst = static_cast<const AnyTypeMixin<FixedInputsInst1> *>(inst);
    ASSERT(mixin_inst != nullptr);
    auto allocator = mixin_inst->GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(mixin_inst->GetOpcode()), adapter);
    ArenaString any_base_type(AnyTypeTypeToString(mixin_inst->GetAnyType()), adapter);
    out << std::setw(INDENT_OPCODE)
        << opcode + space + any_base_type + (mixin_inst->IsIntegerWasSeen() ? " i" : "") +
               (mixin_inst->IsSpecialWasSeen() ? " s" : "") + (mixin_inst->IsTypeWasProfiled() ? " p" : "") + space;
}

void PhiInst::DumpOpcode(std::ostream *out) const
{
    if (GetBasicBlock()->GetGraph()->IsDynamicMethod()) {
        DumpOpcodeAnyTypeMixin(*out, this);
    } else {
        Inst::DumpOpcode(out);
    }
}

void CompareAnyTypeInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void GetAnyTypeNameInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void CastAnyTypeValueInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void CastValueToAnyTypeInst::DumpOpcode(std::ostream *out) const
{
    DumpOpcodeAnyTypeMixin(*out, this);
}

void AnyTypeCheckInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString any_base_type(AnyTypeTypeToString(GetAnyType()), adapter);
    (*out) << std::setw(INDENT_OPCODE)
           << (opcode + space + any_base_type + (IsIntegerWasSeen() ? " i" : "") + (IsSpecialWasSeen() ? " s" : "") +
               (IsTypeWasProfiled() ? " p" : "") + space);
}

void HclassCheckInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString open("[", adapter);
    ArenaString close("]", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    bool is_first = true;
    ArenaString summary = opcode + space + open;
    if (GetCheckIsFunction()) {
        summary += ArenaString("IsFunc", adapter);
        is_first = false;
    }
    if (GetCheckFunctionIsNotClassConstructor()) {
        if (!is_first) {
            summary += ArenaString(", ", adapter);
        }
        summary += ArenaString("IsNotClassConstr", adapter);
    }
    summary += close + space;
    (*out) << std::setw(INDENT_OPCODE) << summary;
}

void LoadImmediateInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString open("(", adapter);
    ArenaString close(") ", adapter);
    if (IsClass()) {
        ArenaString type("class: ", adapter);
        ArenaString class_name(GetBasicBlock()->GetGraph()->GetRuntime()->GetClassName(GetObject()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type + class_name + close;
    } else if (IsMethod()) {
        ArenaString type("method: ", adapter);
        ArenaString method_name(GetBasicBlock()->GetGraph()->GetRuntime()->GetMethodName(GetObject()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type + method_name + close;
    } else if (IsString()) {
        ArenaString type("string: 0x", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetString() << close;
    } else if (IsPandaFileOffset()) {
        ArenaString type("PandaFileOffset: ", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetPandaFileOffset() << close;
    } else if (IsConstantPool()) {
        ArenaString type("constpool: 0x", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + open + type << std::hex << GetConstantPool() << close;
    } else {
        UNREACHABLE();
    }
}

void FunctionImmediateInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString prefix(" 0x", adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode << prefix << std::hex << GetFunctionPtr() << " ";
}

void LoadObjFromConstInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString prefix(" 0x", adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode << prefix << std::hex << GetObjPtr() << " ";
}

void SelectInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void SelectImmInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void IfInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void IfImmInst::DumpOpcode(std::ostream *out) const
{
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString cc(GetCondCodeToString(GetCc(), allocator), adapter);
    ArenaString type(DataType::ToString(GetOperandsType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + cc + space + type;
}

void MonitorInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString suffix(IsExit() ? ".Exit" : ".Entry", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + suffix;
}

void CmpInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    auto type = GetOperandsType();
    ArenaString suffix = ArenaString(" ", adapter) + ArenaString(DataType::ToString(type), adapter);
    if (IsFloatType(type)) {
        (*out) << std::setw(INDENT_OPCODE) << ArenaString(IsFcmpg() ? "Fcmpg" : "Fcmpl", adapter) + suffix;
    } else if (IsTypeSigned(type)) {
        (*out) << std::setw(INDENT_OPCODE) << ArenaString("Cmp", adapter) + ArenaString(" ", adapter) + suffix;
    } else {
        (*out) << std::setw(INDENT_OPCODE) << ArenaString("Ucmp", adapter) + suffix;
    }
}

void CastInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString space(" ", adapter);
    (*out) << std::setw(INDENT_OPCODE)
           << (ArenaString(GetOpcodeString(GetOpcode()), adapter) + space +
               ArenaString(DataType::ToString(GetOperandsType()), adapter));
}

void NewObjectInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void NewArrayInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadConstArrayInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void FillConstArrayInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadObjectInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto field_name = FieldToString(graph->GetRuntime(), GetObjectType(), GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), field_name, graph->GetLocalAllocator());
}

void LoadMemInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetType(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void ResolveObjectFieldInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadResolvedObjectFieldInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreObjectInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto field_name = FieldToString(graph->GetRuntime(), GetObjectType(), GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), field_name, graph->GetLocalAllocator());
}

void StoreResolvedObjectFieldInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreMemInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetType(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadStaticInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto field_name =
        FieldToString(graph->GetRuntime(), ObjectType::MEM_STATIC, GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), field_name, graph->GetLocalAllocator());
}

void ResolveObjectFieldStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadResolvedObjectFieldStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreStaticInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto field_name =
        FieldToString(graph->GetRuntime(), ObjectType::MEM_STATIC, GetObjField(), graph->GetLocalAllocator());
    DumpTypedFieldOpcode(out, GetOpcode(), GetTypeId(), field_name, graph->GetLocalAllocator());
}

void UnresolvedStoreStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void StoreResolvedObjectFieldStaticInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadFromPool::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void LoadFromPoolDynamic::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void ClassInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();

    ArenaString space(" ", adapter);
    ArenaString qt("'", adapter);
    ArenaString opc(GetOpcodeString(GetOpcode()), adapter);
    ArenaString class_name(GetClass() == nullptr ? ArenaString("", adapter)
                                                 : ArenaString(graph->GetRuntime()->GetClassName(GetClass()), adapter));
    (*out) << std::setw(INDENT_OPCODE) << opc + space + qt + class_name + qt << " ";
}

void RuntimeClassInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();

    ArenaString space(" ", adapter);
    ArenaString qt("'", adapter);
    ArenaString opc(GetOpcodeString(GetOpcode()), adapter);
    ArenaString class_name(GetClass() == nullptr ? ArenaString("", adapter)
                                                 : ArenaString(graph->GetRuntime()->GetClassName(GetClass()), adapter));
    (*out) << std::setw(INDENT_OPCODE) << opc + space + qt + class_name + qt << " ";
}

void GlobalVarInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void CheckCastInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void IsInstanceInst::DumpOpcode(std::ostream *out) const
{
    DumpTypedOpcode(out, GetOpcode(), GetTypeId(), GetBasicBlock()->GetGraph()->GetLocalAllocator());
}

void IntrinsicInst::DumpOpcode(std::ostream *out) const
{
    const auto &adapter = GetBasicBlock()->GetGraph()->GetLocalAllocator()->Adapter();
    ArenaString intrinsic(IsBuiltin() ? ArenaString("BuiltinIntrinsic.", adapter) : ArenaString("Intrinsic.", adapter));
    ArenaString opcode(GetIntrinsicName(intrinsic_id_), adapter);
    (*out) << std::setw(INDENT_OPCODE) << intrinsic + opcode << " ";
}

void Inst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString flags("", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    if (CanDeoptimize()) {
        flags += "D";
    }
    if (GetFlag(inst_flags::MEM_BARRIER)) {
        static constexpr auto FENCE = "F";
        flags += FENCE;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    (*out) << std::setw(INDENT_OPCODE) << opcode + space + flags;
}

void ResolveStaticInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString method_id(ToArenaString(GetCallMethodId(), allocator));
    if (GetCallMethod() != nullptr) {
        ArenaString method(graph->GetRuntime()->GetMethodFullName(GetCallMethod()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + method_id + ' ' + method << ' ';
    } else {
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + method_id << ' ';
    }
}

void ResolveVirtualInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString method_id(ToArenaString(GetCallMethodId(), allocator));
    if (GetCallMethod() != nullptr) {
        ArenaString method(graph->GetRuntime()->GetMethodFullName(GetCallMethod()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + method_id + ' ' + method << ' ';
    } else {
        (*out) << std::setw(INDENT_OPCODE) << opcode + ' ' + method_id << ' ';
    }
}

void InitStringInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    if (IsFromString()) {
        ArenaString mode(" FromString", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + mode << ' ';
    } else {
        ASSERT(IsFromCharArray());
        ArenaString mode(" FromCharArray", adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + mode << ' ';
    }
}

void CallInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString space(" ", adapter);
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString inlined(IsInlined() ? ".Inlined " : " ", adapter);
    ArenaString method_id(ToArenaString(GetCallMethodId(), allocator));
    if (!IsUnresolved() && GetCallMethod() != nullptr) {
        ArenaString method(graph->GetRuntime()->GetMethodFullName(GetCallMethod()), adapter);
        (*out) << std::setw(INDENT_OPCODE) << opcode + inlined + method_id + ' ' + method << ' ';
    } else {
        (*out) << std::setw(INDENT_OPCODE) << opcode + inlined + method_id << ' ';
    }
}

void DeoptimizeInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString type(DeoptimizeTypeToString(GetDeoptimizeType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + ArenaString(" ", adapter) + type << ' ';
}

void DeoptimizeIfInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(GetOpcodeString(GetOpcode()), adapter);
    ArenaString type(DeoptimizeTypeToString(GetDeoptimizeType()), adapter);
    (*out) << std::setw(INDENT_OPCODE) << opcode + ArenaString(" ", adapter) + type << ' ';
}

void DeoptimizeCompareInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(ArenaString(GetOpcodeString(GetOpcode()), adapter).append(" "));
    ArenaString cc(ArenaString(GetCondCodeToString(GetCc(), allocator), adapter).append(" "));
    ArenaString type(ArenaString(DeoptimizeTypeToString(GetDeoptimizeType()), adapter).append(" "));
    ArenaString cmp_type(ArenaString(DataType::ToString(GetOperandsType()), adapter).append(" "));
    (*out) << std::setw(INDENT_OPCODE) << opcode.append(cc).append(cmp_type).append(type);
}

void DeoptimizeCompareImmInst::DumpOpcode(std::ostream *out) const
{
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();
    const auto &adapter = allocator->Adapter();
    ArenaString opcode(ArenaString(GetOpcodeString(GetOpcode()), adapter).append(" "));
    ArenaString cc(ArenaString(GetCondCodeToString(GetCc(), allocator), adapter).append(" "));
    ArenaString type(ArenaString(DeoptimizeTypeToString(GetDeoptimizeType()), adapter).append(" "));
    ArenaString cmp_type(ArenaString(DataType::ToString(GetOperandsType()), adapter).append(" "));
    (*out) << std::setw(INDENT_OPCODE) << opcode.append(cc).append(cmp_type).append(type);
}

bool DeoptimizeCompareImmInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool BoundsCheckInstI::DumpInputs(std::ostream *out) const
{
    Inst *len_input = GetInput(0).GetInst();
    Inst *ss_input = GetInput(1).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();

    (*out) << InstId(len_input, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(ss_input, allocator);
    return true;
}

bool StoreInstI::DumpInputs(std::ostream *out) const
{
    Inst *arr_input = GetInput(0).GetInst();
    Inst *ss_input = GetInput(1).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    auto arch = graph->GetArch();
    const auto &allocator = graph->GetLocalAllocator();

    (*out) << InstId(arr_input, allocator);
    PrintIfValidLocation(GetLocation(0), arch, out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(ss_input, allocator);
    PrintIfValidLocation(GetLocation(1), arch, out, true);
    return true;
}

bool StoreMemInstI::DumpInputs(std::ostream *out) const
{
    Inst *arr_input = GetInput(0).GetInst();
    Inst *ss_input = GetInput(1).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    const auto &allocator = graph->GetLocalAllocator();

    (*out) << InstId(arr_input, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(ss_input, allocator);
    return true;
}

bool LoadInstI::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool LoadMemInstI::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool LoadMemInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    if (GetScale() != 0) {
        (*out) << " Scale " << GetScale();
    }
    return true;
}

bool StoreMemInst::DumpInputs(std::ostream *out) const
{
    Inst::DumpInputs(out);
    if (GetScale() != 0) {
        (*out) << " Scale " << GetScale();
    }
    return true;
}

bool LoadPairPartInst::DumpInputs(std::ostream *out) const
{
    Inst *arr_input = GetInput(0).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    const auto &allocator = graph->GetLocalAllocator();

    (*out) << InstId(arr_input, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool LoadArrayPairInstI::DumpInputs(std::ostream *out) const
{
    Inst *arr_input = GetInput(0).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    const auto &allocator = graph->GetLocalAllocator();
    (*out) << InstId(arr_input, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    return true;
}

bool StoreArrayPairInstI::DumpInputs(std::ostream *out) const
{
    Inst *arr_input = GetInput(0).GetInst();
    Inst *fss_input = GetInput(1).GetInst();
    constexpr auto IMM_2 = 2;
    Inst *sss_input = GetInput(IMM_2).GetInst();
    auto graph = GetBasicBlock()->GetGraph();
    auto allocator = graph->GetLocalAllocator();

    (*out) << InstId(arr_input, allocator);
    PrintIfValidLocation(GetLocation(0), graph->GetArch(), out, true);
    (*out) << ", 0x" << std::hex << GetImm() << std::dec;
    (*out) << ", " << InstId(fss_input, allocator);
    (*out) << ", " << InstId(sss_input, allocator);
    return true;
}

bool ReturnInstI::DumpInputs(std::ostream *out) const
{
    (*out) << "0x" << std::hex << GetImm() << std::dec;
    return true;
}

void IntrinsicInst::DumpImms(std::ostream *out) const
{
    if (!HasImms()) {
        return;
    }

    const auto &imms = GetImms();
    ASSERT(!imms.empty());
    (*out) << "0x" << std::hex << imms[0U];
    for (size_t i = 1U; i < imms.size(); ++i) {
        (*out) << ", 0x" << imms[i];
    }
    (*out) << ' ' << std::dec;
}

bool IntrinsicInst::DumpInputs(std::ostream *out) const
{
    DumpImms(out);
    return Inst::DumpInputs(out);
}

void Inst::DumpBytecode(std::ostream *out) const
{
    if (pc_ != INVALID_PC) {
        auto graph = GetBasicBlock()->GetGraph();
        auto byte_code = graph->GetRuntime()->GetBytecodeString(graph->GetMethod(), pc_);
        if (!byte_code.empty()) {
            (*out) << byte_code << '\n';
        }
    }
}

#ifdef PANDA_COMPILER_DEBUG_INFO
void Inst::DumpSourceLine(std::ostream *out) const
{
    auto current_method = GetCurrentMethod();
    auto pc = GetPc();
    if (current_method != nullptr && pc != INVALID_PC) {
        auto line = GetBasicBlock()->GetGraph()->GetRuntime()->GetLineNumberAndSourceFile(current_method, pc);
        (*out) << " (" << line << " )";
    }
}
#endif  // PANDA_COMPILER_DEBUG_INFO

void Inst::Dump(std::ostream *out, bool new_line) const
{
    if (OPTIONS.IsCompilerDumpCompact() && IsSaveState()) {
        return;
    }
    auto allocator = GetBasicBlock()->GetGraph()->GetLocalAllocator();
    // Id
    (*out) << std::setw(INDENT_ID) << std::setfill(' ') << std::right
           << IdToString(id_, allocator, false, IsPhi()) + '.';
    // Type
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    (*out) << std::setw(INDENT_TYPE) << std::left << DataType::ToString(GetType());
    // opcode
    DumpOpcode(out);
    auto operands_pos = out->tellp();
    // inputs
    bool has_input = DumpInputs(out);
    // users
    if (has_input && !GetUsers().Empty()) {
        (*out) << " -> ";
    }
    DumpUsers(this, out);
    // Align rest of the instruction info
    static constexpr auto ALIGN_BUF_SIZE = 64;
    if (auto pos_diff = out->tellp() - operands_pos; pos_diff < ALIGN_BUF_SIZE) {
        pos_diff = ALIGN_BUF_SIZE - pos_diff;
        static std::array<char, ALIGN_BUF_SIZE + 1> space_buf;
        if (space_buf[0] != ' ') {
            std::fill(space_buf.begin(), space_buf.end(), ' ');
        }
        space_buf[pos_diff] = 0;
        (*out) << space_buf.data();
        space_buf[pos_diff] = ' ';
    }
    // bytecode pointer
    if (pc_ != INVALID_PC && !OPTIONS.IsCompilerDumpCompact()) {
        (*out) << ' ' << PcToString(pc_, allocator);
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    if (OPTIONS.IsCompilerDumpSourceLine()) {
        DumpSourceLine(out);
    }
#endif
    if (new_line) {
        (*out) << '\n';
    }
    if (OPTIONS.IsCompilerDumpBytecode()) {
        DumpBytecode(out);
    }
    if (GetOpcode() == Opcode::Parameter) {
        auto spill_fill = static_cast<const ParameterInst *>(this)->GetLocationData();
        if (spill_fill.DstValue() != INVALID_REG) {
            (*out) << sf_data::ToString(spill_fill, GetBasicBlock()->GetGraph()->GetArch());
            if (new_line) {
                *out << std::endl;
            }
        }
    }
}

void CheckPrintPropsFlag(std::ostream *out, bool *print_props_flag)
{
    if (!(*print_props_flag)) {
        (*out) << "prop: ";
        (*print_props_flag) = true;
    } else {
        (*out) << ", ";
    }
}

void PrintLoopInfo(std::ostream *out, Loop *loop)
{
    (*out) << "loop" << (loop->IsIrreducible() ? " (irreducible) " : " ") << loop->GetId();
    if (loop->GetDepth() > 0) {
        (*out) << ", depth " << loop->GetDepth();
    }
}

void BlockProps(const BasicBlock *block, std::ostream *out)
{
    bool print_props_flag = false;
    if (block->IsStartBlock()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "start";
    }
    if (block->IsEndBlock()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "end";
    }
    if (block->IsLoopPreHeader()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "prehead";
    }
    if (block->IsLoopValid() && !block->GetLoop()->IsRoot()) {
        if (block->IsLoopHeader()) {
            CheckPrintPropsFlag(out, &print_props_flag);
            (*out) << "head";
        }
        CheckPrintPropsFlag(out, &print_props_flag);
        PrintLoopInfo(out, block->GetLoop());
    }
    if (block->IsTryBegin()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "try_begin (id " << block->GetTryId() << ")";
    }
    if (block->IsTry()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "try (id " << block->GetTryId() << ")";
    }
    if (block->IsTryEnd()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "try_end (id " << block->GetTryId() << ")";
    }
    if (block->IsCatchBegin()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "catch_begin";
    }
    if (block->IsCatch()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "catch";
    }
    if (block->IsCatchEnd()) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << "catch_end";
    }

    if (block->GetGuestPc() != INVALID_PC) {
        CheckPrintPropsFlag(out, &print_props_flag);
        (*out) << PcToString(block->GetGuestPc(), block->GetGraph()->GetLocalAllocator());
    }
    if (print_props_flag) {
        (*out) << std::endl;
    }
}

void BasicBlock::Dump(std::ostream *out) const
{
    const auto &allocator = GetGraph()->GetLocalAllocator();
    (*out) << "BB " << IdToString(bb_id_, allocator);
    // predecessors
    if (!preds_.empty()) {
        (*out) << "  ";
        BBDependence("preds", preds_, out, allocator);
    }
    (*out) << '\n';
    // properties
    BlockProps(this, out);
    // instructions
    for (auto inst : this->AllInsts()) {
        inst->Dump(out);
    }
    // successors
    if (!succs_.empty()) {
        BBDependence("succs", succs_, out, allocator);
        (*out) << '\n';
    }
}

void Graph::Dump(std::ostream *out) const
{
    const auto &runtime = GetRuntime();
    const auto &method = GetMethod();
    const auto &adapter = GetLocalAllocator()->Adapter();
    ArenaString return_type(DataType::ToString(runtime->GetMethodReturnType(method)), adapter);
    (*out) << "Method: " << runtime->GetMethodFullName(method, true) << " " << method << std::endl;
    if (IsOsrMode()) {
        (*out) << "OSR mode\n";
    }
    (*out) << std::endl;

    auto &blocks = GetAnalysis<LinearOrder>().IsValid() ? GetBlocksLinearOrder() : GetBlocksRPO();
    for (const auto &block_it : blocks) {
        if (!block_it->GetPredsBlocks().empty() || !block_it->GetSuccsBlocks().empty()) {
            block_it->Dump(out);
            (*out) << '\n';
        } else {
            // to print the dump before cleanup, still unconnected nodes exist
            (*out) << "BB " << block_it->GetId() << " is unconnected\n\n";
        }
    }
}
}  // namespace panda::compiler
