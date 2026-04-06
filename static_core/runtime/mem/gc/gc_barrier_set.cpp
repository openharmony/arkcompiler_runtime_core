/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <atomic>
#include "libarkbase/mem/gc_barrier.h"
#include "libarkbase/mem/mem.h"
#include "runtime/arch/memory_helpers.h"
#include "runtime/include/managed_thread.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/rem_set.h"
#include "runtime/mem/gc/heap-space-misc/crossing_map.h"
#include "runtime/mem/object_helpers-inl.h"

namespace ark::mem {

GCBarrierSet::~GCBarrierSet() = default;

bool CheckPostBarrier(CardTable *cardTable, const void *objAddr, bool checkCardTable = true)
{
    if constexpr (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        return true;
    }

    // check that obj_addr must be object header
    ASSERT(IsAddressInObjectsHeap(objAddr));
    [[maybe_unused]] auto *object = reinterpret_cast<const ObjectHeader *>(objAddr);
    ASSERT(IsAddressInObjectsHeap(object->ClassAddr<BaseClass>()));

    // we need to check that card related by object header must be young/marked/processed.
    // doesn't for G1, because card_table is processed concurrently, so it can be cleared before we enter here
    bool res = true;
    if (checkCardTable) {
        res = !cardTable->IsClear(ToUintPtr(objAddr));
    }
    return res;
}

BarrierOperand GCBarrierSet::GetBarrierOperand(BarrierPosition barrierPosition, std::string_view name)
{
    if (barrierPosition == BarrierPosition::BARRIER_POSITION_PRE) {
        if (UNLIKELY(preOperands_.find(name.data()) == preOperands_.end())) {
            LOG(FATAL, GC) << "Operand " << name << " not found for pre barrier";
        }
        return preOperands_.at(name.data());
    }
    if (UNLIKELY(postOperands_.find(name.data()) == postOperands_.end())) {
        LOG(FATAL, GC) << "Operand " << name << " not found for post barrier";
    }
    return postOperands_.at(name.data());
}

BarrierOperand GCBarrierSet::GetPostBarrierOperand(std::string_view name)
{
    return GetBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, name);
}

static inline GCG1BarrierSet *GetG1BarrierSet()
{
    const auto *mutator = Mutator::GetCurrent();
    ASSERT(mutator != nullptr);
    GCBarrierSet *barrierSet = mutator->GetBarrierSet();
    ASSERT(barrierSet != nullptr);
    return static_cast<GCG1BarrierSet *>(barrierSet);
}

extern "C" void PreWrbFuncEntrypoint(ObjectPointerType oldval)
{
    ASSERT(IsAddressInObjectsHeap(oldval));
    ASSERT(IsAddressInObjectsHeap(reinterpret_cast<const ObjectHeader *>(oldval)->ClassAddr<BaseClass>()));
    LOG(DEBUG, GC) << "G1GC pre barrier val = " << oldval;
    ValidateObject(RootType::SATB_BUFFER, reinterpret_cast<const ObjectHeader *>(oldval));
    auto *thread = ManagedThread::GetCurrent();
    // thread can't be null here because pre-barrier is called only in concurrent-mark, but we don't process
    // weak-references in concurrent mark
    ASSERT(thread != nullptr);
    auto bufferVec = thread->GetPreBuff();
    bufferVec->push_back(reinterpret_cast<ObjectHeader *>(oldval));
}

