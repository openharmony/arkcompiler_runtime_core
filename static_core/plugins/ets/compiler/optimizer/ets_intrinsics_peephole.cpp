/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/ir/analysis.h"
#include "compiler/optimizer/ir/runtime_interface.h"
#include "compiler/optimizer/optimizations/const_folding.h"
#include "optimizer/ir/datatype.h"
#include "runtime/include/coretypes/string.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace ark::compiler {

static void ReplaceWithCompareEQ(IntrinsicInst *intrinsic)
{
    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();

    auto bb = intrinsic->GetBasicBlock();
    auto graph = bb->GetGraph();

    auto compare = graph->CreateInst(Opcode::Compare)->CastToCompare();
    compare->SetCc(ConditionCode::CC_EQ);
    compare->SetType(intrinsic->GetType());
    ASSERT(input0->GetType() == input1->GetType());
    compare->SetOperandsType(input0->GetType());

    compare->SetInput(0, input0);
    compare->SetInput(1, input1);
    bb->InsertAfter(compare, intrinsic);
    intrinsic->ReplaceUsers(compare);
}

static bool ReplaceTypeofWithIsInstance(IntrinsicInst *intrinsic)
{
    if (intrinsic->GetBasicBlock()->GetGraph()->IsAotMode()) {
        return false;
    }
    auto typeOf = intrinsic->GetDataFlowInput(0);
    auto loadString = intrinsic->GetInput(1).GetInst();
    if (loadString->GetOpcode() != Opcode::LoadString || !typeOf->IsIntrinsic()) {
        return false;
    }
    auto intrinsicId = typeOf->CastToIntrinsic()->GetIntrinsicId();
    if (intrinsicId != RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_TYPEOF) {
        return false;
    }
    auto typeId = loadString->CastToLoadString()->GetTypeId();
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto method = graph->GetMethod();

    //#15311: MethodId may from another abc file, so use GetSaveState to get the method
    if (loadString->GetSaveState()->GetMethod() != nullptr) {
        method = loadString->GetSaveState()->GetMethod();
    }

    auto stringValue = runtime->GetStringValue(method, typeId);
    RuntimeInterface::ClassPtr klass;
    uint32_t ktypeId = 0;
    if (stringValue == "string") {
        klass = runtime->GetStringClass(method, &ktypeId);
    } else {
        return false;
    }

    auto pc = intrinsic->GetPc();
    auto saveState = typeOf->GetInput(1U).GetInst();
    ASSERT(saveState->GetOpcode() == Opcode::SaveState);
    auto loadClass =
        graph->CreateInstLoadClass(DataType::REFERENCE, pc, saveState, TypeIdMixin {ktypeId, method}, nullptr);
    loadClass->SetClass(klass);
    auto *bb = saveState->GetBasicBlock();
    bb->InsertAfter(loadClass, saveState);
    auto isInstance = graph->CreateInstIsInstance(DataType::BOOL, pc, typeOf->GetInput(0).GetInst(), loadClass,
                                                  saveState, TypeIdMixin {typeId, method}, ClassType::OTHER_CLASS);
    intrinsic->ReplaceUsers(isInstance);
    bb->InsertAfter(isInstance, loadClass);
    return true;
}

