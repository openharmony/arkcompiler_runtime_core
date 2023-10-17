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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/optimizations/const_folding.h"
#include "utils/math_helpers.h"

#include <cmath>
#include <map>

namespace panda::compiler {
template <class T>
uint64_t ConvertIntToInt(T value, DataType::Type target_type)
{
    switch (target_type) {
        case DataType::BOOL:
            return static_cast<uint64_t>(static_cast<bool>(value));
        case DataType::UINT8:
            return static_cast<uint64_t>(static_cast<uint8_t>(value));
        case DataType::INT8:
            return static_cast<uint64_t>(static_cast<int8_t>(value));
        case DataType::UINT16:
            return static_cast<uint64_t>(static_cast<uint16_t>(value));
        case DataType::INT16:
            return static_cast<uint64_t>(static_cast<int16_t>(value));
        case DataType::UINT32:
            return static_cast<uint64_t>(static_cast<uint32_t>(value));
        case DataType::INT32:
            return static_cast<uint64_t>(static_cast<int32_t>(value));
        case DataType::UINT64:
            return static_cast<uint64_t>(value);
        case DataType::INT64:
            return static_cast<uint64_t>(static_cast<int64_t>(value));
        default:
            UNREACHABLE();
    }
}

template <class T>
T ConvertIntToFloat(uint64_t value, DataType::Type source_type)
{
    switch (source_type) {
        case DataType::BOOL:
            return static_cast<T>(static_cast<bool>(value));
        case DataType::UINT8:
            return static_cast<T>(static_cast<uint8_t>(value));
        case DataType::INT8:
            return static_cast<T>(static_cast<int8_t>(value));
        case DataType::UINT16:
            return static_cast<T>(static_cast<uint16_t>(value));
        case DataType::INT16:
            return static_cast<T>(static_cast<int16_t>(value));
        case DataType::UINT32:
            return static_cast<T>(static_cast<uint32_t>(value));
        case DataType::INT32:
            return static_cast<T>(static_cast<int32_t>(value));
        case DataType::UINT64:
            return static_cast<T>(value);
        case DataType::INT64:
            return static_cast<T>(static_cast<int64_t>(value));
        default:
            UNREACHABLE();
    }
}

template <class To, class From>
To ConvertFloatToInt(From value)
{
    To res;

    constexpr To MIN_INT = std::numeric_limits<To>::min();
    constexpr To MAX_INT = std::numeric_limits<To>::max();
    const auto float_min_int = static_cast<From>(MIN_INT);
    const auto float_max_int = static_cast<From>(MAX_INT);

    if (value > float_min_int) {
        if (value < float_max_int) {
            res = static_cast<To>(value);
        } else {
            res = MAX_INT;
        }
    } else if (std::isnan(value)) {
        res = 0;
    } else {
        res = MIN_INT;
    }

    return static_cast<To>(res);
}

template <class From>
uint64_t ConvertFloatToInt(From value, DataType::Type target_type)
{
    ASSERT(DataType::GetCommonType(target_type) == DataType::INT64);
    switch (target_type) {
        case DataType::BOOL:
            return static_cast<uint64_t>(ConvertFloatToInt<bool>(value));
        case DataType::UINT8:
            return static_cast<uint64_t>(ConvertFloatToInt<uint8_t>(value));
        case DataType::INT8:
            return static_cast<uint64_t>(ConvertFloatToInt<int8_t>(value));
        case DataType::UINT16:
            return static_cast<uint64_t>(ConvertFloatToInt<uint16_t>(value));
        case DataType::INT16:
            return static_cast<uint64_t>(ConvertFloatToInt<int16_t>(value));
        case DataType::UINT32:
            return static_cast<uint64_t>(ConvertFloatToInt<uint32_t>(value));
        case DataType::INT32:
            return static_cast<uint64_t>(ConvertFloatToInt<int32_t>(value));
        case DataType::UINT64:
            return ConvertFloatToInt<uint64_t>(value);
        case DataType::INT64:
            return static_cast<uint64_t>(ConvertFloatToInt<int64_t>(value));
        default:
            UNREACHABLE();
    }
}

template <class From>
uint64_t ConvertFloatToIntDyn(From value, RuntimeInterface *runtime, size_t bits)
{
    return runtime->DynamicCastDoubleToInt(static_cast<double>(value), bits);
}

ConstantInst *ConstFoldingCreateIntConst(Inst *inst, uint64_t value, bool is_literal_data)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType()) && !is_literal_data) {
        return graph->FindOrCreateConstant<uint32_t>(value);
    }
    return graph->FindOrCreateConstant(value);
}

