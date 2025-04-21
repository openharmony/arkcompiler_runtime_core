/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "optimizer/code_generator/encode.h"
#include "optimizer/ir_builder/inst_builder.h"
#include "bytecode_instruction-inl.h"

namespace ark::compiler {
/*
    FMOV: f -> n
    return CMP(AND(SHR(n, FP_FRACT_SIZE), FP_EXP_MASK), FP_EXP_MASK)

    1. bitcast f1
    2. shr v1, FP_FRACT_SIZE
    3. and v2, FP_EXP_MASK
    4. compare v3, FP_EXP_MASK

    fraction size is 23 bits for floats and 52 bits for doubles
    exponent mask is 0xff (8 bits) for floats and 0x7ff (11 bits) for doubles
 */
void InstBuilder::BuildIsFiniteIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto type = GetMethodArgumentType(methodId, 0);
    auto itype = type == DataType::FLOAT32 ? DataType::INT32 : DataType::INT64;
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto fpFractSize = type == DataType::FLOAT32 ? 23 : 52;
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto fpExpMask = type == DataType::FLOAT32 ? 0xff : 0x7ff;

    auto bitcast =
        GetGraph()->CreateInstBitcast(itype, GetPc(bcInst->GetAddress()), GetArgDefinition(bcInst, 0, accRead), type);
    auto shift =
        GetGraph()->CreateInstShr(itype, GetPc(bcInst->GetAddress()), bitcast, FindOrCreateConstant(fpFractSize));
    auto mask = GetGraph()->CreateInstAnd(itype, GetPc(bcInst->GetAddress()), shift, FindOrCreateConstant(fpExpMask));
    auto cmp = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), mask,
                                             FindOrCreateConstant(fpExpMask), itype, ConditionCode::CC_NE);

    AddInstruction(bitcast, shift, mask, cmp);
    UpdateDefinitionAcc(cmp);
}

void InstBuilder::BuildStdRuntimeEquals(const BytecodeInstruction *bcInst, bool accRead)
{
    auto cmp =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), GetArgDefinition(bcInst, 1, accRead),
                                      GetArgDefinition(bcInst, 2, accRead), DataType::REFERENCE, ConditionCode::CC_EQ);
    AddInstruction(cmp);
    UpdateDefinitionAcc(cmp);
}

void InstBuilder::BuildSignbitIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bitcast = GetGraph()->CreateInstBitcast(DataType::INT64, GetPc(bcInst->GetAddress()),
                                                 GetArgDefinition(bcInst, 0, accRead), DataType::FLOAT64);
    constexpr auto SHIFT = 63;
    auto res =
        GetGraph()->CreateInstShr(DataType::INT64, GetPc(bcInst->GetAddress()), bitcast, FindOrCreateConstant(SHIFT));
    AddInstruction(bitcast, res);
    UpdateDefinitionAcc(res);
}

/*
    1. SaveState
    2. NullCheck obj
    3. LoadNative actualLength ACTUAL_LENGTH_OFFSET
    4. BoundsCheck actualLength index (forced de-opt if check fails)
    5. LoadNative buffer BUFFER_OFFSET
    6. StoreArray buffer, index, value
 */
void InstBuilder::BuildEscompatArraySetIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    size_t bcAddr = GetPc(bcInst->GetAddress());
    Inst *obj = GetArgDefinition(bcInst, 0, accRead);
    Inst *index = GetArgDefinition(bcInst, 1, accRead);
    Inst *value = GetArgDefinition(bcInst, 2, accRead);

    SaveStateInst *saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    NullCheckInst *nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, obj, saveState);
    AddInstruction(saveState, nullCheck);

    ptrdiff_t actualLengthOffset = cross_values::GetEscompatArrayActualLengthOffset(graph_->GetArch());
    LoadMemInst *actualLength = graph_->CreateInstLoadNative(DataType::INT32, bcAddr, nullCheck,
                                                             graph_->FindOrCreateConstant(actualLengthOffset));
    AddInstruction(actualLength);
    BoundsCheckInst *boundsCheck =
        graph_->CreateInstBoundsCheck(DataType::INT32, bcAddr, actualLength, index, saveState);
    boundsCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
    AddInstruction(boundsCheck);

    ptrdiff_t bufferOffset = cross_values::GetEscompatArrayBufferOffset(graph_->GetArch());
    LoadMemInst *buffer = graph_->CreateInstLoadNative(DataType::REFERENCE, bcAddr, nullCheck,
                                                       graph_->FindOrCreateConstant(bufferOffset));
    AddInstruction(buffer);
    // true : needBarrier
    StoreInst *storeValue = graph_->CreateInstStoreArray(DataType::REFERENCE, bcAddr, buffer, boundsCheck, value, true);
    AddInstruction(storeValue);
}

