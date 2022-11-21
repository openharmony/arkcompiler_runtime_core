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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H_
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H_

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "compiler_options.h"

namespace panda::compiler {
class MemoryCoalescing : public Optimization {
public:
    bool RunImpl() override;

    bool IsEnable() const override
    {
        return options.IsCompilerMemoryCoalescing();
    }

    const char *GetPassName() const override
    {
        return "MemoryCoalescing";
    }

    /**
     * Types of memory accesses that can be coalesced
     */
    static bool AcceptedType(DataType::Type type)
    {
        switch (type) {
            case DataType::UINT32:
            case DataType::INT32:
            case DataType::UINT64:
            case DataType::INT64:
            case DataType::FLOAT32:
            case DataType::FLOAT64:
                return true;
            case DataType::REFERENCE:
                return options.IsCompilerMemoryCoalescingObjects();
            default:
                return false;
        }
    }

    NO_MOVE_SEMANTIC(MemoryCoalescing);
    NO_COPY_SEMANTIC(MemoryCoalescing);
    ~MemoryCoalescing() override = default;
};
}  // namespace panda::compiler

#endif  //  COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H_
