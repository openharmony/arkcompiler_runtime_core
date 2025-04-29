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
#ifndef PANDA_PLUGINS_ETS_COMPILER_INTRINSICS_IR_BUILD_ETS_INL_H
#define PANDA_PLUGINS_ETS_COMPILER_INTRINSICS_IR_BUILD_ETS_INL_H

void BuildIsFiniteIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
void BuildStdRuntimeEquals(const BytecodeInstruction *bcInst, bool accRead);
void BuildSignbitIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
void BuildEscompatArrayGetIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
void BuildEscompatArraySetIntrinsic(const BytecodeInstruction *bcInst, bool accRead);
void BuildTypedArraySetIntrinsic(const BytecodeInstruction *bcInst, ark::compiler::DataType::Type type, bool accRead);
void BuildUint8ClampedArraySetIntrinsic(const BytecodeInstruction *bcInst, ark::compiler::DataType::Type type,
                                        bool accRead);
void BuildTypedArraySetIntrinsic(const BytecodeInstruction *bcInst, ark::compiler::Inst *value,
                                 ark::compiler::DataType::Type type, bool accRead);
void BuildTypedArrayGetIntrinsic(const BytecodeInstruction *bcInst, ark::compiler::DataType::Type type, bool accRead);
// CC-OFFNXT(G.NAM.01,G.NAM.03) false positive
std::tuple<ark::compiler::Inst *, ark::compiler::Inst *> BuildTypedArrayLoadDataAndOffset(
    const BytecodeInstruction *bcInst, ark::compiler::DataType::Type type, bool accRead);
// CC-OFFNXT(G.NAM.01,G.NAM.03) false positive
std::tuple<ark::compiler::Inst *, ark::compiler::Inst *> BuildTypedUnsignedArrayLoadDataAndOffset(
    const BytecodeInstruction *bcInst, ark::compiler::DataType::Type type, bool accRead);
void BuildTypedArrayDeoptimizeIfNotArrayBufferBacked(ark::compiler::Inst *typedArrayInst, size_t bcAddr,
                                                     ark::compiler::SaveStateInst *saveState, size_t fieldOffset);
void BuildTypedArrayDeoptimizeIfExternalData(ark::compiler::Inst *dataInst, size_t bcAddr,
                                             ark::compiler::SaveStateInst *saveState);
void BuildTypedArrayDeoptimizeIfOutOfRange(ark::compiler::Inst *posInst, ark::compiler::Inst *lengthInst, size_t bcAddr,
                                           ark::compiler::SaveStateInst *saveState);

#endif  // PANDA_PLUGINS_ETS_COMPILER_INTRINSICS_IR_BUILD_ETS_INL_H