template <typename T>
ConstantInst *ConstFoldingCreateConst(Inst *inst, ConstantInst *cnst, bool is_literal_data = false)
{
    return ConstFoldingCreateIntConst(inst, ConvertIntToInt(static_cast<T>(cnst->GetIntValue()), inst->GetType()),
                                      is_literal_data);
}

ConstantInst *ConstFoldingCastInt2Int(Inst *inst, ConstantInst *cnst)
{
    switch (inst->GetInputType(0)) {
        case DataType::BOOL:
            return ConstFoldingCreateConst<bool>(inst, cnst);
        case DataType::UINT8:
            return ConstFoldingCreateConst<uint8_t>(inst, cnst);
        case DataType::INT8:
            return ConstFoldingCreateConst<int8_t>(inst, cnst);
        case DataType::UINT16:
            return ConstFoldingCreateConst<uint16_t>(inst, cnst);
        case DataType::INT16:
            return ConstFoldingCreateConst<int16_t>(inst, cnst);
        case DataType::UINT32:
            return ConstFoldingCreateConst<uint32_t>(inst, cnst);
        case DataType::INT32:
            return ConstFoldingCreateConst<int32_t>(inst, cnst);
        case DataType::UINT64:
            return ConstFoldingCreateConst<uint64_t>(inst, cnst);
        case DataType::INT64:
            return ConstFoldingCreateConst<int64_t>(inst, cnst);
        default:
            return nullptr;
    }
}

ConstantInst *ConstFoldingCastIntConst(Graph *graph, Inst *inst, ConstantInst *cnst, bool is_literal_data = false)
{
    auto inst_type = DataType::GetCommonType(inst->GetType());
    if (inst_type == DataType::INT64) {
        // INT -> INT
        return ConstFoldingCastInt2Int(inst, cnst);
    }
    if (inst_type == DataType::FLOAT32) {
        // INT -> FLOAT
        if (graph->IsBytecodeOptimizer() && !is_literal_data) {
            return nullptr;
        }
        return graph->FindOrCreateConstant(ConvertIntToFloat<float>(cnst->GetIntValue(), inst->GetInputType(0)));
    }
    if (inst_type == DataType::FLOAT64) {
        // INT -> DOUBLE
        return graph->FindOrCreateConstant(ConvertIntToFloat<double>(cnst->GetIntValue(), inst->GetInputType(0)));
    }
    return nullptr;
}

ConstantInst *ConstFoldingCastConst(Inst *inst, Inst *input, bool is_literal_data)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto cnst = static_cast<ConstantInst *>(input);
    auto inst_type = DataType::GetCommonType(inst->GetType());
    if (cnst->GetType() == DataType::INT32 || cnst->GetType() == DataType::INT64) {
        return ConstFoldingCastIntConst(graph, inst, cnst);
    }
    if (cnst->GetType() == DataType::FLOAT32) {
        if (graph->IsBytecodeOptimizer() && !is_literal_data) {
            return nullptr;
        }
        if (inst_type == DataType::INT64) {
            // FLOAT->INT
            return graph->FindOrCreateConstant(ConvertFloatToInt(cnst->GetFloatValue(), inst->GetType()));
        }
        if (inst_type == DataType::FLOAT32) {
            // FLOAT -> FLOAT
            return cnst;
        }
        if (inst_type == DataType::FLOAT64) {
            // FLOAT -> DOUBLE
            return graph->FindOrCreateConstant(static_cast<double>(cnst->GetFloatValue()));
        }
    } else if (cnst->GetType() == DataType::FLOAT64) {
        if (inst_type == DataType::INT64) {
            // DOUBLE->INT/LONG
            uint64_t val = graph->IsDynamicMethod()
                               ? ConvertFloatToIntDyn(cnst->GetDoubleValue(), graph->GetRuntime(),
                                                      DataType::GetTypeSize(inst->GetType(), graph->GetArch()))
                               : ConvertFloatToInt(cnst->GetDoubleValue(), inst->GetType());
            return ConstFoldingCreateIntConst(inst, val, is_literal_data);
        }
        if (inst_type == DataType::FLOAT32) {
            // DOUBLE -> FLOAT
            if (graph->IsBytecodeOptimizer() && !is_literal_data) {
                return nullptr;
            }
            return graph->FindOrCreateConstant(static_cast<float>(cnst->GetDoubleValue()));
        }
        if (inst_type == DataType::FLOAT64) {
            // DOUBLE -> DOUBLE
            return cnst;
        }
    }
    return nullptr;
}

