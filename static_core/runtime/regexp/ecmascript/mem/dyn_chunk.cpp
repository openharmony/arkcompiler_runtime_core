/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dyn_chunk.h"
#include "runtime/include/runtime.h"
#include "securec.h"

namespace panda {
int DynChunk::Expand(size_t new_size)
{
    if (new_size > allocated_size_) {
        if (error_) {
            return FAILURE;
        }
        ASSERT(allocated_size_ <= std::numeric_limits<size_t>::max() / ALLOCATE_MULTIPLIER);
        size_t size = allocated_size_ * ALLOCATE_MULTIPLIER;
        new_size = std::max({size, new_size, ALLOCATE_MIN_SIZE});
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        auto *new_buf = Runtime::GetCurrent()->GetInternalAllocator()->New<uint8_t[]>(new_size);
        if (new_buf == nullptr) {
            error_ = true;
            return FAILURE;
        }
        if (memset_s(new_buf, new_size, 0, new_size) != EOK) {
            error_ = true;
            return FAILURE;
        }
        if (buf_ != nullptr) {
            if (memcpy_s(new_buf, size_, buf_, size_) != EOK) {
                error_ = true;
                return FAILURE;
            }
        }
        Runtime::GetCurrent()->GetInternalAllocator()->DeleteArray(buf_);
        buf_ = new_buf;
        allocated_size_ = new_size;
    }
    return SUCCESS;
}

int DynChunk::Insert(uint32_t position, size_t len)
{
    if (size_ < position) {
        return FAILURE;
    }
    if (Expand(size_ + len) != 0) {
        return FAILURE;
    }
    size_t move_size = size_ - position;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (memmove_s(buf_ + position + len, move_size, buf_ + position, move_size) != EOK) {
        return FAILURE;
    }
    size_ += len;
    return SUCCESS;
}

int DynChunk::Emit(const uint8_t *data, size_t length)
{
    if (UNLIKELY((size_ + length) > allocated_size_)) {
        if (Expand(size_ + length) != 0) {
            return FAILURE;
        }
    }

    if (memcpy_s(buf_ + size_,  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                 length, data, length) != EOK) {
        return FAILURE;
    }
    size_ += length;
    return SUCCESS;
}

int DynChunk::EmitChar(uint8_t c)
{
    return Emit(&c, 1);
}

int DynChunk::EmitSelf(size_t offset, size_t length)
{
    if (UNLIKELY((size_ + length) > allocated_size_)) {
        if (Expand(size_ + length) != 0) {
            return FAILURE;
        }
    }

    if (memcpy_s(buf_ + size_,  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                 length,
                 buf_ + offset,  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                 length) != EOK) {
        return FAILURE;
    }
    size_ += length;
    return SUCCESS;
}

int DynChunk::EmitStr(const char *str)
{
    return Emit(reinterpret_cast<const uint8_t *>(str), strlen(str) + 1);
}
}  // namespace panda