/*
    1. SaveState
    2. NullCheck obj
    3. LoadNative actualLength ACTUAL_LENGTH_OFFSET
    4. BoundsCheck actualLength index (forced de-opt if check fails)
    5. LoadNative buffer BUFFER_OFFSET
    6. LoadArray value, buffer, index
    Returns value
 */
void InstBuilder::BuildEscompatArrayGetIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    size_t bcAddr = GetPc(bcInst->GetAddress());
    Inst *obj = GetArgDefinition(bcInst, 0, accRead);
    Inst *index = GetArgDefinition(bcInst, 1, accRead);

    SaveStateInst *saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    NullCheckInst *nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, obj, saveState);
    AddInstruction(saveState, nullCheck);

    ptrdiff_t actualLengthOffset = cross_values::GetEscompatArrayActualLengthOffset(graph_->GetArch());
    LoadMemInst *actualLength = graph_->CreateInstLoadNative(DataType::INT32, bcAddr, nullCheck,
                                                             graph_->FindOrCreateConstant(actualLengthOffset));
    AddInstruction(actualLength);
    BoundsCheckInst *boundsCheck =
        graph_->CreateInstBoundsCheck(DataType::INT32, bcAddr, actualLength, index, saveState);
    boundsCheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
    AddInstruction(boundsCheck);

    ptrdiff_t bufferOffset = cross_values::GetEscompatArrayBufferOffset(graph_->GetArch());
    LoadMemInst *buffer = graph_->CreateInstLoadNative(DataType::REFERENCE, bcAddr, nullCheck,
                                                       graph_->FindOrCreateConstant(bufferOffset));
    AddInstruction(buffer);

    LoadInst *result = graph_->CreateInstLoadArray(DataType::REFERENCE, bcAddr, buffer, boundsCheck);
    AddInstruction(result);
    UpdateDefinitionAcc(result);
}

void InstBuilder::BuildTypedArraySetIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type, bool accRead)
{
    auto *value = GetArgDefinition(bcInst, 2, accRead);
    BuildTypedArraySetIntrinsic(bcInst, value, type, accRead);
}

void InstBuilder::BuildUint8ClampedArraySetIntrinsic(const BytecodeInstruction *bcInst,
                                                     ark::compiler::DataType::Type type, bool accRead)
{
    constexpr auto MAX_VALUE = 255;
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *value = GetArgDefinition(bcInst, 2, accRead);
    auto *graph = GetGraph();
    auto *clamped0 = graph->CreateInstMax(DataType::INT32, bcAddr);
    clamped0->SetInput(0, value);
    clamped0->SetInput(1, graph->FindOrCreateConstant(0));
    AddInstruction(clamped0);
    auto *clamped = graph->CreateInstMin(DataType::INT32, bcAddr);
    clamped->SetInput(0, clamped0);
    clamped->SetInput(1, graph->FindOrCreateConstant(MAX_VALUE));
    AddInstruction(clamped);
    BuildTypedArraySetIntrinsic(bcInst, clamped, type, accRead);
}