bool ConstFoldingCast(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Cast);
    auto input = inst->GetInput(0).GetInst();
    if (input->IsConst()) {
        ConstantInst *nw_cnst = ConstFoldingCastConst(inst, input);
        if (nw_cnst != nullptr) {
            inst->ReplaceUsers(nw_cnst);
            return true;
        }
    }
    return false;
}

bool ConstFoldingNeg(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Neg);
    auto input = inst->GetInput(0);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input.GetInst()->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input.GetInst());
        ConstantInst *new_cnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                new_cnst = ConstFoldingCreateIntConst(inst, ConvertIntToInt(-cnst->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(-cnst->GetFloatValue());
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(-cnst->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingAbs(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Abs);
    auto input = inst->GetInput(0);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input.GetInst()->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input.GetInst());
        ConstantInst *new_cnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64: {
                ASSERT(DataType::IsTypeSigned(inst->GetType()));
                int64_t value = cnst->GetIntValue();
                if (value == INT64_MIN) {
                    new_cnst = cnst;
                    break;
                }
                uint64_t uvalue = (value < 0) ? -value : value;
                new_cnst = ConstFoldingCreateIntConst(inst, ConvertIntToInt(uvalue, inst->GetType()));
                break;
            }
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(std::abs(cnst->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(std::abs(cnst->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingNot(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Not);
    auto input = inst->GetInput(0);
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input.GetInst()->IsConst()) {
        auto cnst = static_cast<ConstantInst *>(input.GetInst());
        auto new_cnst = ConstFoldingCreateIntConst(inst, ConvertIntToInt(~cnst->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingAdd(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Add);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        ConstantInst *new_cnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                new_cnst = ConstFoldingCreateIntConst(
                    inst, ConvertIntToInt(cnst0->GetIntValue() + cnst1->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() + cnst1->GetFloatValue());
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() + cnst1->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingSub(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Sub);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        ConstantInst *new_cnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                new_cnst = ConstFoldingCreateIntConst(
                    inst, ConvertIntToInt(cnst0->GetIntValue() - cnst1->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() - cnst1->GetFloatValue());
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() - cnst1->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    if (input0.GetInst() == input1.GetInst() && DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
        // for floating point values 'x-x -> 0' optimization is not applicable because of NaN/Infinity values
        auto new_cnst = ConstFoldingCreateIntConst(inst, 0);
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingMul(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Mul);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ConstantInst *new_cnst = nullptr;
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                new_cnst = ConstFoldingCreateIntConst(
                    inst, ConvertIntToInt(cnst0->GetIntValue() * cnst1->GetIntValue(), inst->GetType()));
                break;
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() * cnst1->GetFloatValue());
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() * cnst1->GetDoubleValue());
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    if (input0.GetInst()->IsConst()) {
        new_cnst = static_cast<ConstantInst *>(input0.GetInst());
    } else if (input1.GetInst()->IsConst()) {
        new_cnst = static_cast<ConstantInst *>(input1.GetInst());
    }
    if (new_cnst != nullptr && new_cnst->IsEqualConst(0, graph->IsBytecodeOptimizer())) {
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

ConstantInst *ConstFoldingDivInt2Int(Inst *inst, Graph *graph, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (cnst1->GetIntValue() == 0) {
        return nullptr;
    }
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            if (static_cast<int32_t>(cnst0->GetIntValue()) == INT32_MIN &&
                static_cast<int32_t>(cnst1->GetIntValue()) == -1) {
                return graph->FindOrCreateConstant<uint32_t>(INT32_MIN);
            }
            return graph->FindOrCreateConstant<uint32_t>(
                ConvertIntToInt(static_cast<int32_t>(cnst0->GetIntValue()) / static_cast<int32_t>(cnst1->GetIntValue()),
                                inst->GetType()));
        }
        if (static_cast<int64_t>(cnst0->GetIntValue()) == INT64_MIN &&
            static_cast<int64_t>(cnst1->GetIntValue()) == -1) {
            return graph->FindOrCreateConstant<uint64_t>(INT64_MIN);
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            static_cast<int64_t>(cnst0->GetIntValue()) / static_cast<int64_t>(cnst1->GetIntValue()), inst->GetType()));
    }

    return ConstFoldingCreateIntConst(inst, ConvertIntToInt(cnst0->GetIntValue(), inst->GetType()) /
                                                ConvertIntToInt(cnst1->GetIntValue(), inst->GetType()));
}

bool ConstFoldingDiv(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Div);
    auto input0 = inst->GetDataFlowInput(0);
    auto input1 = inst->GetDataFlowInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (!input0->IsConst() || !input1->IsConst()) {
        return false;
    }
    auto cnst0 = input0->CastToConstant();
    auto cnst1 = input1->CastToConstant();
    ConstantInst *new_cnst = nullptr;
    switch (DataType::GetCommonType(inst->GetType())) {
        case DataType::INT64:
            new_cnst = ConstFoldingDivInt2Int(inst, graph, cnst0, cnst1);
            if (new_cnst == nullptr) {
                return false;
            }
            break;
        case DataType::FLOAT32:
            new_cnst = graph->FindOrCreateConstant(cnst0->GetFloatValue() / cnst1->GetFloatValue());
            break;
        case DataType::FLOAT64:
            new_cnst = graph->FindOrCreateConstant(cnst0->GetDoubleValue() / cnst1->GetDoubleValue());
            break;
        default:
            UNREACHABLE();
    }
    inst->ReplaceUsers(new_cnst);
    return true;
}

ConstantInst *ConstFoldingMinInt(Inst *inst, Graph *graph, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            return graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                std::min(static_cast<int32_t>(cnst0->GetIntValue()), static_cast<int32_t>(cnst1->GetIntValue())),
                inst->GetType()));
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            std::min(static_cast<int64_t>(cnst0->GetIntValue()), static_cast<int64_t>(cnst1->GetIntValue())),
            inst->GetType()));
    }
    return ConstFoldingCreateIntConst(
        inst, ConvertIntToInt(std::min(cnst0->GetIntValue(), cnst1->GetIntValue()), inst->GetType()));
}

bool ConstFoldingMin(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Min);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        ConstantInst *new_cnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                new_cnst = ConstFoldingMinInt(inst, graph, cnst0, cnst1);
                ASSERT(new_cnst != nullptr);
                break;
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(
                    panda::helpers::math::Min(cnst0->GetFloatValue(), cnst1->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(
                    panda::helpers::math::Min(cnst0->GetDoubleValue(), cnst1->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

ConstantInst *ConstFoldingMaxInt(Inst *inst, Graph *graph, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            return graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                std::max(static_cast<int32_t>(cnst0->GetIntValue()), static_cast<int32_t>(cnst1->GetIntValue())),
                inst->GetType()));
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            std::max(static_cast<int64_t>(cnst0->GetIntValue()), static_cast<int64_t>(cnst1->GetIntValue())),
            inst->GetType()));
    }
    return ConstFoldingCreateIntConst(
        inst, ConvertIntToInt(std::max(cnst0->GetIntValue(), cnst1->GetIntValue()), inst->GetType()));
}

