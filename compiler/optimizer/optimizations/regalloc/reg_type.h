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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_TYPE_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_TYPE_H

#include "compiler/optimizer/ir/graph.h"

namespace panda::compiler {

inline DataType::Type ConvertRegType(const Graph *graph, DataType::Type type)
{
    if (DataType::IsFloatType(type)) {
        return DataType::Type::UINT64;
    }

    ASSERT(GetCommonType(type) == DataType::INT64 || type == DataType::REFERENCE || type == DataType::POINTER ||
           type == DataType::ANY);
    if (type == DataType::REFERENCE) {
        return type;
    }

    if (DataType::Is32Bits(type, graph->GetArch())) {
        return DataType::Type::UINT32;
    }

    return DataType::Type::UINT64;
}

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_TYPE_H
