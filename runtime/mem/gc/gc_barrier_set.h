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
#ifndef PANDA_GC_BARRIER_H
#define PANDA_GC_BARRIER_H

#include "libpandabase/mem/gc_barrier.h"
#include "libpandabase/mem/ringbuf/lock_free_ring_buffer.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/mem/gc/card_table.h"

namespace panda::mem {

/**
 * Base barrier set
 */
class GCBarrierSet {
public:
    GCBarrierSet() = delete;
    GCBarrierSet(mem::InternalAllocatorPtr allocator, BarrierType pre_type, BarrierType post_type)
        : pre_type_(pre_type),
          post_type_(post_type),
          pre_operands_(allocator->Adapter()),
          post_operands_(allocator->Adapter())
    {
    }

    NO_COPY_SEMANTIC(GCBarrierSet);
    NO_MOVE_SEMANTIC(GCBarrierSet);
    virtual ~GCBarrierSet() = 0;

    BarrierType GetPreType() const
    {
        ASSERT(IsPreBarrier(pre_type_));
        return pre_type_;
    }

    BarrierType GetPostType() const
    {
        ASSERT(IsPostBarrier(post_type_));
        return post_type_;
    }

    virtual bool IsPreBarrierEnabled()
    {
        return !mem::IsEmptyBarrier(pre_type_);
    }

    /**
     * Pre barrier. Used by interpreter.
     * @param pre_val_addr - reference currently(before store/load happened) stored in the field
     */
    virtual void PreBarrier(void *pre_val_addr) = 0;

    /**
     * Post barrier. Used by interpeter.
     * @param obj_addr - address of field where we store
     * @param val_addr - reference stored into or loaded from the field
     */
    virtual void PostBarrier(const void *obj_addr, void *val_addr) = 0;

    /**
     * Post barrier for array write. Used by interpeter.
     * @param obj_addr - address of the array object
     * @param size - size of the array object
     */
    virtual void PostBarrierArrayWrite(const void *obj_addr, size_t size) = 0;

    /**
     * Post barrier for writing in every field of an object. Used by interpeter.
     * @param object_addr - address of the object
     * @param size - size of the object
     */
    virtual void PostBarrierEveryObjectFieldWrite(const void *obj_addr, size_t size) = 0;

    /**
     * Get barrier operand (literal, function pointer, address etc. See enum BarrierType for details.
     * Should be used for barrier generation in Compiler.
     * @param name - string with name of operand
     * @return barrier operand (value is address or literal)
     */
    BarrierOperand GetBarrierOperand(BarrierPosition barrier_position, std::string_view name);

    BarrierOperand GetPostBarrierOperand(std::string_view name);

protected:
    /**
     * Add barrier operand if there are no operands with this name
     * @param barrier_position - pre or post position of barrier with added operand
     * @param name - name of operand
     * @param barrier_operand - operand
     */
    void AddBarrierOperand(BarrierPosition barrier_position, std::string_view name,
                           const BarrierOperand &barrier_operand)
    {
        if (barrier_position == BarrierPosition::BARRIER_POSITION_PRE) {
            ASSERT(pre_operands_.find(name) == pre_operands_.end());
            pre_operands_.insert({name.data(), barrier_operand});
        } else {
            ASSERT(barrier_position == BarrierPosition::BARRIER_POSITION_POST);
            ASSERT(post_operands_.find(name) == post_operands_.end());
            post_operands_.insert({name.data(), barrier_operand});
        }
    }

private:
    BarrierType pre_type_;   // Type of PRE barrier.
    BarrierType post_type_;  // Type of POST barrier.
    PandaMap<PandaString, BarrierOperand> pre_operands_;
    PandaMap<PandaString, BarrierOperand> post_operands_;
};

/**
 * BarrierSet with barriers do nothing
 */
class GCDummyBarrierSet : public GCBarrierSet {
public:
    explicit GCDummyBarrierSet(mem::InternalAllocatorPtr allocator)
        : GCBarrierSet(allocator, BarrierType::PRE_WRB_NONE, BarrierType::POST_WRB_NONE)
    {
    }

