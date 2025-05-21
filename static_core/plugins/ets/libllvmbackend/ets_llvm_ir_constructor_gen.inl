/*
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

bool LLVMIrConstructor::EmitArrayCopyTo(Inst *inst)
{
    RuntimeInterface::EntrypointId eid;
    switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BOOL_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_1B;
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_2B;
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_4B;
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_COPY_TO:
            eid = RuntimeInterface::EntrypointId::ARRAY_COPY_TO_8B;
            break;
        default:
            UNREACHABLE();
            break;
    }

    CreateFastPathCall(inst, eid,
                       {GetInputValue(inst, 0), GetInputValue(inst, 1), GetInputValue(inst, 2), GetInputValue(inst, 3),
                        GetInputValue(inst, 4)});
    // Fastpath doesn't return anything, but result is in 'dst' arg, which is second
    ValueMapAdd(inst, GetInputValue(inst, 1));
    return true;
}

bool LLVMIrConstructor::EmitStdStringSubstring(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::SUB_STRING_FROM_STRING_TLAB_COMPRESSED, 3U);
}

bool LLVMIrConstructor::EmitStringBuilderAppendBool(Inst *inst)
{
    auto offset = GetGraph()->GetRuntime()->GetArrayU16ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_BOOL;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), GetInputValue(inst, 1), klass});
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendChar(Inst *inst)
{
    auto offset = GetGraph()->GetRuntime()->GetArrayU16ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_CHAR_COMPRESSED;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), GetInputValue(inst, 1), klass});
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendByte(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendShort(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendInt(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendLong(Inst *inst)
{
    auto call = CreateStringBuilderAppendLong(inst);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringBuilderAppendString(Inst *inst)
{
    auto result = CreateStringBuilderAppendString(inst);
    ValueMapAdd(inst, result);
    return true;
}

RuntimeInterface::EntrypointId GetAppendStringsEntrypoint(uint32_t numArgs, mem::BarrierType barrierType)
{
    using EntrypointId = RuntimeInterface::EntrypointId;
    switch (barrierType) {
        case mem::BarrierType::POST_INTERGENERATIONAL_BARRIER: {  // Gen GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_ASYNC_MANUAL,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_ASYNC_MANUAL,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_ASYNC_MANUAL,
            };
            return entrypoints[numArgs];
        }
        case mem::BarrierType::POST_INTERREGION_BARRIER: {  // G1 GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_ASYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_ASYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_ASYNC,
            };
            return entrypoints[numArgs];
        }
        default: {  // STW GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_SYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_SYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_SYNC,
            };
            return entrypoints[numArgs];
        }
    }
}

bool LLVMIrConstructor::EmitStringBuilderAppendStrings(Inst *inst)
{
    ASSERT(GetGraph()->GetRuntime()->IsCompressedStringsEnabled());
    RuntimeInterface::EntrypointId eid = RuntimeInterface::EntrypointId::INVALID;
    switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2:
            eid = GetAppendStringsEntrypoint(2U, GetGraph()->GetRuntime()->GetPostType());
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3:
            eid = GetAppendStringsEntrypoint(3U, GetGraph()->GetRuntime()->GetPostType());
            break;
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4:
            eid = GetAppendStringsEntrypoint(4U, GetGraph()->GetRuntime()->GetPostType());
            break;
        default:
            UNREACHABLE();
    }
    return EmitFastPath(inst, eid, inst->GetInputsCount() - 1U);  // -1 to skip save state
}

bool LLVMIrConstructor::EmitStringBuilderToString(Inst *inst)
{
    auto offset = GetGraph()->GetRuntime()->GetStringClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_TO_STRING;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), klass});
    ValueMapAdd(inst, call);
    return true;
}

llvm::Value *LLVMIrConstructor::CreateStringBuilderAppendLong(Inst *inst)
{
    auto sb = GetInputValue(inst, 0);
    auto value = builder_.CreateSExt(GetInputValue(inst, 1), builder_.getInt64Ty());
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_LONG;
    auto offset = GetGraph()->GetRuntime()->GetArrayU16ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    return CreateFastPathCall(inst, eid, {sb, value, klass});
}

llvm::Value *LLVMIrConstructor::CreateStringBuilderAppendString(Inst *inst)
{
    auto sb = GetInputValue(inst, 0);
    auto str = GetInputValue(inst, 1);
    auto *strInst = inst->GetInput(1).GetInst();
    auto eidNull = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_NULL_STRING;
    auto eid = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_STRING;
    if (strInst->IsNullPtr()) {
        return CreateEntrypointCall(eidNull, inst, {sb});
    }
    auto maybeNull = !IsInstNotNull(strInst);
    auto branchWeights = llvm::MDBuilder(func_->getContext())
                             .createBranchWeights(llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT,
                                                  llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT);
    auto contBb = llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_cont"), func_);
    auto slowPathBb =
        llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_slow_path"), func_);
    auto curBb = GetCurrentBasicBlock();

    SetCurrentBasicBlock(contBb);
    auto result = builder_.CreatePHI(sb->getType(), maybeNull ? 4U : 3U);

    SetCurrentBasicBlock(curBb);
    if (maybeNull) {
        StringBuilderAppendStringNull(inst, result, contBb);
    }

    auto fastBb = llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_fast_check"), func_);
    auto sbBufferOffset =
        builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), sb, RuntimeInterface::GetSbBufferOffset());
    auto sbBuffer = builder_.CreateLoad(builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE), sbBufferOffset);
    auto bufferLengthOffset = builder_.CreateConstInBoundsGEP1_32(
        builder_.getInt8Ty(), sbBuffer, GetGraph()->GetRuntime()->GetArrayLengthOffset(GetGraph()->GetArch()));
    auto bufferLength = builder_.CreateLoad(builder_.getInt32Ty(), bufferLengthOffset);
    auto sbIndexOffset =
        builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), sb, RuntimeInterface::GetSbIndexOffset());
    auto sbIndex = builder_.CreateLoad(builder_.getInt32Ty(), sbIndexOffset);
    auto iCmp = builder_.CreateICmpULT(sbIndex, bufferLength);
    builder_.CreateCondBr(iCmp, fastBb, slowPathBb, branchWeights);

    SetCurrentBasicBlock(fastBb);
    StringBuilderAppendStringMain(inst, sbIndexOffset, sbIndex, sbBuffer, contBb);
    result->addIncoming(sb, GetCurrentBasicBlock());

    SetCurrentBasicBlock(slowPathBb);
    auto slowPath = CreateEntrypointCall(eid, inst, {sb, str});
    builder_.CreateBr(contBb);
    result->addIncoming(sb, fastBb);
    result->addIncoming(slowPath, slowPathBb);
    SetCurrentBasicBlock(contBb);
    return result;
}

void LLVMIrConstructor::StringBuilderAppendStringNull(Inst *inst, llvm::PHINode *result, llvm::BasicBlock *contBb)
{
    auto sb = GetInputValue(inst, 0);
    auto str = GetInputValue(inst, 1);
    auto eidNull = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_NULL_STRING;
    auto nullBb = llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_null"), func_);
    auto bufCheckBb =
        llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_buffer_check"), func_);
    auto strNull = builder_.CreateICmpEQ(str, llvm::Constant::getNullValue(str->getType()));
    builder_.CreateCondBr(strNull, nullBb, bufCheckBb);

    SetCurrentBasicBlock(nullBb);
    auto resultNull = CreateEntrypointCall(eidNull, inst, {sb});
    result->addIncoming(resultNull, nullBb);
    builder_.CreateBr(contBb);

    SetCurrentBasicBlock(bufCheckBb);
}

void LLVMIrConstructor::StringBuilderAppendStringMain(Inst *inst, llvm::Value *sbIndexOffset, llvm::Value *sbIndex,
                                                      llvm::Value *sbBuffer, llvm::BasicBlock *contBb)
{
    auto sb = GetInputValue(inst, 0);
    auto str = GetInputValue(inst, 1);
    auto runtime = GetGraph()->GetRuntime();
    auto arch = GetGraph()->GetArch();

    auto mainBb = llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_main"), func_);
    auto checkCompressedBb =
        llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_check_compressed"), func_);
    auto setCompressedBb =
        llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_set_compressed"), func_);
    auto insertStringBb =
        llvm::BasicBlock::Create(func_->getContext(), CreateBasicBlockName(inst, "sb_str_insert_string"), func_);

    auto strLengthOffset =
        builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), str, runtime->GetStringLengthOffset(arch));
    auto strLength = builder_.CreateLoad(builder_.getInt32Ty(), strLengthOffset);
    auto strLengthShr = builder_.CreateLShr(strLength, builder_.getInt32(1));
    auto strLengthZero = builder_.CreateICmpEQ(strLengthShr, builder_.getInt32(0));
    builder_.CreateCondBr(strLengthZero, contBb, mainBb);

    SetCurrentBasicBlock(mainBb);
    auto sbStrLengthOffset =
        builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), sb, RuntimeInterface::GetSbLengthOffset());
    auto sbStrLength = builder_.CreateLoad(builder_.getInt32Ty(), sbStrLengthOffset);
    builder_.CreateStore(builder_.CreateAdd(sbStrLength, strLengthShr), sbStrLengthOffset);
    auto isCompressed = builder_.CreateTrunc(strLength, builder_.getInt1Ty());
    builder_.CreateCondBr(isCompressed, checkCompressedBb, insertStringBb);

    SetCurrentBasicBlock(checkCompressedBb);
    auto sbCompressOffset =
        builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), sb, RuntimeInterface::GetSbCompressOffset());
    auto sbCompress = builder_.CreateLoad(builder_.getInt8Ty(), sbCompressOffset);
    auto sbCompressZero = builder_.CreateICmpEQ(sbCompress, llvm::Constant::getNullValue(builder_.getInt8Ty()));
    builder_.CreateCondBr(sbCompressZero, insertStringBb, setCompressedBb);

    SetCurrentBasicBlock(setCompressedBb);
    builder_.CreateStore(llvm::Constant::getNullValue(builder_.getInt8Ty()), sbCompressOffset);
    builder_.CreateBr(insertStringBb);

    SetCurrentBasicBlock(insertStringBb);
    auto shiftValue = DataType::ShiftByType(DataType::REFERENCE, GetGraph()->GetArch());
    auto sbIndexShift = builder_.CreateShl(sbIndex, builder_.getInt32(shiftValue));
    auto sbDataOffset = builder_.CreateAdd(sbIndexShift, builder_.getInt32(runtime->GetArrayDataOffset(arch)));
    auto sbDataInsertPoint = builder_.CreateInBoundsGEP(builder_.getInt8Ty(), sbBuffer, sbDataOffset);
    builder_.CreateStore(builder_.CreateAdd(sbIndex, builder_.getInt32(1)), sbIndexOffset);
    CreatePreWRB(inst, sbDataInsertPoint);
    builder_.CreateStore(str, sbDataInsertPoint);
    CreatePostWRB(inst, sbBuffer, sbDataOffset, str);
    builder_.CreateBr(contBb);
}

bool LLVMIrConstructor::EmitDoubleToStringDecimal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::DOUBLE_TO_STRING_DECIMAL, 3U);
}

bool LLVMIrConstructor::EmitStringTrimLeft(Inst *inst)
{
    auto eid = RuntimeInterface::EntrypointId::STRING_TRIM_LEFT;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), builder_.getInt32(0), builder_.getInt32(0)});
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringTrimRight(Inst *inst)
{
    auto eid = RuntimeInterface::EntrypointId::STRING_TRIM_RIGHT;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), builder_.getInt32(0), builder_.getInt32(0)});
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitStringTrim(Inst *inst)
{
    auto eid = RuntimeInterface::EntrypointId::STRING_TRIM;
    auto call = CreateFastPathCall(inst, eid, {GetInputValue(inst, 0), builder_.getInt32(0), builder_.getInt32(0)});
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitCharIsWhiteSpace(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::CHAR_IS_WHITE_SPACE, 1U);
}

bool LLVMIrConstructor::EmitStringStartsWith(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_STARTS_WITH, 3U);
}

bool LLVMIrConstructor::EmitStringEndsWith(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_ENDS_WITH, 3U);
}

bool LLVMIrConstructor::EmitStringGetBytesTlab(Inst *inst)
{
    auto offset = GetGraph()->GetRuntime()->GetArrayU8ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    auto eid = RuntimeInterface::EntrypointId::STRING_GET_BYTES_TLAB;
    auto result = CreateEntrypointCall(eid, inst,
                                       {GetInputValue(inst, 0), GetInputValue(inst, 1), GetInputValue(inst, 2), klass});
    ASSERT(result->getCallingConv() == llvm::CallingConv::C);
    result->setCallingConv(llvm::CallingConv::ArkFast4);
    result->addRetAttr(llvm::Attribute::NonNull);
    result->addRetAttr(llvm::Attribute::NoAlias);
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitStringIndexOf(Inst *inst)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return false;
    }
    auto entrypointId = RuntimeInterface::EntrypointId::STRING_INDEX_OF;
    auto str = GetInputValue(inst, 0);
    auto ch = GetInputValue(inst, 1);
    auto zero = builder_.getInt32(0);
    auto result = CreateFastPathCall(inst, entrypointId, {str, ch, zero});
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitStringIndexOfAfter(Inst *inst)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return false;
    }
    auto entrypointId = RuntimeInterface::EntrypointId::STRING_INDEX_OF_AFTER;
    auto str = GetInputValue(inst, 0);
    auto ch = GetInputValue(inst, 1);
    auto pos = GetInputValue(inst, 2);
    auto zero = builder_.getInt32(0);
    auto firstBb = GetCurrentBasicBlock();
    auto charFoundBb = llvm::BasicBlock::Create(func_->getContext(),
                                                CreateBasicBlockName(inst, "emit_string_index_of_after_found"), func_);
    auto returnBb = llvm::BasicBlock::Create(func_->getContext(),
                                             CreateBasicBlockName(inst, "emit_string_index_of_after_ret"), func_);
    auto isNegative = builder_.CreateICmpSLT(pos, zero);
    auto startIndex = builder_.CreateSelect(isNegative, zero, pos);
    auto charIndex1 = CreateFastPathCall(inst, entrypointId, {str, ch, startIndex});
    // Irtoc implementation of INDEX_OF_AFTER calls the corresponding INDEX_OF_ variant.
    // That is done for the optimization purpose.
    // So here we must add 'startIndex' to the returned 'charIndex' (if it's not equal to '-1')
    builder_.CreateCondBr(builder_.CreateICmpEQ(charIndex1, builder_.getInt32(-1)), returnBb, charFoundBb);
    SetCurrentBasicBlock(charFoundBb);
    auto charIndex2 = builder_.CreateAdd(charIndex1, startIndex);
    builder_.CreateBr(returnBb);
    SetCurrentBasicBlock(returnBb);
    auto charIndex = builder_.CreatePHI(builder_.getInt32Ty(), 2U);
    charIndex->addIncoming(charIndex1, firstBb);
    charIndex->addIncoming(charIndex2, charFoundBb);
    ValueMapAdd(inst, charIndex);
    return true;
}

bool LLVMIrConstructor::EmitStringFromCharCode(Inst *inst)
{
    ASSERT(GetGraph()->GetRuntime()->IsCompressedStringsEnabled());
    auto getEntryId = [inst]() {
        switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
            case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_FROM_CHAR_CODE:
                return RuntimeInterface::EntrypointId::CREATE_STRING_FROM_CHAR_CODE_TLAB_COMPRESSED;
            case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_STRING_FROM_CHAR_CODE_SINGLE:
                return RuntimeInterface::EntrypointId::CREATE_STRING_FROM_CHAR_CODE_SINGLE_TLAB_COMPRESSED;
            default:
                UNREACHABLE();
        }
    };
    auto klassOffset = GetGraph()->GetRuntime()->GetStringClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, klassOffset, builder_.getPtrTy());
    auto call = CreateFastPathCall(inst, getEntryId(), {GetInputValue(inst, 0), klass});
    MarkAsAllocation(call);
    ValueMapAdd(inst, call);
    return true;
}

bool LLVMIrConstructor::EmitFloat32ArrayFillInternal(Inst *inst)
{
    return EmitFloatArrayFillInternal(inst, RuntimeInterface::EntrypointId::FLOAT32_ARRAY_FILL_INTERNAL_FAST_PATH,
                                      builder_.getInt32Ty());
}

bool LLVMIrConstructor::EmitFloat64ArrayFillInternal(Inst *inst)
{
    return EmitFloatArrayFillInternal(inst, RuntimeInterface::EntrypointId::FLOAT64_ARRAY_FILL_INTERNAL_FAST_PATH,
                                      builder_.getInt64Ty());
}

bool LLVMIrConstructor::EmitFloatArrayFillInternal(Inst *inst, RuntimeInterface::EntrypointId eid,
                                                   llvm::IntegerType *bitcastType)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return false;
    }
    auto val = GetInputValue(inst, 1U);
    auto bitcast = builder_.CreateBitCast(val, bitcastType);

    auto ref = GetInputValue(inst, 0);
    auto beginPos = GetInputValue(inst, 2U);
    auto endPos = GetInputValue(inst, 3U);
    ArenaVector<llvm::Value *> args({ref, bitcast, beginPos, endPos}, GetGraph()->GetLocalAllocator()->Adapter());
    CreateFastPathCall(inst, eid, args);
    return true;
}

bool LLVMIrConstructor::EmitReadString(Inst *inst)
{
    auto entryId = RuntimeInterface::EntrypointId::CREATE_STRING_FROM_MEM;
    auto buf = GetInputValue(inst, 0);
    auto len = GetInputValue(inst, 1);
    auto klassOffset = GetGraph()->GetRuntime()->GetStringClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, klassOffset, builder_.getPtrTy());

    auto result = CreateFastPathCall(inst, entryId, {buf, len, klass});

    MarkAsAllocation(result);
    ValueMapAdd(inst, result);

    return true;
}

bool LLVMIrConstructor::EmitWriteString(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::WRITE_STRING_TO_MEM, 2U);
}