static void PostInterregionBarrier(ObjectPointerType object, size_t offset, ObjectPointerType value,
                                   GCG1BarrierSet *barriers)
{
    ASSERT(reinterpret_cast<void *>(object) != nullptr);
    ASSERT(reinterpret_cast<void *>(value) != nullptr);
    CardTable *cardTable = barriers->GetCardTable();
    ASSERT(cardTable != nullptr);
    // No need to keep remsets for young->young
    // NOTE(dtrubenkov): add assert that we do not have young -> young reference here
    auto *card = cardTable->GetCardPtr(object + offset);
    if (card->IsYoung()) {
        return;
    }
    // StoreLoad barrier is required to guarantee order of previous reference store and card load
    arch::StoreLoadBarrier();

    auto cardValue = card->GetCard();
    auto status = CardTable::Card::GetStatus(cardValue);
    if (!CardTable::Card::IsMarked(status)) {
        LOG(DEBUG, GC) << "GC Interregion barrier write to " << std::hex << object << " value " << value;
        card->Mark();
        if (!CardTable::Card::IsHot(cardValue)) {
            barriers->Enqueue(card);
        }
    }
}

// The declaration for PostWrbUpdateCardFuncEntrypoint is generated from entrypoints.yaml.
extern "C" void PostWrbUpdateCardFuncEntrypoint(ObjectPointerType from, ObjectPointerType to)
{
    GCG1BarrierSet *barriers = GetG1BarrierSet();
    ASSERT(barriers != nullptr);
    PostInterregionBarrier(from, 0, to, barriers);
}

GCG1BarrierSet::GCG1BarrierSet(mem::InternalAllocatorPtr allocator, uint8_t regionSizeBitsCount, CardTable *cardTable,
                               ThreadLocalCardQueues *updatedRefsQueue, os::memory::Mutex *queueLock)
    : GCBarrierSet(allocator, BarrierType::PRE_RB_NONE, BarrierType::PRE_SATB_BARRIER,
                   BarrierType::POST_INTERREGION_BARRIER),
      preStoreFunc_(PreWrbFuncEntrypoint),
      postFunc_(PostWrbUpdateCardFuncEntrypoint),
      regionSizeBitsCount_(regionSizeBitsCount),
      cardTable_(cardTable),
      minAddr_(ToVoidPtr(cardTable->GetMinAddress())),
      updatedRefsQueue_(updatedRefsQueue),
      queueLock_(queueLock)
{
    ASSERT(preStoreFunc_ != nullptr);
    ASSERT(postFunc_ != nullptr);
    // PRE
    AddBarrierOperand(
        BarrierPosition::BARRIER_POSITION_PRE, "STORE_IN_BUFF_TO_MARK_FUNC",
        BarrierOperand(BarrierOperandType::FUNC_WITH_OBJ_REF_ADDRESS, BarrierOperandValue(preStoreFunc_)));
    // POST
    AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "REGION_SIZE_BITS",
                      BarrierOperand(BarrierOperandType::UINT8_LITERAL, BarrierOperandValue(regionSizeBitsCount_)));
    AddBarrierOperand(
        BarrierPosition::BARRIER_POSITION_POST, "UPDATE_CARD_FUNC",
        BarrierOperand(BarrierOperandType::FUNC_WITH_TWO_OBJ_REF_ADDRESSES, BarrierOperandValue(postFunc_)));
    AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "CARD_TABLE_ADDR",
                      BarrierOperand(BarrierOperandType::UINT8_ADDRESS,
                                     BarrierOperandValue(reinterpret_cast<uint8_t *>(*cardTable->begin()))));
    AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "MIN_ADDR",
                      BarrierOperand(BarrierOperandType::ADDRESS, BarrierOperandValue(minAddr_)));
}

bool GCG1BarrierSet::IsPreBarrierEnabled()
{
    // No data race because G1GC sets this flag on pause
    return Mutator::GetCurrent()->GetPreWrbEntrypoint() != nullptr;
}

void GCG1BarrierSet::PreBarrier(void *preValAddr)
{
    LOG_IF(preValAddr != nullptr, DEBUG, GC) << "GC PreBarrier: with pre-value " << preValAddr;
    ASSERT(Mutator::GetCurrent()->GetPreWrbEntrypoint() != nullptr);

    if (preValAddr != nullptr) {
        PreWrbFuncEntrypoint(ToObjPtrType(preValAddr));
    }
}