void InstBuilder::BuildTypedArraySetIntrinsic(const BytecodeInstruction *bcInst, Inst *value, DataType::Type type,
                                              bool accRead)
{
    const size_t valueIndex = 2;
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto [loadDataInst, dataOffsetInst] = DataType::IsTypeSigned(type)
                                              ? BuildTypedArrayLoadDataAndOffset(bcInst, type, accRead)
                                              : BuildTypedUnsignedArrayLoadDataAndOffset(bcInst, type, accRead);

    auto *storeInst = GetGraph()->CreateInstStoreNative(type, bcAddr);
    storeInst->SetInput(0, loadDataInst);
    storeInst->SetInput(1, dataOffsetInst);
    if (!DataType::IsInt32Bit(type) && DataType::IsInt32Bit(value->GetType())) {
        auto *cast = GetGraph()->CreateInstCast(type, bcAddr, value, value->GetType());
        storeInst->SetInput(valueIndex, cast);
        AddInstruction(cast);
    } else {
        storeInst->SetInput(valueIndex, value);
    }
    AddInstruction(storeInst);
}

void InstBuilder::BuildTypedArrayGetIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto [loadDataInst, dataOffsetInst] = DataType::IsTypeSigned(type)
                                              ? BuildTypedArrayLoadDataAndOffset(bcInst, type, accRead)
                                              : BuildTypedUnsignedArrayLoadDataAndOffset(bcInst, type, accRead);

    auto *loadInst = GetGraph()->CreateInstLoadNative(type, bcAddr);
    loadInst->SetInput(0, loadDataInst);
    loadInst->SetInput(1, dataOffsetInst);
    AddInstruction(loadInst);

    if (type == DataType::INT64 || type == DataType::UINT64) {
        UpdateDefinitionAcc(loadInst);
        return;
    }

    auto result = GetGraph()->CreateInstCast(DataType::FLOAT64, bcAddr, loadInst, loadInst->GetType());
    AddInstruction(result);
    UpdateDefinitionAcc(result);
}

/*
    1. typedArray
    2. pos
    3. NullCheck v1
    4. DeoptimizeIf v3.arrayBufferBacked != 1
    5. LoadNative v3, TYPED_ARRAY_BUFFER_OFFSET
    6. LoadNative v5, ARRAY_BUFFER_DATA_OFFSET
    7. DeoptimizeIf v6 == 0
    9. LoadNative v3, TYPED_ARRAY_BUFFER_BYTE_OFFSET_OFFSET
    10. Cast FLOAT64->INT32 v9
    11. LoadNative v3, TYPED_ARRAY_LENGTH_OFFSET
    12. DeoptimizeIf v2 CC_AE v11
    13. Add v2, ARRAY_DATA_OFFSET
    14. Add v13, v10
    Returns (v6, v14)
 */
std::tuple<Inst *, Inst *> InstBuilder::BuildTypedArrayLoadDataAndOffset(const BytecodeInstruction *bcInst,
                                                                         DataType::Type type, bool accRead)
{
    ASSERT(DataType::IsTypeSigned(type));
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *obj = GetArgDefinition(bcInst, 0, accRead);
    auto *pos = GetArgDefinition(bcInst, 1, accRead);
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    AddInstruction(saveState);
    auto *graph = GetGraph();
    auto arch = graph->GetArch();
    auto *nullCheck = graph->CreateInstNullCheck(DataType::REFERENCE, bcAddr, obj, saveState);
    AddInstruction(nullCheck);

    BuildTypedArrayDeoptimizeIfNotArrayBufferBacked(
        nullCheck, bcAddr, saveState, ark::cross_values::GetTypedArrayArrayBufferBackedOffset(graph->GetArch()));

    auto *loadBufferInst =
        graph->CreateInstLoadNative(DataType::REFERENCE, bcAddr, nullCheck,
                                    graph->FindOrCreateConstant(ark::cross_values::GetTypedArrayBufferOffset(arch)));
    AddInstruction(loadBufferInst);

    auto *loadDataInst =
        graph->CreateInstLoadNative(DataType::REFERENCE, bcAddr, loadBufferInst,
                                    graph->FindOrCreateConstant(ark::cross_values::GetArrayBufferDataOffset(arch)));
    AddInstruction(loadDataInst);

    BuildTypedArrayDeoptimizeIfExternalData(loadDataInst, bcAddr, saveState);

    auto *loadDataOffsetFloat64Inst = graph->CreateInstLoadNative(
        DataType::FLOAT64, bcAddr, nullCheck,
        graph->FindOrCreateConstant(ark::cross_values::GetTypedArrayByteOffsetOffset(arch)));
    AddInstruction(loadDataOffsetFloat64Inst);
    auto *loadDataOffsetInst =
        graph->CreateInstCast(DataType::INT32, bcAddr, loadDataOffsetFloat64Inst, loadDataOffsetFloat64Inst->GetType());
    AddInstruction(loadDataOffsetInst);

    auto *loadLengthInst =
        graph->CreateInstLoadNative(DataType::INT32, bcAddr, nullCheck,
                                    graph->FindOrCreateConstant(ark::cross_values::GetTypedArrayLengthOffset(arch)));
    AddInstruction(loadLengthInst);

    BuildTypedArrayDeoptimizeIfOutOfRange(pos, loadLengthInst, bcAddr, saveState);

    auto *arrayDataOffset = graph->FindOrCreateConstant(ark::cross_values::GetCoretypesArrayDataOffset(arch));
    auto scale = DataType::ShiftByType(type, graph->GetArch());
    auto *scaledPos = graph->CreateInstShl(DataType::INT32, bcAddr, pos, graph->FindOrCreateConstant(scale));
    AddInstruction(scaledPos);
    auto *dataOffsetInst = graph->CreateInstAdd(DataType::INT32, bcAddr, scaledPos, arrayDataOffset);
    AddInstruction(dataOffsetInst);
    dataOffsetInst = graph->CreateInstAdd(DataType::INT32, bcAddr, dataOffsetInst, loadDataOffsetInst);
    AddInstruction(dataOffsetInst);

    return std::make_tuple(loadDataInst, dataOffsetInst);
}

