/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "assembler/assembly-literals.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/optimizations/peepholes.h"
#include "const_array_resolver.h"

namespace panda::bytecodeopt {

static constexpr size_t STOREARRAY_INPUTS_NUM = 3;
static constexpr size_t SINGLE_DIM_ARRAY_RANK = 1;
static constexpr size_t MIN_ARRAY_ELEMENTS_AMOUNT = 2;

bool ConstArrayResolver::RunImpl()
{
    if (ir_interface_ == nullptr) {
        return false;
    }

    if (!FindConstantArrays()) {
        return false;
    }

    // delete instructions for storing array elements
    RemoveArraysFill();

    // replace old NewArray instructions with new SaveState + LoadConst instructions
    InsertLoadConstArrayInsts();

    return true;
}

static bool IsPatchAllowedOpcode(Opcode opcode)
{
    switch (opcode) {
        case Opcode::StoreArray:
        case Opcode::LoadString:
        case Opcode::Constant:
        case Opcode::Cast:
        case Opcode::SaveState:
            return true;
        default:
            return false;
    }
}

std::optional<std::vector<pandasm::LiteralArray::Literal>> ConstArrayResolver::FillLiteralArray(Inst *inst, size_t size)
{
    std::vector<pandasm::LiteralArray::Literal> literals;
    std::vector<Inst *> store_insts;

    auto next = inst->GetNext();
    size_t index = 0;

    // are looking for instructions for uninterrupted filling the array
    while ((next != nullptr) && (index < size)) {
        // check whether the instruction is allowed inside the filling patch
        if (!IsPatchAllowedOpcode(next->GetOpcode())) {
            break;
        }

        // find instructions for storing array elements
        if (next->GetOpcode() != Opcode::StoreArray) {
            next = next->GetNext();
            continue;
        }

        auto store_array_inst = next->CastToStoreArray();
        if (store_array_inst->GetArray() != inst) {
            break;
        }

        // create a literal from the array element, if possible
        pandasm::LiteralArray::Literal literal;
        if (!FillLiteral(store_array_inst, &literal)) {
            // if not, then we can't create a constant literal array
            return std::nullopt;
        }
        literals.push_back(literal);
        store_insts.push_back(next);

        index++;
        next = next->GetNext();
    }

    // save the literal array only if it is completely filled
    // or its size exceeds the minimum number of elements to save
    if ((index < size) || (store_insts.size() < MIN_ARRAY_ELEMENTS_AMOUNT)) {
        return std::nullopt;
    }

    // save the store instructions for deleting them later
    const_arrays_fill_.emplace(inst, std::move(store_insts));
    return std::optional<std::vector<pandasm::LiteralArray::Literal>> {std::move(literals)};
}

void ConstArrayResolver::AddIntroLiterals(pandasm::LiteralArray *lt_ar)
{
    // add an element that stores the array size (it will be stored in the first element)
    pandasm::LiteralArray::Literal len_lit;
    len_lit.tag_ = panda_file::LiteralTag::INTEGER;
    len_lit.value_ = static_cast<uint32_t>(lt_ar->literals_.size());
    lt_ar->literals_.insert(lt_ar->literals_.begin(), len_lit);

    // add an element that stores the array type (it will be stored in the zero element)
    pandasm::LiteralArray::Literal tag_lit;
    tag_lit.tag_ = panda_file::LiteralTag::TAGVALUE;
    tag_lit.value_ = static_cast<uint8_t>(lt_ar->literals_.back().tag_);
    lt_ar->literals_.insert(lt_ar->literals_.begin(), tag_lit);
}

bool ConstArrayResolver::IsMultidimensionalArray(compiler::NewArrayInst *inst)
{
    auto array_type = pandasm::Type::FromName(ir_interface_->GetTypeIdByOffset(inst->GetTypeId()));
    return array_type.GetRank() > SINGLE_DIM_ARRAY_RANK;
}

static bool IsSameBB(Inst *inst1, Inst *inst2)
{
    return inst1->GetBasicBlock() == inst2->GetBasicBlock();
}

static bool IsSameBB(Inst *inst, compiler::BasicBlock *bb)
{
    return inst->GetBasicBlock() == bb;
}

static std::optional<compiler::ConstantInst *> GetConstantIfPossible(Inst *inst)
{
    if (inst->GetOpcode() == Opcode::Cast) {
        auto input = inst->GetInput(0).GetInst();
        if ((input->GetOpcode() == Opcode::NullPtr) || !input->IsConst()) {
            return std::nullopt;
        }
        auto constant_inst = compiler::ConstFoldingCastConst(inst, input, true);
        return constant_inst == nullptr ? std::nullopt : std::optional<compiler::ConstantInst *>(constant_inst);
    }
    if (inst->IsConst()) {
        return std::optional<compiler::ConstantInst *>(inst->CastToConstant());
    }
    return std::nullopt;
}

bool ConstArrayResolver::FindConstantArrays()
{
    size_t init_size = ir_interface_->GetLiteralArrayTableSize();

    for (auto bb : GetGraph()->GetBlocksRPO()) {
        // go through the instructions of the basic block in reverse order
        // until we meet the instruction for storing an array element
        auto inst = bb->GetLastInst();
        while ((inst != nullptr) && IsSameBB(inst, bb)) {
            if (inst->GetOpcode() != Opcode::StoreArray) {
                inst = inst->GetPrev();
                continue;
            }

            // the patch for creating and filling an array should start with the NewArray instruction
            auto array_inst = inst->CastToStoreArray()->GetArray();
            if (array_inst->GetOpcode() != Opcode::NewArray) {
                inst = inst->GetPrev();
                continue;
            }
            auto new_array_inst = array_inst->CastToNewArray();

            // the instructions included in the patch must be in one basic block
            if (!IsSameBB(inst, new_array_inst)) {
                inst = inst->GetPrev();
                continue;
            }

            // TODO(aantipina): add the ability to save multidimensional arrays
            if (IsMultidimensionalArray(new_array_inst)) {
                if (IsSameBB(inst, new_array_inst)) {
                    inst = new_array_inst->GetPrev();
                } else {
                    inst = inst->GetPrev();
                }
                continue;
            }

            auto array_size_inst =
                GetConstantIfPossible(new_array_inst->GetInput(compiler::NewArrayInst::INDEX_SIZE).GetInst());
            if (array_size_inst == std::nullopt) {
                inst = new_array_inst->GetPrev();
                continue;
            }
            auto array_size = (*array_size_inst)->CastToConstant()->GetIntValue();
            if (array_size < MIN_ARRAY_ELEMENTS_AMOUNT) {
                inst = new_array_inst->GetPrev();
                continue;
            }

            // creating a literal array, if possible
            auto raw_literal_array = FillLiteralArray(new_array_inst, array_size);
            if (raw_literal_array == std::nullopt) {
                inst = new_array_inst->GetPrev();
                continue;
            }

            pandasm::LiteralArray literal_array(*raw_literal_array);

            // save the type and length of the array in the first two elements
            AddIntroLiterals(&literal_array);
            auto id = ir_interface_->GetLiteralArrayTableSize();
            ir_interface_->StoreLiteralArray(std::to_string(id), std::move(literal_array));

            // save the NewArray instructions for replacing them with LoadConst instructions later
            const_arrays_init_.emplace(id, new_array_inst);

            inst = new_array_inst->GetPrev();
        }
    }

    // the pass worked if the size of the literal array table increased
    return init_size < ir_interface_->GetLiteralArrayTableSize();
}

void ConstArrayResolver::RemoveArraysFill()
{
    for (const auto &it : const_arrays_fill_) {
        for (const auto &store_inst : it.second) {
            store_inst->GetBasicBlock()->RemoveInst(store_inst);
        }
    }
}

void ConstArrayResolver::InsertLoadConstArrayInsts()
{
    for (const auto &[id, start_inst] : const_arrays_init_) {
        auto method = GetGraph()->GetMethod();
        compiler::LoadConstArrayInst *new_inst = GetGraph()->CreateInstLoadConstArray(REFERENCE, start_inst->GetPc());
        new_inst->SetTypeId(id);
        new_inst->SetMethod(method);

        start_inst->ReplaceUsers(new_inst);
        start_inst->RemoveInputs();

        compiler::SaveStateInst *save_state = GetGraph()->CreateInstSaveState();
        save_state->SetPc(start_inst->GetPc());
        save_state->SetMethod(method);
        save_state->ReserveInputs(0);

        new_inst->SetInput(0, save_state);
        start_inst->InsertBefore(save_state);
        start_inst->GetBasicBlock()->ReplaceInst(start_inst, new_inst);
    }
}

static bool FillPrimitiveLiteral(pandasm::LiteralArray::Literal *literal, panda_file::Type::TypeId type,
                                 compiler::ConstantInst *value_inst)
{
    auto tag = pandasm::LiteralArray::GetArrayTagFromComponentType(type);
    literal->tag_ = tag;
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1:
            literal->value_ = static_cast<bool>(value_inst->GetInt32Value());
            return true;
        case panda_file::LiteralTag::ARRAY_U8:
        case panda_file::LiteralTag::ARRAY_I8:
            literal->value_ = static_cast<uint8_t>(value_inst->GetInt32Value());
            return true;
        case panda_file::LiteralTag::ARRAY_U16:
        case panda_file::LiteralTag::ARRAY_I16:
            literal->value_ = static_cast<uint16_t>(value_inst->GetInt32Value());
            return true;
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_I32:
            literal->value_ = value_inst->GetInt32Value();
            return true;
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_I64:
            literal->value_ = value_inst->GetInt64Value();
            return true;
        case panda_file::LiteralTag::ARRAY_F32:
            literal->value_ = value_inst->GetFloatValue();
            return true;
        case panda_file::LiteralTag::ARRAY_F64:
            literal->value_ = value_inst->GetDoubleValue();
            return true;
        default:
            UNREACHABLE();
    }
    return false;
}