bool ConstFoldingMax(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Max);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        ConstantInst *new_cnst = nullptr;
        switch (DataType::GetCommonType(inst->GetType())) {
            case DataType::INT64:
                new_cnst = ConstFoldingMaxInt(inst, graph, cnst0, cnst1);
                ASSERT(new_cnst != nullptr);
                break;
            case DataType::FLOAT32:
                new_cnst = graph->FindOrCreateConstant(
                    panda::helpers::math::Max(cnst0->GetFloatValue(), cnst1->GetFloatValue()));
                break;
            case DataType::FLOAT64:
                new_cnst = graph->FindOrCreateConstant(
                    panda::helpers::math::Max(cnst0->GetDoubleValue(), cnst1->GetDoubleValue()));
                break;
            default:
                UNREACHABLE();
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}
ConstantInst *ConstFoldingModIntConst(Graph *graph, Inst *inst, ConstantInst *cnst0, ConstantInst *cnst1)
{
    if (DataType::IsTypeSigned(inst->GetType())) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            if (static_cast<int32_t>(cnst0->GetIntValue()) == INT32_MIN &&
                static_cast<int32_t>(cnst1->GetIntValue()) == -1) {
                return graph->FindOrCreateConstant<uint32_t>(0);
            }
            return graph->FindOrCreateConstant<uint32_t>(
                ConvertIntToInt(static_cast<int32_t>(cnst0->GetIntValue()) % static_cast<int32_t>(cnst1->GetIntValue()),
                                inst->GetType()));
        }
        if (static_cast<int64_t>(cnst0->GetIntValue()) == INT64_MIN &&
            static_cast<int64_t>(cnst1->GetIntValue()) == -1) {
            return graph->FindOrCreateConstant<uint64_t>(0);
        }
        return graph->FindOrCreateConstant(ConvertIntToInt(
            static_cast<int64_t>(cnst0->GetIntValue()) % static_cast<int64_t>(cnst1->GetIntValue()), inst->GetType()));
    }
    return ConstFoldingCreateIntConst(inst, ConvertIntToInt(cnst0->GetIntValue(), inst->GetType()) %
                                                ConvertIntToInt(cnst1->GetIntValue(), inst->GetType()));
}

