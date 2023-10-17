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

#include "runtime/include/managed_thread.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "libpandabase/mem/gc_barrier.h"
#include "libpandabase/mem/mem.h"
#include "runtime/include/object_header.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/rem_set.h"
#include "runtime/mem/gc/heap-space-misc/crossing_map.h"
#include <atomic>

namespace panda::mem {

GCBarrierSet::~GCBarrierSet() = default;

bool CheckPostBarrier(CardTable *card_table, const void *obj_addr, bool check_card_table = true)
{
    if constexpr (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        return true;
    }

    // check that obj_addr must be object header
    ASSERT(IsAddressInObjectsHeap(obj_addr));
    [[maybe_unused]] auto *object = reinterpret_cast<const ObjectHeader *>(obj_addr);
    ASSERT(IsAddressInObjectsHeap(object->ClassAddr<BaseClass>()));

    // we need to check that card related by object header must be young/marked/processed.
    // doesn't for G1, because card_table is processed concurrently, so it can be cleared before we enter here
    bool res = true;
    if (check_card_table) {
        res = !card_table->GetCardPtr(ToUintPtr(obj_addr))->IsClear();
    }
    return res;
}

static void PreSATBBarrier(ObjectHeader *pre_val)
{
    if (pre_val != nullptr) {
        LOG(DEBUG, GC) << "GC PreSATBBarrier pre val -> new val:" << pre_val;
        auto pre_buff = static_cast<PandaVector<ObjectHeader *> *>(ManagedThread::GetCurrent()->GetPreBuff());
        ASSERT(pre_buff != nullptr);
        ValidateObject(RootType::SATB_BUFFER, pre_val);
        pre_buff->push_back(pre_val);
    }
}

void PostIntergenerationalBarrier(const void *min_addr, uint8_t *card_table_addr, uint8_t card_bits,
                                  uint8_t dirty_card_value, const void *obj_field_addr)
{
    size_t card_index = (ToUintPtr(obj_field_addr) - ToUintPtr(min_addr)) >> card_bits;
    auto *card_addr = static_cast<std::atomic_uint8_t *>(ToVoidPtr(ToUintPtr(card_table_addr) + card_index));
    // Atomic with relaxed order reason: data race with card_addr with no synchronization or ordering constraints
    // imposed on other reads or writes
    card_addr->store(dirty_card_value, std::memory_order_relaxed);
}

BarrierOperand GCBarrierSet::GetBarrierOperand(BarrierPosition barrier_position, std::string_view name)
{
    if (barrier_position == BarrierPosition::BARRIER_POSITION_PRE) {
        if (UNLIKELY(pre_operands_.find(name.data()) == pre_operands_.end())) {
            LOG(FATAL, GC) << "Operand " << name << " not found for pre barrier";
        }
        return pre_operands_.at(name.data());
    }
    if (UNLIKELY(post_operands_.find(name.data()) == post_operands_.end())) {
        LOG(FATAL, GC) << "Operand " << name << " not found for post barrier";
    }
    return post_operands_.at(name.data());
}

BarrierOperand GCBarrierSet::GetPostBarrierOperand(std::string_view name)
{
    return GetBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, name);
}

void GCGenBarrierSet::PreBarrier([[maybe_unused]] void *pre_val_addr) {}

void GCGenBarrierSet::PostBarrier(const void *obj_addr, [[maybe_unused]] size_t offset,
                                  [[maybe_unused]] void *stored_val_addr)
{
    LOG(DEBUG, GC) << "GC PostBarrier: write to " << std::hex << obj_addr << " value " << stored_val_addr;
    PostIntergenerationalBarrier(min_addr_, card_table_addr_, card_bits_, dirty_card_value_, obj_addr);
    ASSERT(CheckPostBarrier(card_table_, obj_addr));
}

void GCGenBarrierSet::PostBarrier(const void *obj_addr, [[maybe_unused]] size_t offset, [[maybe_unused]] size_t count)
{
    // NOTE: We can improve an implementation here
    // because now we consider every field as an object reference field.
    // Maybe, it will be better to check it, but there can be possible performance degradation.
    PostIntergenerationalBarrier(min_addr_, card_table_addr_, card_bits_, dirty_card_value_, obj_addr);
    ASSERT(CheckPostBarrier(card_table_, obj_addr));
}