void GCG1BarrierSet::PostBarrier(const void *objAddr, size_t offset, void *storedValAddr)
{
    if (storedValAddr == nullptr) {
        return;
    }
#if defined(PANDA_ENABLE_DFX_MEMORY_CHECK)
    ValidateObject(reinterpret_cast<const ObjectHeader *>(objAddr),
                   reinterpret_cast<const ObjectHeader *>(storedValAddr));
#endif
    if (ark::mem::IsSameRegion(objAddr, storedValAddr, regionSizeBitsCount_)) {
        return;
    }

    LOG(DEBUG, GC) << "GC Interregion barrier write object to " << objAddr << " value " << storedValAddr;
    PostInterregionBarrier(ToObjPtrType(objAddr), offset, ToObjPtrType(storedValAddr), this);

    ASSERT(CheckPostBarrier(cardTable_, objAddr, false));
}

void GCG1BarrierSet::PostBarrier(const void *objAddr, size_t offset, size_t count)
{
    // Force post inter-region barrier
    auto firstAddr = ToUintPtr(objAddr) + offset;
    auto *beginCard = cardTable_->GetCardPtr(firstAddr);
    auto *lastCard = cardTable_->GetCardPtr(firstAddr + count - 1);
    if (beginCard->IsYoung()) {
        // Check one card only because cards from beginCard to lastCard belong to the same region
        return;
    }
    // StoreLoad barrier is required to guarantee order of previous reference store and card load
    arch::StoreLoadBarrier();
    Invalidate(beginCard, lastCard);
    ASSERT(CheckPostBarrier(cardTable_, objAddr, false));
    // NOTE: We can improve an implementation here
    // because now we consider every field as an object reference field.
    // Maybe, it will be better to check it, but there can be possible performance degradation.
}

void GCG1BarrierSet::Invalidate(CardTable::CardPtr begin, CardTable::CardPtr last)
{
    LOG(DEBUG, GC) << "GC Interregion barrier write for memory range from  " << cardTable_->GetCardStartAddress(begin)
                   << " to " << cardTable_->GetCardEndAddress(last);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (auto *card = begin; card <= last; ++card) {
        auto cardValue = card->GetCard();
        auto cardStatus = CardTable::Card::GetStatus(cardValue);
        ASSERT(!CardTable::Card::IsYoung(cardStatus));
        if (!CardTable::Card::IsMarked(cardStatus)) {
            card->Mark();
            if (!CardTable::Card::IsHot(cardValue)) {
                Enqueue(card);
            }
        }
    }
}

void GCG1BarrierSet::Enqueue(CardTable::CardPtr card)
{
    auto *thread = ManagedThread::GetCurrent();
    if (thread == nullptr) {  // slow path via shared-queue for VM threads: gc/compiler/etc
        os::memory::LockHolder lock(*queueLock_);
        updatedRefsQueue_->push_back(card);
    } else {
        // general fast-path for mutators
        ASSERT(thread->GetPreBuff() != nullptr);  // write barrier cant be called after Terminate
        auto *buffer = thread->GetG1PostBarrierBuffer();
        ASSERT(buffer != nullptr);
        // try to push it twice
        for (size_t i = 0; i < 2U; i++) {
            bool success = buffer->TryPush(card);
            if (success) {
                return;
            }
        }
        // After 2 unsuccessfull pushing, we see that current buffer still full
        // so, reuse shared buffer
        PostCardToQueue(card);
    }
}

void GCG1BarrierSet::PostCardToQueue(CardTable::CardPtr card)
{
    os::memory::LockHolder lock(*queueLock_);
    updatedRefsQueue_->push_back(card);
}

extern "C" void *ReadBarrierFuncEntrypoint(ObjectPointerType *fieldPtr)
{
    if constexpr (GCCMCBarrierSet::USE_READ_BARRIERS) {
        auto field = reinterpret_cast<common_vm::RefField<false> &>(*fieldPtr);
        return common_vm::BaseRuntime::ReadBarrier(field.GetTargetObject(), fieldPtr);
    }
    return nullptr;
}