    NO_COPY_SEMANTIC(GCDummyBarrierSet);
    NO_MOVE_SEMANTIC(GCDummyBarrierSet);
    ~GCDummyBarrierSet() override = default;

    void PreBarrier([[maybe_unused]] void *pre_val_addr) override {}

    void PostBarrier([[maybe_unused]] const void *obj_addr, [[maybe_unused]] void *stored_val_addr) override {}

    void PostBarrierArrayWrite([[maybe_unused]] const void *obj_addr, [[maybe_unused]] size_t size) override {}

    void PostBarrierEveryObjectFieldWrite([[maybe_unused]] const void *obj_addr, [[maybe_unused]] size_t size) override
    {
    }
};

class GCGenBarrierSet : public GCBarrierSet {
public:
    GCGenBarrierSet(mem::InternalAllocatorPtr allocator,
                    /* POST ARGS: */
                    void *min_addr, CardTable *card_table, uint8_t card_bits, uint8_t dirty_card_value)
        : GCBarrierSet(allocator, BarrierType::PRE_WRB_NONE, BarrierType::POST_INTERGENERATIONAL_BARRIER),
          min_addr_(min_addr),
          card_table_addr_(reinterpret_cast<uint8_t *>(*card_table->begin())),
          card_bits_(card_bits),
          dirty_card_value_(dirty_card_value),
          card_table_(card_table)
    {
        // POST
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "MIN_ADDR",
                          BarrierOperand(BarrierOperandType::ADDRESS, BarrierOperandValue(min_addr)));
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "CARD_TABLE_ADDR",
                          BarrierOperand(BarrierOperandType::UINT8_ADDRESS, BarrierOperandValue(card_table_addr_)));
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "CARD_BITS",
                          BarrierOperand(BarrierOperandType::UINT8_LITERAL, BarrierOperandValue(card_bits)));
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "DIRTY_VAL",
                          BarrierOperand(BarrierOperandType::UINT8_LITERAL, BarrierOperandValue(dirty_card_value)));
    }

    void PreBarrier(void *pre_val_addr) override;

    void PostBarrier(const void *obj_addr, void *stored_val_addr) override;

    void PostBarrierArrayWrite(const void *obj_addr, size_t size) override;

    void PostBarrierEveryObjectFieldWrite(const void *obj_addr, size_t size) override;

    ~GCGenBarrierSet() override = default;

    NO_COPY_SEMANTIC(GCGenBarrierSet);
    NO_MOVE_SEMANTIC(GCGenBarrierSet);

private:
    // Store operands explicitly for interpreter perf
    // POST BARRIER
    void *min_addr_ {nullptr};            //! Minimal address used by VM. Used as a base for card index calculation
    uint8_t *card_table_addr_ {nullptr};  //! Address of card table
    uint8_t card_bits_ {0};               //! how many bits encoded by card (i.e. size covered by card = 2^card_bits_)
    uint8_t dirty_card_value_ {0};        //! value of dirty card
    FIELD_UNUSED CardTable *card_table_ {nullptr};
};

class GCG1BarrierSet : public GCBarrierSet {
public:
    using ThreadLocalCardQueues = PandaVector<CardTable::CardPtr>;
    static constexpr size_t G1_POST_BARRIER_RING_BUFFER_SIZE = 1024 * 8;
    using G1PostBarrierRingBufferType = mem::LockFreeBuffer<mem::CardTable::CardPtr, G1_POST_BARRIER_RING_BUFFER_SIZE>;