bool ConstArrayResolver::FillLiteral(compiler::StoreInst *store_array_inst, pandasm::LiteralArray::Literal *literal)
{
    if (store_array_inst->GetInputsCount() > STOREARRAY_INPUTS_NUM) {
        return false;
    }
    auto raw_elem_inst = store_array_inst->GetStoredValue();
    auto new_array_inst = store_array_inst->GetArray();

    auto array_type =
        pandasm::Type::FromName(ir_interface_->GetTypeIdByOffset(new_array_inst->CastToNewArray()->GetTypeId()));
    auto component_type = array_type.GetComponentType();
    auto component_type_name = array_type.GetComponentName();

    if (pandasm::Type::IsPandaPrimitiveType(component_type_name)) {
        auto value_inst = GetConstantIfPossible(raw_elem_inst);

        if (value_inst == std::nullopt) {
            return false;
        }

        return FillPrimitiveLiteral(literal, component_type.GetId(), *value_inst);
    }

    auto string_type =
        pandasm::Type::FromDescriptor(panda::panda_file::GetStringClassDescriptor(ir_interface_->GetSourceLang()));
    if ((raw_elem_inst->GetOpcode() == Opcode::LoadString) && (component_type_name == string_type.GetName())) {
        literal->tag_ = panda_file::LiteralTag::ARRAY_STRING;
        std::string string_value = ir_interface_->GetStringIdByOffset(raw_elem_inst->CastToLoadString()->GetTypeId());
        literal->value_ = string_value;
        return true;
    }

    return false;
}

}  // namespace panda::bytecodeopt