void InstBuilder::BuildTypedArrayDeoptimizeIfNotArrayBufferBacked(Inst *typedArrayInst, size_t bcAddr,
                                                                  SaveStateInst *saveState, size_t fieldOffset)
{
    auto *graph = GetGraph();
    auto *loadArrayBufferBackedInst =
        graph->CreateInstLoadNative(DataType::INT8, bcAddr, typedArrayInst, graph->FindOrCreateConstant(fieldOffset));
    AddInstruction(loadArrayBufferBackedInst);
    auto *zeroInst = GetGraph()->FindOrCreateConstant(0);
    auto *isNotArrayBufferBackedInst = graph->CreateInstCompare(DataType::BOOL, bcAddr, loadArrayBufferBackedInst,
                                                                zeroInst, DataType::INT8, ConditionCode::CC_EQ);
    AddInstruction(isNotArrayBufferBackedInst);
    auto *deoptIsNotArrayBufferBackedInst =
        graph->CreateInstDeoptimizeIf(bcAddr, isNotArrayBufferBackedInst, saveState, DeoptimizeType::ZERO_CHECK);
    AddInstruction(deoptIsNotArrayBufferBackedInst);
}

void InstBuilder::BuildTypedArrayDeoptimizeIfExternalData(Inst *dataInst, size_t bcAddr, SaveStateInst *saveState)
{
    auto *graph = GetGraph();
    auto *nullPtrInst = graph->GetOrCreateNullPtr();
    auto *isExternalInst = graph->CreateInstCompare(DataType::BOOL, bcAddr, dataInst, nullPtrInst, DataType::REFERENCE,
                                                    ConditionCode::CC_EQ);
    AddInstruction(isExternalInst);
    auto *deoptIsExternalInst =
        graph->CreateInstDeoptimizeIf(bcAddr, isExternalInst, saveState, DeoptimizeType::ZERO_CHECK);
    AddInstruction(deoptIsExternalInst);
}

void InstBuilder::BuildTypedArrayDeoptimizeIfOutOfRange(Inst *posInst, Inst *lengthInst, size_t bcAddr,
                                                        SaveStateInst *saveState)
{
    auto *graph = GetGraph();
    auto *boundsCheck =
        graph->CreateInstCompare(DataType::BOOL, bcAddr, posInst, lengthInst, DataType::INT32, ConditionCode::CC_AE);
    AddInstruction(boundsCheck);

    auto *deoptRangeError = graph->CreateInstDeoptimizeIf(bcAddr, boundsCheck, saveState, DeoptimizeType::BOUNDS_CHECK);
    AddInstruction(deoptRangeError);
}

