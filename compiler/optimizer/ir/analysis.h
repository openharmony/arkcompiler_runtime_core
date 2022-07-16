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

#ifndef COMPILER_OPTIMIZER_IR_ANALYSIS_H_
#define COMPILER_OPTIMIZER_IR_ANALYSIS_H_

#include "graph.h"

namespace panda::compiler {

/**
 * The file contains small analysis functions which can be used in different passes
 */
class Inst;
// returns Store value, for StoreArrayPair and StoreArrayPairI saved not last store value in second_value
Inst *InstStoredValue(Inst *inst, Inst **second_value);
Inst *InstStoredValue(Inst *inst);
bool HasOsrEntryBetween(Inst *dominate_inst, Inst *inst);
bool IsSuitableForImplicitNullCheck(const Inst *inst);
bool IsInstNotNull(const Inst *inst);
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_ANALYSIS_H_
