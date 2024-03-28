/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#endif  // PANDA_PLUGINS_ETS_COMPILER_INTRINSICS_IR_BUILD_ETS_INL_H
