/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_CODEGEN_CODEGEN_LOAD_ENTRYPOINT_H
#define COMPILER_OPTIMIZER_CODEGEN_CODEGEN_LOAD_ENTRYPOINT_H

#include "optimizer/code_generator/encode.h"
#include "optimizer/code_generator/operands.h"

namespace ark::compiler {

inline void LoadEntrypoint(Reg entry, Encoder *enc, MemRef epTable, intptr_t epOffset)
{
    enc->EncodeLdr(entry, false, epTable);
    enc->EncodeLdr(entry, false, MemRef(entry, epOffset));
}

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_CODEGEN_LOAD_ENTRYPOINT_H