bool GCG1BarrierSet::IsPreBarrierEnabled()
{
    // No data race because G1GC sets this flag on pause
    return Thread::GetCurrent()->GetPreWrbEntrypoint() != nullptr;
}

void GCG1BarrierSet::PreBarrier(void *pre_val_addr)
{
    LOG_IF(pre_val_addr != nullptr, DEBUG, GC) << "GC PreBarrier: with pre-value " << pre_val_addr;
    ASSERT(Thread::GetCurrent()->GetPreWrbEntrypoint() != nullptr);

    PreSATBBarrier(reinterpret_cast<ObjectHeader *>(pre_val_addr));
}

void GCG1BarrierSet::PostBarrier(const void *obj_addr, size_t offset, void *stored_val_addr)
{
    LOG_IF(stored_val_addr != nullptr, DEBUG, GC)
        << "GC PostBarrier: write to " << std::hex << obj_addr << " value " << stored_val_addr;
    if (stored_val_addr != nullptr) {
        // If it is cross-region reference
        auto *card = card_table_->GetCardPtr(ToUintPtr(obj_addr) + offset);
        if (!card->IsYoung() && !card->IsMarked() &&
            !panda::mem::IsSameRegion(obj_addr, stored_val_addr, region_size_bits_count_)) {
            LOG(DEBUG, GC) << "GC Interregion barrier write to " << obj_addr << " value " << stored_val_addr;
            card->Mark();
            Enqueue(card);
        }
    }
    ASSERT(stored_val_addr == nullptr || panda::mem::IsSameRegion(obj_addr, stored_val_addr, region_size_bits_count_) ||
           CheckPostBarrier(card_table_, obj_addr, false));
}

void GCG1BarrierSet::PostBarrier(const void *obj_addr, size_t offset, size_t count)
{
    // Force post inter-region barrier
    auto first_addr = ToUintPtr(obj_addr) + offset;
    auto last_addr = first_addr + count - 1;
    Invalidate(first_addr, last_addr);
    ASSERT(CheckPostBarrier(card_table_, obj_addr, false));
    // NOTE: We can improve an implementation here
    // because now we consider every field as an object reference field.
    // Maybe, it will be better to check it, but there can be possible performance degradation.
}

void GCG1BarrierSet::Invalidate(uintptr_t begin, uintptr_t last)
{
    LOG(DEBUG, GC) << "GC Interregion barrier write for memory range from  " << ToVoidPtr(begin) << " to "
                   << ToVoidPtr(last);
    auto *begin_card = card_table_->GetCardPtr(begin);
    auto *last_card = card_table_->GetCardPtr(last);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (auto *card = begin_card; card <= last_card; ++card) {
        if (!card->IsYoung() && !card->IsMarked()) {
            card->Mark();
            Enqueue(card);
        }
    }
}

void GCG1BarrierSet::Enqueue(CardTable::CardPtr card)
{
    auto *thread = ManagedThread::GetCurrent();
    if (thread == nullptr) {  // slow path via shared-queue for VM threads: gc/compiler/etc
        os::memory::LockHolder lock(*queue_lock_);
        updated_refs_queue_->push_back(card);
    } else {
        // general fast-path for mutators
        ASSERT(thread->GetPreBuff() != nullptr);  // write barrier cant be called after Terminate
        auto *buffer = thread->GetG1PostBarrierBuffer();
        ASSERT(buffer != nullptr);
        // try to push it twice
        for (size_t i = 0; i < 2; i++) {
            bool success = buffer->TryPush(card);
            if (success) {
                return;
            }
        }
        // After 2 unsuccessfull pushing, we see that current buffer still full
        // so, reuse shared buffer
        os::memory::LockHolder lock(*queue_lock_);
        updated_refs_queue_->push_back(card);
    }
}
}  // namespace panda::mem
