/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "runtime/mem/internal_arena_allocator.h"

namespace ark::mem {

size_t InternalArenaAllocator::GetAllocatedSize()
{
    size_t size = 0;
    for (Arena *cur = arenas_; cur != nullptr; cur = cur->GetNextArena()) {
        size += cur->GetOccupiedSize();
    }
    return size;
}

void InternalArenaAllocator::Resize(size_t newSize)
{
    size_t curSize = GetAllocatedSize();
    if (curSize <= newSize) {
        return;
    }

    size_t bytesToDelete = curSize - newSize;

    // Try to delete unused arenas
    while ((arenas_ != nullptr) && (bytesToDelete != 0)) {
        Arena *next = arenas_->GetNextArena();
        size_t curArenaSize = arenas_->GetOccupiedSize();
        if (curArenaSize < bytesToDelete) {
            // We need to free the whole arena
            arenas_->~Arena();
            allocator_->Free(arenas_);
            arenas_ = next;
            bytesToDelete -= curArenaSize;
        } else {
            arenas_->Resize(curArenaSize - bytesToDelete);
            bytesToDelete = 0;
        }
    }
    ASSERT(bytesToDelete == 0);
}

InternalArenaAllocator *InternalArenaPool::Get(mem::InternalAllocatorPtr allocator)
{
    ASSERT(allocator != nullptr);
    InternalArenaAllocator *ret = pool_.exchange(nullptr);
    if (ret != nullptr) {
        return ret;
    }
    return allocator->New<InternalArenaAllocator>(allocator);
}

void InternalArenaPool::SetArenaPool(InternalArenaAllocator *allocator)
{
    // Allow setting to null pointer (maybe allocator == nullptr)
    InternalArenaAllocator *old = pool_.exchange(allocator);
    // Delete the old allocator if it exists
    if (old != nullptr) {
        InternalAllocatorPtr internalAllocator = old->GetInternalAllocator();
        old->~InternalArenaAllocator();
        internalAllocator->Free(old);
    }
}

void InternalArenaPool::Release(InternalArenaAllocator *allocator)
{
    ASSERT(allocator != nullptr);
    allocator->Resize(0);
    SetArenaPool(allocator);
}

void InternalArenaPool::Clear()
{
    SetArenaPool(nullptr);
}

}  // namespace ark::mem