bool Peepholes::PeepholeStringEquals([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    // Replaces
    //      Intrinsic.StdCoreStringEquals arg, NullPtr
    // with
    //      Compare EQ ref             arg, NullPtr

    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();
    if (input0->IsNullPtr() || input1->IsNullPtr()) {
        ReplaceWithCompareEQ(intrinsic);
        return true;
    }

    return ReplaceTypeofWithIsInstance(intrinsic);
}

static Inst *GetStringFromLength(Inst *inst)
{
    Inst *lenArray = inst;
    if (inst->GetBasicBlock()->GetGraph()->GetRuntime()->IsCompressedStringsEnabled()) {
        if (inst->GetOpcode() != Opcode::Shr || inst->GetType() != DataType::INT32) {
            return nullptr;
        }
        auto input1 = inst->GetInput(1).GetInst();
        if (!input1->IsConst() ||
            input1->CastToConstant()->GetRawValue() != ark::coretypes::String::STRING_LENGTH_SHIFT) {
            return nullptr;
        }
        lenArray = inst->GetInput(0).GetInst();
    }

    if (lenArray->GetOpcode() != Opcode::LenArray || !lenArray->CastToLenArray()->IsString()) {
        return nullptr;
    }
    return lenArray->GetDataFlowInput(0);
}

bool Peepholes::PeepholeStringSubstring([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    // Replaces
    //      string
    //      Intrinsic.StdCoreStringSubstring string, 0, string.Length -> .....
    // with
    //      string -> .....

    auto string = intrinsic->GetDataFlowInput(0);
    auto begin = intrinsic->GetInput(1).GetInst();
    auto end = intrinsic->GetInput(2).GetInst();
    if (!begin->IsConst() || begin->GetType() != DataType::INT64) {
        return false;
    }
    if (static_cast<int64_t>(begin->CastToConstant()->GetRawValue()) > 0) {
        return false;
    }
    if (GetStringFromLength(end) != string) {
        return false;
    }
    intrinsic->ReplaceUsers(string);
    intrinsic->ClearFlag(inst_flags::NO_DCE);

    return true;
}

static void ReplaceWithCompareNullish(IntrinsicInst *intrinsic, Inst *input)
{
    auto bb = intrinsic->GetBasicBlock();
    auto graph = bb->GetGraph();

    auto compareNull = graph->CreateInstCompare(DataType::BOOL, intrinsic->GetPc(), input, graph->GetOrCreateNullPtr(),
                                                DataType::REFERENCE, ConditionCode::CC_EQ);
    auto compareUniqueObj =
        graph->CreateInstCompare(DataType::BOOL, intrinsic->GetPc(), input, graph->GetOrCreateUniqueObjectInst(),
                                 DataType::REFERENCE, ConditionCode::CC_EQ);

    auto orInst = graph->CreateInstOr(DataType::BOOL, intrinsic->GetPc(), compareNull, compareUniqueObj);

    bb->InsertAfter(compareNull, intrinsic);
    bb->InsertAfter(compareUniqueObj, compareNull);
    bb->InsertAfter(orInst, compareUniqueObj);

    intrinsic->ReplaceUsers(orInst);
}

static bool IsNullish(const Inst *input)
{
    return input->IsNullPtr() || input->IsLoadUniqueObject();
}

static bool ReplaceIfNonValueTyped(IntrinsicInst *intrinsic, compiler::Graph *graph)
{
    auto klass1 = GetClassPtrForObject(intrinsic, 0);
    auto klass2 = GetClassPtrForObject(intrinsic, 1);
    if ((klass1 != nullptr && !graph->GetRuntime()->IsClassValueTyped(klass1)) ||
        (klass2 != nullptr && !graph->GetRuntime()->IsClassValueTyped(klass2))) {
        ReplaceWithCompareEQ(intrinsic);
        return true;
    }
    // NOTE(vpukhov): #16340 try ObjectTypePropagation
    return false;
}

static bool ReplaceWithStdCoreStringEquals(IntrinsicInst *intrinsic, Graph *graph)
{
    // Replaces
    //   .ref inp1 [string only]
    //   .ref inp2 [string only]
    //   .    ss
    //   .b Intrinsic.CompilerEtsEquals inp1, inp2, ss -> ......
    // with
    //   .b Intrinsic.StdCoreStringEquals inp1, inp2, ss -> .....
    //
    // The optimization cannot be applied by BCO because the Intrinsic.StdCoreStringEquals intrinsic is unknown.
    ASSERT(!graph->IsBytecodeOptimizer() && "The peephole cannot be applied by BCO");
    auto *input0 = intrinsic->GetInput(0).GetInst();
    auto *input1 = intrinsic->GetInput(1).GetInst();
    auto *runtime = graph->GetRuntime();

    auto input0TypeInfo = input0->GetObjectTypeInfo();
    if (!input0TypeInfo.IsValid() || input0TypeInfo.GetClass() == nullptr ||
        !runtime->IsStringClass(input0TypeInfo.GetClass())) {
        return false;
    }
    auto input1TypeInfo = input1->GetObjectTypeInfo();
    if (!input1TypeInfo.IsValid() || input1TypeInfo.GetClass() == nullptr ||
        !runtime->IsStringClass(input1TypeInfo.GetClass())) {
        return false;
    }

    ASSERT(intrinsic->RequireState() && "CompilerEtsEquals must have the require_state flag");
    auto compareStrings = graph->CreateInstIntrinsic(DataType::BOOL, intrinsic->GetPc(),
                                                     RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_EQUALS);
    compareStrings->AllocateInputTypes(graph->GetAllocator(), intrinsic->GetInputsCount());
    for (size_t idx = 0; idx < intrinsic->GetInputsCount(); idx++) {
        compareStrings->AppendInput(intrinsic->GetInput(idx));
        compareStrings->AddInputType(intrinsic->GetInputType(idx));
    }

    intrinsic->ReplaceUsers(compareStrings);
    intrinsic->RemoveInputs();
    intrinsic->GetBasicBlock()->ReplaceInst(intrinsic, compareStrings);

    return true;
}

static bool MayBeBoxedFloating(RuntimeInterface::ClassPtr klass, compiler::Graph *graph)
{
    if (klass == nullptr) {
        return true;
    }
    if (graph->GetRuntime()->IsClassBoxedFloat(klass) || graph->GetRuntime()->IsClassBoxedDouble(klass)) {
        return true;
    }
    return false;
}

bool Peepholes::PeepholeEquals([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if ((IsNullish(input0) && IsNullish(input1)) ||
        (input0 == input1 && !MayBeBoxedFloating(GetClassPtrForObject(intrinsic, 0), graph)))  // NaN != NaN
    {
        intrinsic->ReplaceUsers(ConstFoldingCreateIntConst(intrinsic, 1));
        return true;
    }
    if (graph->IsBytecodeOptimizer()) {
        return false;
    }
    if (IsNullish(input0) || IsNullish(input1)) {
        auto nonNullishInput = IsNullish(input0) ? input1 : input0;
        ReplaceWithCompareNullish(intrinsic, nonNullishInput);
        return true;
    }
    return ReplaceWithStdCoreStringEquals(intrinsic, graph) || ReplaceIfNonValueTyped(intrinsic, graph);
}

bool Peepholes::PeepholeStrictEquals([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto input0 = intrinsic->GetInput(0).GetInst();
    auto input1 = intrinsic->GetInput(1).GetInst();
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if (input0 == input1 && !MayBeBoxedFloating(GetClassPtrForObject(intrinsic, 0), graph)) {  // NaN != NaN
        intrinsic->ReplaceUsers(ConstFoldingCreateIntConst(intrinsic, 1));
        return true;
    }

    if (graph->IsBytecodeOptimizer()) {
        return false;
    }

    if (IsNullish(input0) || IsNullish(input1)) {
        ReplaceWithCompareEQ(intrinsic);
        return true;
    }

    return ReplaceWithStdCoreStringEquals(intrinsic, graph) || ReplaceIfNonValueTyped(intrinsic, graph);
}

bool Peepholes::PeepholeTypeof([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return ReplaceTypeofWithIsInstance(intrinsic);
}

static bool HandleIstrueNullAndUnique(Inst *input, IntrinsicInst *intrinsic, Graph *graph)
{
    if (!input->IsNullPtr() && !input->IsLoadUniqueObject()) {
        return false;
    }
    intrinsic->ReplaceUsers(graph->FindOrCreateConstant<bool>(false));
    return true;
}

static bool HandleIstrueFunctionReference(RuntimeInterface::ClassPtr klass, IntrinsicInst *intrinsic, Graph *graph,
                                          RuntimeInterface *runtime)
{
    if (!runtime->IsFunctionReference(klass)) {
        return false;
    }
    intrinsic->ReplaceUsers(graph->FindOrCreateConstant<bool>(true));
    return true;
}

static bool HandleIstrueNonValueTypedClass(RuntimeInterface::ClassPtr klass, Inst *input, IntrinsicInst *intrinsic,
                                           Graph *graph, RuntimeInterface *runtime)
{
    if (runtime->IsClassValueTyped(klass) || input->GetOpcode() != Opcode::NewObject) {
        return false;
    }
    // Assume new objects are always truthy.
    intrinsic->ReplaceUsers(graph->FindOrCreateConstant<bool>(true));
    return true;
}

static bool HandleIstrueBigInt(RuntimeInterface::ClassPtr klass, Inst *input, IntrinsicInst *intrinsic, Graph *graph,
                               RuntimeInterface *runtime)
{
    if (!runtime->IsBigIntClass(klass)) {
        return false;
    }
    auto fieldPtr = runtime->GetFieldPtrByName(input->GetObjectTypeInfo().GetClass(), "sign");
    if (fieldPtr == nullptr) {
        return false;
    }
    auto fieldId = fieldPtr->GetFileId().GetOffset();
    auto loadField =
        graph->CreateInstLoadObject(DataType::INT32, input->GetPc(), input, TypeIdMixin {fieldId, graph->GetMethod()},
                                    fieldPtr, runtime->IsFieldVolatile(fieldPtr));
    intrinsic->InsertBefore(loadField);
    auto checkNonZero = graph->CreateInstCompare(DataType::BOOL, intrinsic->GetPc(), loadField,
                                                 graph->FindOrCreateConstant(0), DataType::INT32, ConditionCode::CC_NE);
    intrinsic->InsertBefore(checkNonZero);
    intrinsic->ReplaceUsers(checkNonZero);
    return true;
}

static Inst *CreateLoadFieldInst(RuntimeInterface::ClassPtr klass, Inst *input, Graph *graph, RuntimeInterface *runtime,
                                 DataType::Type valueType)
{
    auto fieldPtr = runtime->GetFieldPtrByName(klass, "value");
    if (fieldPtr == nullptr) {
        return nullptr;
    }
    auto loadObjectInst =
        graph->CreateInstLoadObject(valueType, input->GetPc(), input, TypeIdMixin {0U, graph->GetMethod()}, fieldPtr,
                                    runtime->IsFieldVolatile(fieldPtr));
    return loadObjectInst;
}

static bool HandleIstrueBoxedClass(RuntimeInterface::ClassPtr klass, Inst *input, IntrinsicInst *intrinsic,
                                   Graph *graph, RuntimeInterface *runtime);

static bool HandleIstrueEnumClass(RuntimeInterface::ClassPtr klass, Inst *input, IntrinsicInst *intrinsic, Graph *graph,
                                  RuntimeInterface *runtime)
{
    if (!runtime->IsEnumClass(klass) || input->GetOpcode() != Opcode::LoadStatic) {
        return false;
    }
    if (input->GetBasicBlock() != intrinsic->GetBasicBlock()) {
        return false;
    }

    RuntimeInterface::ClassPtr boxedKlass = runtime->GetEnumBoxedClass(klass);
    if (boxedKlass == nullptr || !runtime->IsBoxedClass(boxedKlass)) {
        return false;
    }

    auto loadValueInst = CreateLoadFieldInst(klass, input, graph, runtime, DataType::REFERENCE);
    if (loadValueInst == nullptr) {
        return false;
    }
    intrinsic->InsertBefore(loadValueInst);

    return HandleIstrueBoxedClass(boxedKlass, loadValueInst, intrinsic, graph, runtime);
}

static bool HandleIstrueStringClass(RuntimeInterface::ClassPtr klass, Inst *input, IntrinsicInst *intrinsic,
                                    Graph *graph, RuntimeInterface *runtime)
{
    if (!runtime->IsStringClass(klass) || input->GetOpcode() != Opcode::LoadString) {
        return false;
    }
    auto pc = intrinsic->GetPc();
    auto lenArrayInst = graph->CreateInstLenArray(DataType::INT32, pc, input, false);
    auto checkNonZero = graph->CreateInstCompare(DataType::BOOL, pc, lenArrayInst, graph->FindOrCreateConstant(0),
                                                 DataType::INT32, ConditionCode::CC_NE);
    intrinsic->InsertBefore(lenArrayInst);
    intrinsic->InsertBefore(checkNonZero);
    intrinsic->ReplaceUsers(checkNonZero);
    return true;
}

template <typename FloatType, typename IntType>
Inst *CreateIstrueFloatCheck(Graph *graph, BasicBlock *trueBlock, Inst *loadObjectInst, IntrinsicInst *intrinsic)
{
    auto pc = intrinsic->GetPc();

    // NaN check:
    // NaN == NaN => false
    // for any other values: val == val => true
    auto checkIsNotNanInst = graph->CreateInstCompare(
        DataType::BOOL, pc, loadObjectInst, loadObjectInst,
        std::is_same_v<FloatType, float> ? DataType::FLOAT32 : DataType::FLOAT64, ConditionCode::CC_EQ);

    // Check not equal to 0.0.
    auto checkZeroInst = graph->CreateInstCompare(
        DataType::BOOL, pc, loadObjectInst, graph->FindOrCreateConstant<FloatType>(0),
        std::is_same_v<FloatType, float> ? DataType::FLOAT32 : DataType::FLOAT64, ConditionCode::CC_NE);

    // Append instructions to block.
    trueBlock->AppendInst(checkIsNotNanInst);
    trueBlock->AppendInst(checkZeroInst);

    // Combine checks.
    return graph->CreateInstAnd(DataType::BOOL, pc, checkIsNotNanInst, checkZeroInst);
}

static Inst *HandleIstrueNonFloatBoxed(IntrinsicInst *intrinsic, Graph *graph, Inst *loadField,
                                       DataType::Type valueType)
{
    ConstantInst *zeroConst = nullptr;
    switch (valueType) {
        case DataType::BOOL:
            zeroConst = graph->FindOrCreateConstant<bool>(false);
            break;
        case DataType::UINT8:
            zeroConst = graph->FindOrCreateConstant<uint8_t>(0);
            break;
        case DataType::INT8:
            zeroConst = graph->FindOrCreateConstant<int8_t>(0);
            break;
        case DataType::UINT16:
            zeroConst = graph->FindOrCreateConstant<uint16_t>(0);
            break;
        case DataType::INT16:
            zeroConst = graph->FindOrCreateConstant<int16_t>(0);
            break;
        case DataType::UINT32:
            zeroConst = graph->FindOrCreateConstant<uint32_t>(0);
            break;
        case DataType::INT32:
            zeroConst = graph->FindOrCreateConstant<int32_t>(0);
            break;
        case DataType::UINT64:
            zeroConst = graph->FindOrCreateConstant<uint64_t>(0);
            break;
        case DataType::INT64:
            zeroConst = graph->FindOrCreateConstant<int64_t>(0);
            break;
        default:
            UNREACHABLE();
            return nullptr;
    }
    return graph->CreateInstCompare(DataType::BOOL, intrinsic->GetPc(), loadField, zeroConst, valueType,
                                    ConditionCode::CC_NE);
}

static bool HandleIstrueBoxedClass(RuntimeInterface::ClassPtr klass, Inst *input, IntrinsicInst *intrinsic,
                                   Graph *graph, RuntimeInterface *runtime)
{
    if (!runtime->IsBoxedClass(klass)) {
        return false;
    }

    DataType::Type valueType = runtime->GetBoxedClassDataType(klass);
    if (valueType == DataType::NO_TYPE) {
        return false;
    }

    // Load the actual value from the boxed object.
    Inst *loadObjectInst = CreateLoadFieldInst(klass, input, graph, runtime, valueType);
    if (loadObjectInst == nullptr) {
        return false;
    }

    // Check if the boxed object is `nullptr` (comparison + conditional branch).
    auto compareNullPtrInst =
        graph->CreateInstCompare(DataType::BOOL, intrinsic->GetPc(), input, graph->GetOrCreateNullPtr(),
                                 DataType::REFERENCE, ConditionCode::CC_EQ);
    auto ifImmInst = graph->CreateInstIfImm(DataType::BOOL, intrinsic->GetPc(), compareNullPtrInst, 0U, DataType::BOOL,
                                            ConditionCode::CC_EQ);

    intrinsic->InsertBefore(compareNullPtrInst);
    intrinsic->InsertBefore(ifImmInst);

    // Split the current block to handle nullptr and non-nullptr cases.
    auto resultBB = intrinsic->GetBasicBlock()->SplitBlockAfterInstruction(ifImmInst, false);
    auto phiInst = graph->CreateInstPhi(DataType::BOOL, intrinsic->GetPc());
    resultBB->AppendPhi(phiInst);

    // Create "true" block (object is not nullptr).
    auto ifImmBB = ifImmInst->GetBasicBlock();
    auto trueBlock = graph->CreateEmptyBlock(ifImmBB);
    trueBlock->AddSucc(resultBB);
    ifImmBB->GetLoop()->AppendBlock(trueBlock);
    ifImmBB->AddSucc(trueBlock);

    // Create "false" block (object is nullptr).
    auto falseBlock = graph->CreateEmptyBlock(ifImmBB);
    falseBlock->AddSucc(resultBB);
    ifImmBB->GetLoop()->AppendBlock(falseBlock);
    ifImmBB->AddSucc(falseBlock);

    trueBlock->AppendInst(loadObjectInst);

    // Generate the actual truthiness check depending on the boxed type.
    Inst *result = nullptr;
    if (valueType == DataType::FLOAT64) {
        result = CreateIstrueFloatCheck<double, int64_t>(graph, trueBlock, loadObjectInst, intrinsic);
    } else if (valueType == DataType::FLOAT32) {
        result = CreateIstrueFloatCheck<float, int32_t>(graph, trueBlock, loadObjectInst, intrinsic);
    } else {
        result = HandleIstrueNonFloatBoxed(intrinsic, graph, loadObjectInst, valueType);
    }

    trueBlock->AppendInst(result);

    // Build the PHI node: result if non-null, false if null.
    phiInst->AppendInput(result);
    phiInst->AppendInput(graph->FindOrCreateConstant<bool>(false));
    intrinsic->ReplaceUsers(phiInst);

    graph->RunPass<DominatorsTree>();
    graph->InvalidateAnalysis<LoopAnalyzer>();
    graph->RunPass<LoopAnalyzer>();

    return true;
}

bool Peepholes::PeepholeIstrue([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] IntrinsicInst *intrinsic)
{
    if (intrinsic->GetIntrinsicId() != RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_ISTRUE ||
        !intrinsic->HasUsers()) {
        return false;
    }

    auto input = intrinsic->GetInput(0).GetInst();
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    if (HandleIstrueNullAndUnique(input, intrinsic, graph)) {
        return true;
    }

    auto klass = input->GetObjectTypeInfo().GetClass();
    if (klass == nullptr) {
        return false;
    }

    return (HandleIstrueFunctionReference(klass, intrinsic, graph, runtime) ||
            HandleIstrueNonValueTypedClass(klass, input, intrinsic, graph, runtime) ||
            HandleIstrueBigInt(klass, input, intrinsic, graph, runtime) ||
            HandleIstrueEnumClass(klass, input, intrinsic, graph, runtime) ||
            HandleIstrueStringClass(klass, input, intrinsic, graph, runtime) ||
            HandleIstrueBoxedClass(klass, input, intrinsic, graph, runtime));
}

bool Peepholes::PeepholeIntShortToString([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    ASSERT(intrinsic->GetInputsCount() == 2U);
    ASSERT(intrinsic->GetInput(1U).GetInst()->IsSaveState());
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || graph->GetArch() == Arch::AARCH32 ||
        !graph->GetRuntime()->IsStringCachesUsed()) {
        return false;
    }
    auto pc = intrinsic->GetPc();
    auto num = intrinsic->GetInput(0).GetInst();
    auto numInt = graph->CreateInstCast(DataType::INT64, pc, num, num->GetType());
    auto newInst = graph->CreateInstIntrinsic(
        DataType::REFERENCE, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LONG_TO_STRING_DECIMAL);
    newInst->SetInputs(graph->GetAllocator(), {{numInt, DataType::INT64},
                                               {graph->FindOrCreateConstant<uint64_t>(0), DataType::UINT64},
                                               {intrinsic->GetSaveState(), DataType::NO_TYPE}});
    intrinsic->InsertBefore(numInt);
    intrinsic->InsertBefore(newInst);
    intrinsic->ReplaceUsers(newInst);
    // remove intrinsic to satisfy SaveState checker
    intrinsic->GetBasicBlock()->RemoveInst(intrinsic);
    intrinsic->SetNext(newInst->GetNext());
    return true;
}

bool Peepholes::PeepholeLongToString([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    ASSERT(intrinsic->GetInputsCount() == 2U);
    ASSERT(intrinsic->GetInput(1U).GetInst()->IsSaveState());
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || graph->GetArch() == Arch::AARCH32 ||
        !graph->GetRuntime()->IsStringCachesUsed()) {
        return false;
    }
    auto pc = intrinsic->GetPc();
    auto num = intrinsic->GetInput(0).GetInst();
    auto newInst = graph->CreateInstIntrinsic(
        DataType::REFERENCE, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_LONG_TO_STRING_DECIMAL);
    newInst->SetInputs(graph->GetAllocator(), {{num, DataType::INT64},
                                               {graph->FindOrCreateConstant<uint64_t>(0), DataType::UINT64},
                                               {intrinsic->GetSaveState(), DataType::NO_TYPE}});
    intrinsic->InsertBefore(newInst);
    intrinsic->ReplaceUsers(newInst);
    // remove intrinsic to satisfy SaveState checker
    intrinsic->GetBasicBlock()->RemoveInst(intrinsic);
    intrinsic->SetNext(newInst->GetNext());
    return true;
}

bool Peepholes::PeepholeFloatToString([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    ASSERT(intrinsic->GetInputsCount() == 3U);
    ASSERT(intrinsic->GetInput(2U).GetInst()->IsSaveState());
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || graph->GetArch() == Arch::AARCH32 ||
        !graph->GetRuntime()->IsStringCachesUsed()) {
        return false;
    }
    auto radix = intrinsic->GetInput(1U).GetInst();
    constexpr auto TEN = 10U;
    if (!(radix->IsConst() && radix->CastToConstant()->GetIntValue() == TEN)) {
        return false;
    }
    auto pc = intrinsic->GetPc();
    auto num = intrinsic->GetInput(0).GetInst();
    auto numInt = graph->CreateInstBitcast(DataType::UINT32, pc, num);
    auto newInst = graph->CreateInstIntrinsic(
        DataType::REFERENCE, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_FLOAT_TO_STRING_DECIMAL);
    newInst->SetInputs(graph->GetAllocator(), {{numInt, DataType::UINT32},
                                               {graph->FindOrCreateConstant<uint64_t>(0), DataType::UINT64},
                                               {intrinsic->GetSaveState(), DataType::NO_TYPE}});
    intrinsic->InsertBefore(numInt);
    intrinsic->InsertBefore(newInst);
    intrinsic->ReplaceUsers(newInst);
    // remove intrinsic to satisfy SaveState checker
    intrinsic->GetBasicBlock()->RemoveInst(intrinsic);
    intrinsic->SetNext(newInst->GetNext());
    return true;
}

bool Peepholes::PeepholeDoubleToString([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    ASSERT(intrinsic->GetInputsCount() == 3U);
    ASSERT(intrinsic->GetInput(2U).GetInst()->IsSaveState());
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || graph->GetArch() == Arch::AARCH32 ||
        !graph->GetRuntime()->IsStringCachesUsed()) {
        return false;
    }
    auto radix = intrinsic->GetInput(1U).GetInst();
    constexpr auto TEN = 10U;
    if (!(radix->IsConst() && radix->CastToConstant()->GetIntValue() == TEN)) {
        return false;
    }
    auto pc = intrinsic->GetPc();
    auto num = intrinsic->GetInput(0).GetInst();
    auto numInt = graph->CreateInstBitcast(DataType::UINT64, pc, num);
    auto newInst = graph->CreateInstIntrinsic(
        DataType::REFERENCE, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_DOUBLE_TO_STRING_DECIMAL);
    newInst->SetInputs(graph->GetAllocator(), {{numInt, DataType::UINT64},
                                               {graph->FindOrCreateConstant<uint64_t>(0), DataType::UINT64},
                                               {intrinsic->GetSaveState(), DataType::NO_TYPE}});
    intrinsic->InsertBefore(numInt);
    intrinsic->InsertBefore(newInst);
    intrinsic->ReplaceUsers(newInst);
    // remove intrinsic to satisfy SaveState checker
    intrinsic->GetBasicBlock()->RemoveInst(intrinsic);
    intrinsic->SetNext(newInst->GetNext());
    return true;
}

bool Peepholes::PeepholeNullcheck([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    auto input = intrinsic->GetInput(0).GetInst();
    if (IsInstNotNull(input)) {
        intrinsic->ReplaceUsers(input);
        intrinsic->ClearFlag(inst_flags::NO_DCE);
        return true;
    }
    return false;
}

bool Peepholes::PeepholeStringFromCharCodeSingle([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    // Implements the following optimization for the Intrinsic.StdCoreStringFromCharCodeSingle intrinsic:
    //
    // before:
    //   7.f64  Cast i32                   v0 -> (...,8)
    //   9.     SaveState                  v7(ACC), inlining_depth=0 -> (...,8)
    //   8.ref  Intrinsic.StdCoreStringFromCharCodeSingle v7, v9 -> (v10)
    //
    // after JIT:
    //    7.f64  Cast i32                   v0 -> (v11, v9)
    //    9.     SaveState                  v7(ACC), inlining_depth=0 -> (v13)
    //   12.ref  LoadImmediate(<the pointer to the cache of short (one-byte length) strings>) (v13)
    //   11.u64  Bitcast                    v7 -> (v13)
    //   13.ref  Intrinsic.CompilerEtsStringFromCharCodeSingle[NoCache] [v12,] v11, v9 -> (v10)
    //
    // after AOT:
    //    7.f64  Cast i32                   v0 -> (v11, v9)
    //    9.     SaveState                  v7(ACC), inlining_depth=0 -> (v15)
    //   12.ptr  LoadImmediate(<TlsOffset: to the cache of short (one-byte length) strings>) (v14)
    //   14.ref  Load 1                     v12, v13 -> (v15)
    //   11.u64  Bitcast                    v7 -> (v15)
    //   15.ref  Intrinsic.CompilerEtsStringFromCharCodeSingle[NoCache] [v14,] v11, v9 -> (v10)
    //
    // (the using of the `NoCache` suffix and the first input (the cache) depends on whether the ark/ark_aot
    // is run with the --use-string-caches=true argument.)

    ASSERT(intrinsic->GetInputsCount() == 2U);
    ASSERT(intrinsic->GetInput(1U).GetInst()->IsSaveState());
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    if (graph->IsBytecodeOptimizer() || graph->GetArch() == Arch::AARCH32) {
        return false;
    }

    auto pc = intrinsic->GetPc();
    auto *charCode = intrinsic->GetInput(0).GetInst();
    auto charCodeInt = graph->CreateInstCast(DataType::INT32, pc, charCode);
    IntrinsicInst *createStringInst = nullptr;
    if (graph->GetRuntime()->IsStringCachesUsed()) {
        Inst *cache = nullptr;
        void *cachePtr = nullptr;
        if (!graph->IsAotMode() && (cachePtr = graph->GetRuntime()->GetAsciiCharCache()) != nullptr) {
            cache = graph->CreateInstLoadImmediate(DataType::REFERENCE, pc, cachePtr,
                                                   LoadImmediateInst::ObjectType::OBJECT);
        } else {
            auto ls = graph->CreateInstLoadImmediate(DataType::POINTER, pc,
                                                     cross_values::GetEtsCoroutineLocalStorageOffset(graph->GetArch()),
                                                     LoadImmediateInst::ObjectType::TLS_OFFSET);
            intrinsic->InsertBefore(ls);
            cache = graph->CreateInstLoad(
                DataType::REFERENCE, pc, ls,
                graph->FindOrCreateConstant(cross_values::GetPlatformTypesAsciiCharCacheOffset(graph->GetArch())));
            // GraphChecker hack
            cache->ClearFlag(inst_flags::LOW_LEVEL);
        }
        intrinsic->InsertBefore(cache);
        createStringInst = graph->CreateInstIntrinsic(
            DataType::REFERENCE, pc,
            RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_STRING_FROM_CHAR_CODE_SINGLE);
        createStringInst->SetInputs(graph->GetAllocator(), {{cache, DataType::REFERENCE},
                                                            {charCodeInt, DataType::INT32},
                                                            {intrinsic->GetSaveState(), DataType::NO_TYPE}});
    } else {
        createStringInst = graph->CreateInstIntrinsic(
            DataType::REFERENCE, pc,
            RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_STRING_FROM_CHAR_CODE_SINGLE_NO_CACHE);
        createStringInst->SetInputs(graph->GetAllocator(),
                                    {{charCodeInt, DataType::INT32}, {intrinsic->GetSaveState(), DataType::NO_TYPE}});
    }
    intrinsic->InsertBefore(charCodeInt);
    intrinsic->ReplaceUsers(createStringInst);
    intrinsic->RemoveInputs();
    intrinsic->GetBasicBlock()->ReplaceInst(intrinsic, createStringInst);
    return true;
}

#ifdef PANDA_ETS_INTEROP_JS

bool Peepholes::TryFuseGetPropertyAndCast(IntrinsicInst *intrinsic, RuntimeInterface::IntrinsicId newId)
{
    auto input = intrinsic->GetInput(0).GetInst();
    if (!input->HasSingleUser() || input->GetBasicBlock() != intrinsic->GetBasicBlock()) {
        return false;
    }
    if (!input->IsIntrinsic() || input->CastToIntrinsic()->GetIntrinsicId() !=
                                     RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_JS_VALUE) {
        return false;
    }
    for (auto inst = input->GetNext(); inst != intrinsic; inst = inst->GetNext()) {
        ASSERT(inst != nullptr);
        if (inst->IsNotRemovable()) {
            return false;
        }
    }
    input->CastToIntrinsic()->SetIntrinsicId(newId);
    input->SetType(intrinsic->GetType());
    intrinsic->ReplaceUsers(input);
    intrinsic->GetBasicBlock()->RemoveInst(intrinsic);
    intrinsic->SetNext(input->GetNext());  // Fix for InstForwardIterator in Peepholes visitor
    return true;
}

bool Peepholes::PeepholeJSRuntimeGetValueString([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseGetPropertyAndCast(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_STRING);
}

bool Peepholes::PeepholeJSRuntimeGetValueDouble([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseGetPropertyAndCast(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_DOUBLE);
}

bool Peepholes::PeepholeJSRuntimeGetValueBoolean([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseGetPropertyAndCast(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_BOOLEAN);
}

bool Peepholes::TryFuseCastAndSetProperty(IntrinsicInst *intrinsic, RuntimeInterface::IntrinsicId newId)
{
    size_t userCount = 0;
    constexpr size_t STORE_VALUE_IDX = 2;
    constexpr auto SET_PROP_ID = RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_JS_VALUE;
    for (auto &user : intrinsic->GetUsers()) {
        ++userCount;
        if (user.GetIndex() != STORE_VALUE_IDX || !user.GetInst()->IsIntrinsic() ||
            user.GetInst()->CastToIntrinsic()->GetIntrinsicId() != SET_PROP_ID) {
            return false;
        }
    }
    auto userIt = intrinsic->GetUsers().begin();
    for (size_t userIdx = 0; userIdx < userCount; ++userIdx) {
        ASSERT(userIt != intrinsic->GetUsers().end());
        auto *storeInst = userIt->GetInst();
        auto newValue = intrinsic->GetInput(0).GetInst();
        storeInst->CastToIntrinsic()->SetIntrinsicId(newId);
        storeInst->ReplaceInput(intrinsic, newValue);
        storeInst->CastToIntrinsic()->SetInputType(STORE_VALUE_IDX, newValue->GetType());
        userIt = intrinsic->GetUsers().begin();
    }
    return true;
}

bool Peepholes::PeepholeJSRuntimeNewJSValueString(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseCastAndSetProperty(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_STRING);
}

bool Peepholes::PeepholeJSRuntimeNewJSValueDouble(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseCastAndSetProperty(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_DOUBLE);
}

bool Peepholes::PeepholeJSRuntimeNewJSValueBoolean(GraphVisitor *v, IntrinsicInst *intrinsic)
{
    return static_cast<Peepholes *>(v)->TryFuseCastAndSetProperty(
        intrinsic, RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_BOOLEAN);
}

static std::pair<Inst *, Inst *> BuildLoadPropertyChain(IntrinsicInst *intrinsic, uint64_t qnameStart,
                                                        uint64_t qnameLen)
{
    auto *jsThis = intrinsic->GetInput(0).GetInst();
    auto *jsFn = jsThis;
    auto saveState = intrinsic->GetSaveState();
    auto graph = intrinsic->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto klass = runtime->GetClass(intrinsic->GetMethod());
    auto pc = intrinsic->GetPc();
    auto jsConstPool = graph->CreateInstIntrinsic(
        DataType::POINTER, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_LOAD_JS_CONSTANT_POOL);
    jsConstPool->SetInputs(graph->GetAllocator(), {{saveState, DataType::NO_TYPE}});
    intrinsic->InsertBefore(jsConstPool);
    Inst *cpOffsetForClass = intrinsic->GetInput(3U).GetInst();
    for (uint32_t strIndexInAnnot = qnameStart; strIndexInAnnot < qnameStart + qnameLen; strIndexInAnnot++) {
        IntrinsicInst *jsProperty = nullptr;
        auto uniqueStrIndex =
            graph->GetRuntime()->GetAnnotationElementUniqueIndex(klass, "###33445-deprecated", strIndexInAnnot);
        auto strIndexInAnnotConst = graph->FindOrCreateConstant(uniqueStrIndex);
        auto indexInst = graph->CreateInstAdd(DataType::INT32, pc, cpOffsetForClass, strIndexInAnnotConst);
        intrinsic->InsertBefore(indexInst);
        jsProperty = graph->CreateInstIntrinsic(DataType::POINTER, pc,
                                                RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_JS_ELEMENT);
        // ConstantPool elements are immutable
        jsProperty->ClearFlag(inst_flags::NO_CSE);
        jsProperty->ClearFlag(inst_flags::NO_DCE);

        jsProperty->SetInputs(
            graph->GetAllocator(),
            {{jsConstPool, DataType::POINTER}, {indexInst, DataType::INT32}, {saveState, DataType::NO_TYPE}});

        auto getProperty = graph->CreateInstIntrinsic(
            DataType::POINTER, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_JS_PROPERTY);
        getProperty->SetInputs(
            graph->GetAllocator(),
            {{jsFn, DataType::POINTER}, {jsProperty, DataType::POINTER}, {saveState, DataType::NO_TYPE}});
        intrinsic->InsertBefore(jsProperty);
        intrinsic->InsertBefore(getProperty);
        jsThis = jsFn;
        jsFn = getProperty;
    }
    return {jsThis, jsFn};
}

bool Peepholes::PeepholeResolveQualifiedJSCall([[maybe_unused]] GraphVisitor *v, IntrinsicInst *intrinsic)
{
    if (!intrinsic->HasUsers()) {
        return false;
    }
    auto qnameStartInst = intrinsic->GetInput(1).GetInst();
    auto qnameLenInst = intrinsic->GetInput(2U).GetInst();
    if (!qnameStartInst->IsConst() || !qnameLenInst->IsConst()) {
        // qnameStart and qnameLen are always constant, but may be e.g. Phi instructions after BCO
        return false;
    }
    auto qnameStart = qnameStartInst->CastToConstant()->GetIntValue();
    auto qnameLen = qnameLenInst->CastToConstant()->GetIntValue();
    ASSERT(qnameLen > 0);
    auto [jsThis, jsFn] = BuildLoadPropertyChain(intrinsic, qnameStart, qnameLen);
    constexpr auto FN_PSEUDO_USER = RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_LOAD_RESOLVED_JS_FUNCTION;
    for (auto &user : intrinsic->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsIntrinsic() && userInst->CastToIntrinsic()->GetIntrinsicId() == FN_PSEUDO_USER) {
            userInst->ReplaceUsers(jsFn);
        }
    }
    intrinsic->ReplaceUsers(jsThis);
    intrinsic->ClearFlag(inst_flags::NO_DCE);
    return true;
}

#endif

}  // namespace ark::compiler