/*
    1. typedUArray
    2. pos
    3. NullCheck v1
    4. DeoptimizeIf v3.arrayBufferBacked != 1
    5. LoadNative v3, TYPED_U_ARRAY_BUFFER_OFFSET
    6. LoadNative v5, ARRAY_BUFFER_DATA_OFFSET
    7. DeoptimizeIf v6 == 0
    9. LoadNative v3, TYPED_U_ARRAY_BUFFER_BYTE_OFFSET_OFFSET
    11. LoadNative v3, TYPED_U_ARRAY_LENGTH_OFFSET
    12. DeoptimizeIf v2 CC_AE v11
    13. Add v2, ARRAY_DATA_OFFSET
    14. Add v13, v9
    Returns (v6, v14)
 */
std::tuple<Inst *, Inst *> InstBuilder::BuildTypedUnsignedArrayLoadDataAndOffset(const BytecodeInstruction *bcInst,
                                                                                 DataType::Type type, bool accRead)
{
    ASSERT(!DataType::IsTypeSigned(type));
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto *obj = GetArgDefinition(bcInst, 0, accRead);
    auto *pos = GetArgDefinition(bcInst, 1, accRead);
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    AddInstruction(saveState);
    auto *graph = GetGraph();
    auto arch = graph->GetArch();
    auto *nullCheck = graph->CreateInstNullCheck(DataType::REFERENCE, bcAddr, obj, saveState);
    AddInstruction(nullCheck);

    BuildTypedArrayDeoptimizeIfNotArrayBufferBacked(
        nullCheck, bcAddr, saveState,
        ark::cross_values::GetTypedUnsignedArrayArrayBufferBackedOffset(graph->GetArch()));

    auto *loadBufferInst = graph->CreateInstLoadNative(
        DataType::REFERENCE, bcAddr, nullCheck,
        graph->FindOrCreateConstant(ark::cross_values::GetTypedUnsignedArrayBufferOffset(arch)));
    AddInstruction(loadBufferInst);

    auto *loadDataInst =
        graph->CreateInstLoadNative(DataType::REFERENCE, bcAddr, loadBufferInst,
                                    graph->FindOrCreateConstant(ark::cross_values::GetArrayBufferDataOffset(arch)));
    AddInstruction(loadDataInst);

    BuildTypedArrayDeoptimizeIfExternalData(loadDataInst, bcAddr, saveState);

    auto *loadDataOffsetInst = graph->CreateInstLoadNative(
        DataType::INT32, bcAddr, nullCheck,
        graph->FindOrCreateConstant(ark::cross_values::GetTypedUnsignedArrayByteOffsetOffset(arch)));
    AddInstruction(loadDataOffsetInst);

    auto *loadLengthInst = graph->CreateInstLoadNative(
        DataType::INT32, bcAddr, nullCheck,
        graph->FindOrCreateConstant(ark::cross_values::GetTypedUnsignedArrayLengthOffset(arch)));
    AddInstruction(loadLengthInst);

    BuildTypedArrayDeoptimizeIfOutOfRange(pos, loadLengthInst, bcAddr, saveState);

    auto *arrayDataOffset = graph->FindOrCreateConstant(ark::cross_values::GetCoretypesArrayDataOffset(arch));
    auto scale = DataType::ShiftByType(type, graph->GetArch());
    auto *scaledPos = graph->CreateInstShl(DataType::INT32, bcAddr, pos, graph->FindOrCreateConstant(scale));
    AddInstruction(scaledPos);
    auto *dataOffsetInst = graph->CreateInstAdd(DataType::INT32, bcAddr, scaledPos, arrayDataOffset);
    AddInstruction(dataOffsetInst);
    dataOffsetInst = graph->CreateInstAdd(DataType::INT32, bcAddr, dataOffsetInst, loadDataOffsetInst);
    AddInstruction(dataOffsetInst);

    return std::make_tuple(loadDataInst, dataOffsetInst);
}

}  // namespace ark::compiler