bool ConstFoldingMod(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Mod);
    auto input0 = inst->GetDataFlowInput(0);
    auto input1 = inst->GetDataFlowInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (input1->IsConst() && !DataType::IsFloatType(inst->GetType()) && input1->CastToConstant()->GetIntValue() == 1) {
        ConstantInst *cnst = ConstFoldingCreateIntConst(inst, 0);
        inst->ReplaceUsers(cnst);
        return true;
    }
    if (!input0->IsConst() || !input1->IsConst()) {
        return false;
    }
    ConstantInst *new_cnst = nullptr;
    auto cnst0 = input0->CastToConstant();
    auto cnst1 = input1->CastToConstant();
    if (DataType::GetCommonType(inst->GetType()) == DataType::INT64) {
        if (cnst1->GetIntValue() == 0) {
            return false;
        }
        new_cnst = ConstFoldingModIntConst(graph, inst, cnst0, cnst1);
    } else if (inst->GetType() == DataType::FLOAT32) {
        if (cnst1->GetFloatValue() == 0) {
            return false;
        }
        new_cnst =
            graph->FindOrCreateConstant(static_cast<float>(fmodf(cnst0->GetFloatValue(), cnst1->GetFloatValue())));
    } else if (inst->GetType() == DataType::FLOAT64) {
        if (cnst1->GetDoubleValue() == 0) {
            return false;
        }
        new_cnst = graph->FindOrCreateConstant(fmod(cnst0->GetDoubleValue(), cnst1->GetDoubleValue()));
    }
    inst->ReplaceUsers(new_cnst);
    return true;
}