    GCG1BarrierSet(mem::InternalAllocatorPtr allocator,
                   // PRE ARGS:
                   std::atomic<bool> *concurrent_marking_flag, objRefProcessFunc pre_store_func,
                   // POST ARGS:
                   objTwoRefProcessFunc post_func, uint8_t region_size_bits_count, CardTable *card_table,
                   ThreadLocalCardQueues *updated_refs_queue, os::memory::Mutex *queue_lock)
        : GCBarrierSet(allocator, BarrierType::PRE_SATB_BARRIER, BarrierType::POST_INTERREGION_BARRIER),
          concurrent_marking_flag_(concurrent_marking_flag),
          pre_store_func_(pre_store_func),
          post_func_(post_func),
          region_size_bits_count_(region_size_bits_count),
          card_table_(card_table),
          min_addr_(ToVoidPtr(card_table->GetMinAddress())),
          updated_refs_queue_(updated_refs_queue),
          queue_lock_(queue_lock)
    {
        ASSERT(pre_store_func_ != nullptr);
        ASSERT(post_func_ != nullptr);
        // PRE
        AddBarrierOperand(
            BarrierPosition::BARRIER_POSITION_PRE, "CONCURRENT_MARKING_ADDR",
            BarrierOperand(BarrierOperandType::BOOL_ADDRESS, BarrierOperandValue(concurrent_marking_flag)));
        AddBarrierOperand(
            BarrierPosition::BARRIER_POSITION_PRE, "STORE_IN_BUFF_TO_MARK_FUNC",
            BarrierOperand(BarrierOperandType::FUNC_WITH_OBJ_REF_ADDRESS, BarrierOperandValue(pre_store_func_)));
        // POST
        AddBarrierOperand(
            BarrierPosition::BARRIER_POSITION_POST, "REGION_SIZE_BITS",
            BarrierOperand(BarrierOperandType::UINT8_LITERAL, BarrierOperandValue(region_size_bits_count_)));
        AddBarrierOperand(
            BarrierPosition::BARRIER_POSITION_POST, "UPDATE_CARD_FUNC",
            BarrierOperand(BarrierOperandType::FUNC_WITH_TWO_OBJ_REF_ADDRESSES, BarrierOperandValue(post_func_)));
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "CARD_TABLE_ADDR",
                          BarrierOperand(BarrierOperandType::UINT8_ADDRESS,
                                         BarrierOperandValue(reinterpret_cast<uint8_t *>(*card_table->begin()))));
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "MIN_ADDR",
                          BarrierOperand(BarrierOperandType::ADDRESS, BarrierOperandValue(min_addr_)));
    }

    bool IsPreBarrierEnabled() override;

    void PreBarrier(void *pre_val_addr) override;

    void PostBarrier(const void *obj_addr, void *stored_val_addr) override;

    void PostBarrierArrayWrite(const void *obj_addr, size_t size) override;

    void PostBarrierEveryObjectFieldWrite(const void *obj_addr, size_t size) override;

    ~GCG1BarrierSet() override = default;

    CardTable *GetCardTable() const
    {
        return card_table_;
    }

    ThreadLocalCardQueues *GetUpdatedRefsQueue() const
    {
        return updated_refs_queue_;
    }

    os::memory::Mutex *GetQueueLock() const
    {
        return queue_lock_;
    }

    NO_COPY_SEMANTIC(GCG1BarrierSet);
    NO_MOVE_SEMANTIC(GCG1BarrierSet);

private:
    using PostFuncT = std::function<void(const void *, const void *)> *;
    // Store operands explicitly for interpreter perf
    // PRE BARRIER
    std::atomic<bool> *concurrent_marking_flag_ {nullptr};
    objRefProcessFunc pre_store_func_ {nullptr};
    // POST BARRIER
    objTwoRefProcessFunc post_func_;      //! function which is called for the post barrier if all conditions
    uint8_t region_size_bits_count_ {0};  //! how much bits needed for the region
    CardTable *card_table_ {nullptr};     //!
    void *min_addr_ {nullptr};            //! Minimal address used by VM. Used as a base for card index calculation
    ThreadLocalCardQueues *updated_refs_queue_;
    os::memory::Mutex *queue_lock_;
};

}  // namespace panda::mem

#endif  // PANDA_GC_BARRIER_H