GCCMCBarrierSet::GCCMCBarrierSet(mem::InternalAllocatorPtr allocator)
    : GCBarrierSet(allocator, BarrierType::CMC_READ_BARRIER, BarrierType::PRE_WRB_NONE,
                   BarrierType::POST_CMC_WRITE_BARRIER)
{
    readBarrierFunc_ = &ReadBarrierFuncEntrypoint;
    ASSERT(readBarrierFunc_ != nullptr);
    AddBarrierOperand(
        BarrierPosition::BARRIER_POSITION_PRE, "CMC_READ_BARRIER",
        BarrierOperand(BarrierOperandType::FUNC_WITH_OBJ_FIELD_ADDRESS, BarrierOperandValue(readBarrierFunc_)));
}

void GCCMCBarrierSet::PostBarrier([[maybe_unused]] const void *objAddr, [[maybe_unused]] size_t offset,
                                  [[maybe_unused]] void *storedValAddr)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    if (storedValAddr == nullptr) {
        return;
    }
#if defined(PANDA_ENABLE_DFX_MEMORY_CHECK)
    ValidateObject(reinterpret_cast<const ObjectHeader *>(objAddr), reinterpret_cast<ObjectHeader *>(storedValAddr));
#endif
    common_vm::BaseRuntime::WriteBarrier(const_cast<void *>(objAddr), ToVoidPtr(ToUintPtr(objAddr) + offset),
                                         storedValAddr, Mutator::GetCurrent());
#endif  // ARK_USE_COMMON_RUNTIME
}

void GCCMCBarrierSet::PostBarrier([[maybe_unused]] const void *objAddr, [[maybe_unused]] size_t offset,
                                  [[maybe_unused]] size_t count)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    auto *begin = reinterpret_cast<ObjectPointerType *>(ToUintPtr(objAddr) + offset);
    auto *end = reinterpret_cast<ObjectPointerType *>(ToUintPtr(begin) + count);
    while (begin < end) {
        auto value = *begin;
        if (value != 0) {
#if defined(PANDA_ENABLE_DFX_MEMORY_CHECK)
            ValidateObject(reinterpret_cast<const ObjectHeader *>(objAddr),
                           reinterpret_cast<ObjectHeader *>(ToVoidPtr(value)));
#endif
            common_vm::BaseRuntime::WriteBarrier(const_cast<void *>(objAddr), static_cast<void *>(begin),
                                                 ToVoidPtr(value), Mutator::GetCurrent());
        }
        ++begin;
    }
#endif  // ARK_USE_COMMON_RUNTIME
}

bool GCCMCBarrierSet::IsReadBarrierEnabled()
{
    return ManagedThread::GetCurrent()->GetReadBarrierEntrypoint() != nullptr;
}

void *GCCMCBarrierSet::ReadBarrier([[maybe_unused]] const void *objAddr, [[maybe_unused]] size_t offset)
{
    if constexpr (USE_READ_BARRIERS) {
        return common_vm::BaseRuntime::ReadBarrier(const_cast<void *>(objAddr), ToVoidPtr(ToUintPtr(objAddr) + offset));
    }
    return nullptr;
}

void *GCCMCBarrierSet::ReadBarrier(void **refAddr)
{
    if constexpr (USE_READ_BARRIERS) {
        auto *readBarrier = ManagedThread::GetCurrent()->GetReadBarrierEntrypoint();
        if (readBarrier != nullptr) {
            reinterpret_cast<ObjFieldProcessFunc>(readBarrier)(reinterpret_cast<ObjectPointerType *>(refAddr));
            auto field = reinterpret_cast<common_vm::RefField<false> &>(*refAddr);
            return field.GetTargetObject();
        }
    }
    auto field = reinterpret_cast<common_vm::RefField<false> &>(*refAddr);
    return field.GetTargetObject();
}

}  // namespace ark::mem