bool ConstFoldingShl(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Shl);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = input0.GetInst()->CastToConstant()->GetIntValue();
        auto cnst1 = input1.GetInst()->CastToConstant()->GetIntValue();
        ConstantInst *new_cnst = nullptr;
        uint64_t size_mask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            new_cnst = graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                static_cast<uint32_t>(cnst0) << (static_cast<uint32_t>(cnst1) & static_cast<uint32_t>(size_mask)),
                inst->GetType()));
        } else {
            new_cnst = graph->FindOrCreateConstant(ConvertIntToInt(cnst0 << (cnst1 & size_mask), inst->GetType()));
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingShr(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Shr);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = input0.GetInst()->CastToConstant()->GetIntValue();
        auto cnst1 = input1.GetInst()->CastToConstant()->GetIntValue();
        uint64_t size_mask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        // zerod high part of the constant
        if (size_mask < DataType::GetTypeSize(DataType::INT32, graph->GetArch())) {
            // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
            uint64_t type_mask = (1ULL << (size_mask + 1)) - 1;
            cnst0 = cnst0 & type_mask;
        }
        ConstantInst *new_cnst = nullptr;
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            new_cnst = graph->FindOrCreateConstant<uint32_t>(ConvertIntToInt(
                static_cast<uint32_t>(cnst0) >> (static_cast<uint32_t>(cnst1) & static_cast<uint32_t>(size_mask)),
                inst->GetType()));
        } else {
            new_cnst = graph->FindOrCreateConstant(ConvertIntToInt(cnst0 >> (cnst1 & size_mask), inst->GetType()));
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingAShr(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::AShr);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    auto graph = inst->GetBasicBlock()->GetGraph();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        int64_t cnst0 = input0.GetInst()->CastToConstant()->GetIntValue();
        auto cnst1 = input1.GetInst()->CastToConstant()->GetIntValue();
        uint64_t size_mask = DataType::GetTypeSize(inst->GetType(), graph->GetArch()) - 1;
        ConstantInst *new_cnst = nullptr;
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
            new_cnst = graph->FindOrCreateConstant<uint32_t>(
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                ConvertIntToInt(static_cast<int32_t>(cnst0) >>
                                    (static_cast<uint32_t>(cnst1) & static_cast<uint32_t>(size_mask)),
                                inst->GetType()));
        } else {
            new_cnst = graph->FindOrCreateConstant(
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                ConvertIntToInt(cnst0 >> (cnst1 & size_mask), inst->GetType()));
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingAnd(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::And);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    ConstantInst *new_cnst = nullptr;
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        new_cnst = ConstFoldingCreateIntConst(
            inst, ConvertIntToInt(cnst0->GetIntValue() & cnst1->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    if (input0.GetInst()->IsConst()) {
        new_cnst = static_cast<ConstantInst *>(input0.GetInst());
    } else if (input1.GetInst()->IsConst()) {
        new_cnst = static_cast<ConstantInst *>(input1.GetInst());
    }
    if (new_cnst != nullptr && new_cnst->GetIntValue() == 0) {
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingOr(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Or);
    auto input0 = inst->GetInput(0);
    auto input1 = inst->GetInput(1);
    ConstantInst *new_cnst = nullptr;
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0.GetInst()->IsConst() && input1.GetInst()->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0.GetInst());
        auto cnst1 = static_cast<ConstantInst *>(input1.GetInst());
        new_cnst = ConstFoldingCreateIntConst(
            inst, ConvertIntToInt(cnst0->GetIntValue() | cnst1->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    if (input0.GetInst()->IsConst()) {
        new_cnst = static_cast<ConstantInst *>(input0.GetInst());
    } else if (input1.GetInst()->IsConst()) {
        new_cnst = static_cast<ConstantInst *>(input1.GetInst());
    }
    if (new_cnst != nullptr && new_cnst->GetIntValue() == static_cast<uint64_t>(-1)) {
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

bool ConstFoldingXor(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Xor);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    ASSERT(DataType::GetCommonType(inst->GetType()) == DataType::INT64);
    if (input0->IsConst() && input1->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0);
        auto cnst1 = static_cast<ConstantInst *>(input1);
        ConstantInst *new_cnst = nullptr;
        new_cnst = ConstFoldingCreateIntConst(
            inst, ConvertIntToInt(cnst0->GetIntValue() ^ cnst1->GetIntValue(), inst->GetType()));
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    // A xor A = 0
    if (input0 == input1) {
        inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
        return true;
    }
    return false;
}

template <class T>
int64_t GetResult(T l, T r, [[maybe_unused]] const CmpInst *cmp)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (std::is_same<T, float>() || std::is_same<T, double>()) {
        ASSERT(DataType::IsFloatType(cmp->GetInputType(0)));
        if (std::isnan(l) || std::isnan(r)) {
            if (cmp->IsFcmpg()) {
                return 1;
            }
            return -1;
        }
    }
    if (l > r) {
        return 1;
    }
    if (l < r) {
        return -1;
    }
    return 0;
}

int64_t GetIntResult(ConstantInst *cnst0, ConstantInst *cnst1, DataType::Type input_type, const CmpInst *cmp)
{
    auto l = ConvertIntToInt(cnst0->GetIntValue(), input_type);
    auto r = ConvertIntToInt(cnst1->GetIntValue(), input_type);
    auto graph = cnst0->GetBasicBlock()->GetGraph();
    if (DataType::IsTypeSigned(input_type)) {
        if (graph->IsBytecodeOptimizer() && IsInt32Bit(input_type)) {
            return GetResult(static_cast<int32_t>(l), static_cast<int32_t>(r), cmp);
        }
        return GetResult(static_cast<int64_t>(l), static_cast<int64_t>(r), cmp);
    }
    if (graph->IsBytecodeOptimizer() && IsInt32Bit(input_type)) {
        return GetResult(static_cast<uint32_t>(l), static_cast<uint32_t>(r), cmp);
    }
    return GetResult(l, r, cmp);
}

bool ConstFoldingCmp(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Cmp);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    auto cmp = inst->CastToCmp();
    auto input_type = cmp->GetInputType(0);
    if (input0->IsConst() && input1->IsConst()) {
        auto cnst0 = static_cast<ConstantInst *>(input0);
        auto cnst1 = static_cast<ConstantInst *>(input1);
        int64_t result = 0;
        switch (DataType::GetCommonType(input_type)) {
            case DataType::INT64: {
                result = GetIntResult(cnst0, cnst1, input_type, cmp);
                break;
            }
            case DataType::FLOAT32:
                result = GetResult(cnst0->GetFloatValue(), cnst1->GetFloatValue(), cmp);
                break;
            case DataType::FLOAT64:
                result = GetResult(cnst0->GetDoubleValue(), cnst1->GetDoubleValue(), cmp);
                break;
            default:
                break;
        }
        auto new_cnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(result);
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    if (input0 == input1 && DataType::GetCommonType(input_type) == DataType::INT64) {
        // for floating point values result may be non-zero if x is NaN
        inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
        return true;
    }
    return false;
}

ConstantInst *ConstFoldingCompareCreateConst(Inst *inst, bool value)
{
    auto graph = inst->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() && IsInt32Bit(inst->GetType())) {
        return graph->FindOrCreateConstant(static_cast<uint32_t>(value));
    }
    return graph->FindOrCreateConstant(static_cast<uint64_t>(value));
}

ConstantInst *ConstFoldingCompareCreateNewConst(Inst *inst, uint64_t cnst_val0, uint64_t cnst_val1)
{
    switch (inst->CastToCompare()->GetCc()) {
        case ConditionCode::CC_EQ:
            return ConstFoldingCompareCreateConst(inst, (cnst_val0 == cnst_val1));
        case ConditionCode::CC_NE:
            return ConstFoldingCompareCreateConst(inst, (cnst_val0 != cnst_val1));
        case ConditionCode::CC_LT:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnst_val0) < static_cast<int64_t>(cnst_val1)));
        case ConditionCode::CC_B:
            return ConstFoldingCompareCreateConst(inst, (cnst_val0 < cnst_val1));
        case ConditionCode::CC_LE:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnst_val0) <= static_cast<int64_t>(cnst_val1)));
        case ConditionCode::CC_BE:
            return ConstFoldingCompareCreateConst(inst, (cnst_val0 <= cnst_val1));
        case ConditionCode::CC_GT:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnst_val0) > static_cast<int64_t>(cnst_val1)));
        case ConditionCode::CC_A:
            return ConstFoldingCompareCreateConst(inst, (cnst_val0 > cnst_val1));
        case ConditionCode::CC_GE:
            return ConstFoldingCompareCreateConst(inst,
                                                  (static_cast<int64_t>(cnst_val0) >= static_cast<int64_t>(cnst_val1)));
        case ConditionCode::CC_AE:
            return ConstFoldingCompareCreateConst(inst, (cnst_val0 >= cnst_val1));
        case ConditionCode::CC_TST_EQ:
            return ConstFoldingCompareCreateConst(inst, ((cnst_val0 & cnst_val1) == 0));
        case ConditionCode::CC_TST_NE:
            return ConstFoldingCompareCreateConst(inst, ((cnst_val0 & cnst_val1) != 0));
        default:
            UNREACHABLE();
    }
}

