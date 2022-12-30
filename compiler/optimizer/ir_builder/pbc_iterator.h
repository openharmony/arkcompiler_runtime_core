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

#ifndef COMPILER_OPTIMIZER_IR_BUILDER_PBC_ITERATOR_H
#define COMPILER_OPTIMIZER_IR_BUILDER_PBC_ITERATOR_H

#include "bytecode_instruction.h"
#include "bytecode_instruction-inl.h"

namespace panda::compiler {
struct BytecodeIterator {
    explicit BytecodeIterator(BytecodeInstruction inst) : inst_(inst) {}
    explicit BytecodeIterator(const uint8_t *data) : inst_(data) {}

    BytecodeIterator &operator++()
    {
        inst_ = inst_.GetNext();
        return *this;
    }

    BytecodeInstruction operator*()
    {
        return inst_;
    }
    bool operator!=(const BytecodeIterator &rhs)
    {
        return inst_.GetAddress() != rhs.inst_.GetAddress();
    }

private:
    BytecodeInstruction inst_;
};

struct BytecodeInstructions {
    BytecodeInstructions(const uint8_t *data, size_t size) : data_(data), size_(size) {}

    // NOLINTNEXTLINE(readability-identifier-naming)
    BytecodeIterator begin() const
    {
        return BytecodeIterator(data_);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    BytecodeIterator end() const
    {
        return BytecodeIterator(data_ + size_);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    size_t GetPc(const BytecodeInstruction INST) const
    {
        return INST.GetAddress() - data_;
    }
    size_t GetSize() const
    {
        return size_;
    }

private:
    const uint8_t *data_;
    size_t size_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_BUILDER_PBC_ITERATOR_H
