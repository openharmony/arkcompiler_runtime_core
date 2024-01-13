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

template <RuntimeInterface::IntrinsicId ID, DataType::Type RET_TYPE, DataType::Type... PARAM_TYPES>
friend struct IntrinsicBuilder;

template <size_t N>
IntrinsicInst *BuildInteropIntrinsic(size_t pc, RuntimeInterface::IntrinsicId id, DataType::Type retType,
                                     const std::array<DataType::Type, N> &types,
                                     const std::array<Inst *, N + 1> &inputs);
std::pair<Inst *, Inst *> BuildResolveInteropCallIntrinsic(RuntimeInterface::InteropCallKind callKind, size_t pc,
                                                           RuntimeInterface::MethodPtr method, Inst *arg0, Inst *arg1,
                                                           SaveStateInst *saveState);
void BuildReturnValueConvertInteropIntrinsic(RuntimeInterface::InteropCallKind callKind, size_t pc,
                                             RuntimeInterface::MethodPtr method, Inst *jsCall,
                                             SaveStateInst *saveState);
void BuildInteropCall(const BytecodeInstruction *bcInst, RuntimeInterface::InteropCallKind callKind,
                      RuntimeInterface::MethodPtr method, bool isRange, bool accRead);
bool TryBuildInteropCall(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