bool ConstFoldingCompareEqualInputs(Inst *inst, Inst *input0, Inst *input1)
{
    if (input0 != input1) {
        return false;
    }
    auto cmp_inst = inst->CastToCompare();
    auto common_type = DataType::GetCommonType(input0->GetType());
    switch (cmp_inst->GetCc()) {
        case ConditionCode::CC_EQ:
        case ConditionCode::CC_LE:
        case ConditionCode::CC_GE:
        case ConditionCode::CC_BE:
        case ConditionCode::CC_AE:
            // for floating point values result may be non-zero if x is NaN
            if (common_type == DataType::INT64 || common_type == DataType::POINTER) {
                inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 1));
                return true;
            }
            break;
        case ConditionCode::CC_NE:
            if (common_type == DataType::INT64 || common_type == DataType::POINTER) {
                inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
                return true;
            }
            break;
        case ConditionCode::CC_LT:
        case ConditionCode::CC_GT:
        case ConditionCode::CC_B:
        case ConditionCode::CC_A:
            // x<x is false even for x=NaN
            inst->ReplaceUsers(ConstFoldingCreateIntConst(inst, 0));
            return true;
        default:
            return false;
    }
    return false;
}

bool ConstFoldingCompare(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Compare);
    auto input0 = inst->GetInput(0).GetInst();
    auto input1 = inst->GetInput(1).GetInst();
    if (input0->IsConst() && input1->IsConst() && !DataType::IsFloatType(input0->GetType())) {
        auto cnst0 = input0->CastToConstant();
        auto cnst1 = input1->CastToConstant();

        ConstantInst *new_cnst = nullptr;
        auto type = inst->GetInputType(0);
        if (DataType::GetCommonType(type) == DataType::INT64) {
            uint64_t cnst_val0 = ConvertIntToInt(cnst0->GetIntValue(), type);
            uint64_t cnst_val1 = ConvertIntToInt(cnst1->GetIntValue(), type);
            new_cnst = ConstFoldingCompareCreateNewConst(inst, cnst_val0, cnst_val1);
        } else {
            return false;
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    if (input0->GetOpcode() == Opcode::LoadImmediate && input1->GetOpcode() == Opcode::LoadImmediate) {
        auto class0 = input0->CastToLoadImmediate()->GetObject();
        auto class1 = input1->CastToLoadImmediate()->GetObject();
        auto cc = inst->CastToCompare()->GetCc();
        ASSERT(cc == CC_NE || cc == CC_EQ);
        bool res {(class0 == class1) == (cc == CC_EQ)};
        inst->ReplaceUsers(ConstFoldingCompareCreateConst(inst, res));
        return true;
    }
    if (IsZeroConstantOrNullPtr(input0) && IsZeroConstantOrNullPtr(input1)) {
        auto cc = inst->CastToCompare()->GetCc();
        ASSERT(cc == CC_NE || cc == CC_EQ);
        inst->ReplaceUsers(ConstFoldingCompareCreateConst(inst, cc == CC_EQ));
        return true;
    }
    if (ConstFoldingCompareEqualInputs(inst, input0, input1)) {
        return true;
    }
    if (inst->GetInputType(0) == DataType::REFERENCE) {
        ASSERT(input0 != input1);
        if ((input0->IsAllocation() || input0->GetOpcode() == Opcode::NullPtr) &&
            (input1->IsAllocation() || input1->GetOpcode() == Opcode::NullPtr)) {
            auto cc = inst->CastToCompare()->GetCc();
            inst->ReplaceUsers(ConstFoldingCompareCreateConst(inst, cc == CC_NE));
            return true;
        }
    }
    return false;
}

bool ConstFoldingSqrt(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Sqrt);
    auto input = inst->GetInput(0).GetInst();
    if (input->IsConst()) {
        auto cnst = input->CastToConstant();
        Inst *new_cnst = nullptr;
        if (cnst->GetType() == DataType::FLOAT32) {
            new_cnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(std::sqrt(cnst->GetFloatValue()));
        } else {
            ASSERT(cnst->GetType() == DataType::FLOAT64);
            new_cnst = inst->GetBasicBlock()->GetGraph()->FindOrCreateConstant(std::sqrt(cnst->GetDoubleValue()));
        }
        inst->ReplaceUsers(new_cnst);
        return true;
    }
    return false;
}

}  // namespace panda::compiler
