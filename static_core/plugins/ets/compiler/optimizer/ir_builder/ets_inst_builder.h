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

#ifndef PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H
#define PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H

template <Opcode OPCODE>
void BuildLaunch(const BytecodeInstruction *bc_inst, bool is_range, bool acc_read);

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/compiler/optimizer/ir_builder/js_interop/js_interop_inst_builder.h"
#endif

void BuildLdObjByName(const BytecodeInstruction *bc_inst, DataType::Type type);
void BuildStObjByName(const BytecodeInstruction *bc_inst, DataType::Type type);
#endif  // PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H
