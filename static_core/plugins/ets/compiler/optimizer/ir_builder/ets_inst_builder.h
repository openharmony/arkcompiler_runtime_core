/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H
#define PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H

template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ>
void BuildLaunch(const BytecodeInstruction *bcInst);

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/compiler/optimizer/ir_builder/js_interop/js_interop_inst_builder.h"
#endif

template <bool IS_ABC_KIT = false>
void BuildLdObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type);
template <bool IS_ABC_KIT = false>
IntrinsicInst *CreateStObjByNameIntrinsic(size_t pc, compiler::DataType::Type type);
template <bool IS_ABC_KIT = false>
void BuildStObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type);
template <bool IS_RANGE>
void BuildCallByName(const BytecodeInstruction *bcInst);
virtual void BuildIsNullValue(const BytecodeInstruction *bcInst);
template <bool IS_STRICT = false>
void BuildEquals(const BytecodeInstruction *bcInst);
void BuildTypeof(const BytecodeInstruction *bcInst);
void BuildIstrue(const BytecodeInstruction *bcInst);

#endif  // PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H
