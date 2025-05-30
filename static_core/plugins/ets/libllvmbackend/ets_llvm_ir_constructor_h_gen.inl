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

bool EmitArrayCopyTo(Inst *inst);
bool EmitStdStringSubstring(Inst *inst);
bool EmitStringBuilderAppendBool(Inst *inst);
bool EmitStringBuilderAppendChar(Inst *inst);
bool EmitStringBuilderAppendByte(Inst *inst);
bool EmitStringBuilderAppendShort(Inst *inst);
bool EmitStringBuilderAppendInt(Inst *inst);
bool EmitStringBuilderAppendLong(Inst *inst);
bool EmitStringBuilderAppendString(Inst *inst);
bool EmitStringBuilderAppendStrings(Inst *inst);
bool EmitStringBuilderToString(Inst *inst);

llvm::Value *CreateStringBuilderAppendLong(Inst *inst);
llvm::Value *CreateStringBuilderAppendString(Inst *inst);
llvm::Value *CreateStringBuilderAppendStrings(Inst *inst);

void StringBuilderAppendStringNull(Inst *inst, llvm::PHINode *result, llvm::BasicBlock *contBb);
void StringBuilderAppendStringMain(Inst *inst, llvm::Value *sbIndexOffset, llvm::Value *sbIndex, llvm::Value *sbBuffer,
                                   llvm::BasicBlock *contBb);
bool EmitDoubleToStringDecimal(Inst *inst);
bool EmitStringTrim(Inst *inst);
bool EmitStringTrimLeft(Inst *inst);
bool EmitStringTrimRight(Inst *inst);
bool EmitCharIsWhiteSpace(Inst *inst);
bool EmitStringStartsWith(Inst *inst);
bool EmitStringEndsWith(Inst *inst);
bool EmitStringGetBytesTlab(Inst *inst);
bool EmitStringIndexOf(Inst *inst);
bool EmitStringIndexOfAfter(Inst *inst);
bool EmitStringFromCharCode(Inst *inst);
bool EmitStringRepeat(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_REPEAT, 2U);
}
bool EmitInt8ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT8_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitInt16ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT16_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitInt32ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::INT32_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitBigInt64ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::BIG_INT64_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitFloat32ArrayFillInternal(Inst *inst);
bool EmitFloat64ArrayFillInternal(Inst *inst);
bool EmitFloatArrayFillInternal(Inst *inst, RuntimeInterface::EntrypointId eid, llvm::IntegerType *bitcastType);
bool EmitUInt8ClampedArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT8_CLAMPED_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitUInt8ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT8_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitUInt16ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT16_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitUInt32ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::U_INT32_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}
bool EmitBigUInt64ArrayFillInternal(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::BIG_U_INT64_ARRAY_FILL_INTERNAL_FAST_PATH, 4U);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define EMIT_TYPED_ARRAY_TO_REVERSED(Name, MacroType)                             \
    /* CC-OFFNXT(G.PRE.02) name part */                                           \
    bool Emit##Name##ArrayToReversed(Inst *inst)                                  \
    {                                                                             \
        auto eid = RuntimeInterface::EntrypointId::MacroType##_ARRAY_TO_REVERSED; \
        /* CC-OFFNXT(G.PRE.05) function gen */                                    \
        return EmitFastPath(inst, eid, 1U);                                       \
    }

EMIT_TYPED_ARRAY_TO_REVERSED(Int8, INT8)
EMIT_TYPED_ARRAY_TO_REVERSED(Int16, INT16)
EMIT_TYPED_ARRAY_TO_REVERSED(Int32, INT32)
EMIT_TYPED_ARRAY_TO_REVERSED(BigInt64, BIG_INT64)
EMIT_TYPED_ARRAY_TO_REVERSED(Float32, FLOAT32)
EMIT_TYPED_ARRAY_TO_REVERSED(Float64, FLOAT64)
EMIT_TYPED_ARRAY_TO_REVERSED(Uint8, UINT8)
EMIT_TYPED_ARRAY_TO_REVERSED(Uint16, UINT16)
EMIT_TYPED_ARRAY_TO_REVERSED(Uint32, UINT32)
EMIT_TYPED_ARRAY_TO_REVERSED(BigUint64, BIG_UINT64)

#undef EMIT_TYPED_ARRAY_TO_REVERSED

bool EmitReadString(Inst *inst);
bool EmitWriteString(Inst *inst);
