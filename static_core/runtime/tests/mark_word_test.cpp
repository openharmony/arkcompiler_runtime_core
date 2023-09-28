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

#include <random>

#include "gtest/gtest.h"
#include "runtime/include/managed_thread.h"
#include "runtime/mark_word.cpp"

namespace panda {

class MarkWordTest : public testing::Test {
public:
    MarkWordTest() = default;

protected:
    void SetUp() override
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
    }

    void TearDown() override
    {
        // Logger::Destroy();
    }

    enum MarkWordFieldsMaxValues : MarkWord::MarkWordSize {
        MAX_THREAD_ID = (1UL << MarkWord::MarkWordRepresentation::LIGHT_LOCK_THREADID_SIZE) - 1UL,
        MAX_LOCK_COUNT = (1UL << MarkWord::MarkWordRepresentation::LIGHT_LOCK_LOCK_COUNT_SIZE) - 1UL,
        MAX_MONITOR_ID = (1UL << MarkWord::MarkWordRepresentation::MONITOR_POINTER_SIZE) - 1UL,
        MAX_HASH = (1UL << MarkWord::MarkWordRepresentation::HASH_SIZE) - 1UL,
        MAX_FORWARDING_ADDRESS = std::numeric_limits<MarkWord::MarkWordSize>::max() &
                                 MarkWord::MarkWordRepresentation::FORWARDING_ADDRESS_MASK_IN_PLACE,
    };

    class RandomTestValuesGetter {
    public:
        using MarkWordDistribution = std::uniform_int_distribution<MarkWord::MarkWordSize>;

        // NOLINTNEXTLINE(cert-msc51-cpp)
        RandomTestValuesGetter()
        {
#ifdef PANDA_NIGHTLY_TEST_ON
            seed_ = std::random_device()();
#else
            // NOLINTNEXTLINE(readability-magic-numbers)
            seed_ = 0xC0E67D50;
#endif
            gen_ = std::mt19937(seed_);

            thread_id_range_ = MarkWordDistribution(0, MAX_THREAD_ID);
            lock_count_range_ = MarkWordDistribution(0, MAX_LOCK_COUNT);
            monitor_id_range_ = MarkWordDistribution(0, MAX_MONITOR_ID);
            hash_range_ = MarkWordDistribution(0, MAX_HASH);
            forwarding_address_range_ = MarkWordDistribution(0, MAX_FORWARDING_ADDRESS);
        }

        ManagedThread::ThreadId GetThreadId()
        {
            return thread_id_range_(gen_);
        }

        uint32_t GetLockCount()
        {
            return lock_count_range_(gen_);
        }

        Monitor::MonitorId GetMonitorId()
        {
            return monitor_id_range_(gen_);
        }

        uint32_t GetHash()
        {
            return hash_range_(gen_);
        }

        MarkWord::MarkWordSize GetForwardingAddress()
        {
            return forwarding_address_range_(gen_) & MarkWord::MarkWordRepresentation::FORWARDING_ADDRESS_MASK_IN_PLACE;
        }

        uint32_t GetSeed()
        {
            return seed_;
        }

    private:
        uint32_t seed_;
        std::mt19937 gen_;
        MarkWordDistribution thread_id_range_;
        MarkWordDistribution lock_count_range_;
        MarkWordDistribution monitor_id_range_;
        MarkWordDistribution hash_range_;
        MarkWordDistribution forwarding_address_range_;
    };

    class MaxTestValuesGetter {
    public:
        ManagedThread::ThreadId GetThreadId() const
        {
            return MAX_THREAD_ID;
        }

        uint32_t GetLockCount() const
        {
            return MAX_LOCK_COUNT;
        }

        Monitor::MonitorId GetMonitorId() const
        {
            return static_cast<Monitor::MonitorId>(MAX_MONITOR_ID);
        }

        uint32_t GetHash() const
        {
            return static_cast<uint32_t>(MAX_HASH);
        }

        MarkWord::MarkWordSize GetForwardingAddress() const
        {
            return MAX_FORWARDING_ADDRESS;
        }

        uint32_t GetSeed() const
        {
            // We don't have a seed for this case
            return 0;
        }
    };

    template <class Getter>
    class MarkWordWrapper {
    public:
        explicit MarkWordWrapper(bool is_marked_for_gc = false, bool is_read_barrier_set = false)
        {
            if (is_marked_for_gc) {
                mw_ = mw_.SetMarkedForGC();
            }
            if (is_read_barrier_set) {
                mw_ = mw_.SetReadBarrier();
            }
        };

        void CheckUnlocked(bool is_marked_for_gc = false, bool is_read_barrier_set = false)
        {
            ASSERT_EQ(mw_.GetState(), MarkWord::ObjectState::STATE_UNLOCKED) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.IsMarkedForGC(), is_marked_for_gc) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.IsReadBarrierSet(), is_read_barrier_set) << " seed = " << param_getter_.GetSeed();
        }

        void CheckLightweightLock(const ManagedThread::ThreadId t_id, const uint32_t lock_count, bool is_marked_for_gc,
                                  bool is_read_barrier_set = false)
        {
            ASSERT_EQ(mw_.GetState(), MarkWord::ObjectState::STATE_LIGHT_LOCKED)
                << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.GetThreadId(), t_id) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.GetLockCount(), lock_count) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.IsMarkedForGC(), is_marked_for_gc) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.IsReadBarrierSet(), is_read_barrier_set) << " seed = " << param_getter_.GetSeed();
        }

        void CheckHeavyweightLock(const Monitor::MonitorId m_id, bool is_marked_for_gc,
                                  bool is_read_barrier_set = false)
        {
            ASSERT_EQ(mw_.GetState(), MarkWord::ObjectState::STATE_HEAVY_LOCKED)
                << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.GetMonitorId(), m_id) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.IsMarkedForGC(), is_marked_for_gc) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.IsReadBarrierSet(), is_read_barrier_set) << " seed = " << param_getter_.GetSeed();
        }

        void CheckHashed(uint32_t hash, bool is_marked_for_gc, bool is_read_barrier_set = false)
        {
            if (mw_.CONFIG_IS_HASH_IN_OBJ_HEADER) {
                ASSERT_EQ(mw_.GetState(), MarkWord::ObjectState::STATE_HASHED) << " seed = " << param_getter_.GetSeed();
                ASSERT_EQ(mw_.GetHash(), hash) << " seed = " << param_getter_.GetSeed();
                ASSERT_EQ(mw_.IsMarkedForGC(), is_marked_for_gc) << " seed = " << param_getter_.GetSeed();
                ASSERT_EQ(mw_.IsReadBarrierSet(), is_read_barrier_set) << " seed = " << param_getter_.GetSeed();
            }
        }

        void CheckGC(MarkWord::MarkWordSize forwarding_address)
        {
            ASSERT_EQ(mw_.GetState(), MarkWord::ObjectState::STATE_GC) << " seed = " << param_getter_.GetSeed();
            ASSERT_EQ(mw_.GetForwardingAddress(), forwarding_address) << " seed = " << param_getter_.GetSeed();
        }

        void DecodeLightLock(ManagedThread::ThreadId t_id, uint32_t l_count)
        {
            mw_ = mw_.DecodeFromLightLock(t_id, l_count);
        }

        void DecodeHeavyLock(Monitor::MonitorId m_id)
        {
            mw_ = mw_.DecodeFromMonitor(m_id);
        }

        void DecodeHash(uint32_t hash)
        {
            mw_ = mw_.DecodeFromHash(hash);
        }

        void DecodeForwardingAddress(MarkWord::MarkWordSize f_address)
        {
            mw_ = mw_.DecodeFromForwardingAddress(f_address);
        }

        void DecodeAndCheckLightLock(bool is_marked_for_gc = false, bool is_read_barrier_set = false)
        {
            auto t_id = param_getter_.GetThreadId();
            auto l_count = param_getter_.GetLockCount();
            DecodeLightLock(t_id, l_count);
            CheckLightweightLock(t_id, l_count, is_marked_for_gc, is_read_barrier_set);
        }

        void DecodeAndCheckHeavyLock(bool is_marked_for_gc = false, bool is_read_barrier_set = false)
        {
            auto m_id = param_getter_.GetMonitorId();
            DecodeHeavyLock(m_id);
            CheckHeavyweightLock(m_id, is_marked_for_gc, is_read_barrier_set);
        }

        void DecodeAndCheckHashed(bool is_marked_for_gc = false, bool is_read_barrier_set = false)
        {
            auto hash = param_getter_.GetHash();
            DecodeHash(hash);
            CheckHashed(hash, is_marked_for_gc, is_read_barrier_set);
        }

        void DecodeAndCheckGC()
        {
            auto f_address = param_getter_.GetForwardingAddress();
            DecodeForwardingAddress(f_address);
            CheckGC(f_address);
        }

        void SetMarkedForGC()
        {
            mw_ = mw_.SetMarkedForGC();
        }

        void SetReadBarrier()
        {
            mw_ = mw_.SetReadBarrier();
        }

    private:
        MarkWord mw_;
        Getter param_getter_;
    };

    template <class Getter>
    void CheckMakeHashed(bool is_marked_for_gc, bool is_read_barrier_set);

    template <class Getter>
    void CheckMakeLightweightLock(bool is_marked_for_gc, bool is_read_barrier_set);

    template <class Getter>
    void CheckMakeHeavyweightLock(bool is_marked_for_gc, bool is_read_barrier_set);

    template <class Getter>
    void CheckMakeGC();

    template <class Getter>
    void CheckMarkingWithGC();

    template <class Getter>
    void CheckReadBarrierSet();
};

template <class Getter>
void MarkWordTest::CheckMakeHashed(bool is_marked_for_gc, bool is_read_barrier_set)
{
    // nothing, gc = markedForGC, rb = readBarrierSet, state = unlocked
    MarkWordWrapper<Getter> wrapper(is_marked_for_gc, is_read_barrier_set);

    // check new hash
    wrapper.DecodeAndCheckHashed(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckHashed(is_marked_for_gc, is_read_barrier_set);

    // check after lightweight lock
    wrapper.DecodeAndCheckLightLock(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckHashed(is_marked_for_gc, is_read_barrier_set);

    // check after heavyweight lock
    wrapper.DecodeAndCheckHeavyLock(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckHashed(is_marked_for_gc, is_read_barrier_set);
}

TEST_F(MarkWordTest, CreateHashedWithRandValues)
{
    CheckMakeHashed<RandomTestValuesGetter>(false, false);
    CheckMakeHashed<RandomTestValuesGetter>(false, true);
    CheckMakeHashed<RandomTestValuesGetter>(true, false);
    CheckMakeHashed<RandomTestValuesGetter>(true, true);
}

TEST_F(MarkWordTest, CreateHashedWithMaxValues)
{
    CheckMakeHashed<MaxTestValuesGetter>(false, false);
    CheckMakeHashed<MaxTestValuesGetter>(false, true);
    CheckMakeHashed<MaxTestValuesGetter>(true, false);
    CheckMakeHashed<MaxTestValuesGetter>(true, true);
}

template <class Getter>
void MarkWordTest::CheckMakeLightweightLock(bool is_marked_for_gc, bool is_read_barrier_set)
{
    // nothing, gc = markedForGC, rb = readBarrierSet, state = unlocked
    MarkWordWrapper<Getter> wrapper(is_marked_for_gc, is_read_barrier_set);

    // check new lightweight lock
    wrapper.DecodeAndCheckLightLock(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckLightLock(is_marked_for_gc, is_read_barrier_set);

    // check after hash
    wrapper.DecodeAndCheckHashed(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckLightLock(is_marked_for_gc, is_read_barrier_set);

    // check after heavyweight lock
    wrapper.DecodeAndCheckHeavyLock(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckLightLock(is_marked_for_gc, is_read_barrier_set);
}

TEST_F(MarkWordTest, CreateLightweightLockWithRandValues)
{
    CheckMakeLightweightLock<RandomTestValuesGetter>(false, false);
    CheckMakeLightweightLock<RandomTestValuesGetter>(false, true);
    CheckMakeLightweightLock<RandomTestValuesGetter>(true, false);
    CheckMakeLightweightLock<RandomTestValuesGetter>(true, true);
}

TEST_F(MarkWordTest, CreateLightweightLockWithMaxValues)
{
    CheckMakeLightweightLock<MaxTestValuesGetter>(false, false);
    CheckMakeLightweightLock<MaxTestValuesGetter>(false, true);
    CheckMakeLightweightLock<MaxTestValuesGetter>(true, false);
    CheckMakeLightweightLock<MaxTestValuesGetter>(true, true);
}

template <class Getter>
void MarkWordTest::CheckMakeHeavyweightLock(bool is_marked_for_gc, bool is_read_barrier_set)
{
    // nothing, gc = markedForGC, rb = readBarrierSet, state = unlocked
    MarkWordWrapper<Getter> wrapper(is_marked_for_gc, is_read_barrier_set);

    // check new heavyweight lock
    wrapper.DecodeAndCheckHeavyLock(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckHeavyLock(is_marked_for_gc, is_read_barrier_set);

    // check after hash
    wrapper.DecodeAndCheckHashed(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckHeavyLock(is_marked_for_gc, is_read_barrier_set);

    // check after lightweight lock
    wrapper.DecodeAndCheckLightLock(is_marked_for_gc, is_read_barrier_set);
    wrapper.DecodeAndCheckHeavyLock(is_marked_for_gc, is_read_barrier_set);
}

TEST_F(MarkWordTest, CreateHeavyweightLockWithRandValues)
{
    CheckMakeHeavyweightLock<RandomTestValuesGetter>(false, false);
    CheckMakeHeavyweightLock<RandomTestValuesGetter>(false, true);
    CheckMakeHeavyweightLock<RandomTestValuesGetter>(true, false);
    CheckMakeHeavyweightLock<RandomTestValuesGetter>(true, true);
}

TEST_F(MarkWordTest, CreateHeavyweightLockWithMaxValues)
{
    CheckMakeHeavyweightLock<MaxTestValuesGetter>(false, false);
    CheckMakeHeavyweightLock<MaxTestValuesGetter>(false, true);
    CheckMakeHeavyweightLock<MaxTestValuesGetter>(true, false);
    CheckMakeHeavyweightLock<MaxTestValuesGetter>(true, true);
}

template <class Getter>
void MarkWordTest::CheckMakeGC()
{
    // check new gc
    {
        MarkWordWrapper<Getter> wrapper;
        wrapper.DecodeAndCheckGC();
        wrapper.DecodeAndCheckGC();
    }

    // check after hash
    {
        MarkWordWrapper<Getter> wrapper;
        wrapper.DecodeAndCheckHashed();
        wrapper.DecodeAndCheckGC();
    }

    // check after lightweight lock
    {
        MarkWordWrapper<Getter> wrapper;
        wrapper.DecodeAndCheckLightLock();
        wrapper.DecodeAndCheckGC();
    }

    // check after heavyweight lock
    {
        MarkWordWrapper<Getter> wrapper;
        wrapper.DecodeAndCheckHeavyLock();
        wrapper.DecodeAndCheckGC();
    }
}

TEST_F(MarkWordTest, CreateGCWithRandomValues)
{
    CheckMakeGC<RandomTestValuesGetter>();
}

TEST_F(MarkWordTest, CreateGCWithMaxValues)
{
    CheckMakeGC<MaxTestValuesGetter>();
}

template <class Getter>
void MarkWordTest::CheckMarkingWithGC()
{
    Getter param_getter;

    // with unlocked
    {
        MarkWordWrapper<Getter> wrapper;

        wrapper.SetMarkedForGC();
        wrapper.CheckUnlocked(true);
    }

    // with lightweight locked
    {
        MarkWordWrapper<Getter> wrapper;
        auto t_id = param_getter.GetThreadId();
        auto l_count = param_getter.GetLockCount();
        wrapper.DecodeLightLock(t_id, l_count);

        wrapper.SetMarkedForGC();
        wrapper.CheckLightweightLock(t_id, l_count, true);
    }

    // with heavyweight locked
    {
        MarkWordWrapper<Getter> wrapper;
        auto m_id = param_getter.GetMonitorId();
        wrapper.DecodeHeavyLock(m_id);

        wrapper.SetMarkedForGC();
        wrapper.CheckHeavyweightLock(m_id, true);
    }

    // with hashed
    {
        MarkWordWrapper<Getter> wrapper;
        auto hash = param_getter.GetHash();
        wrapper.DecodeHash(hash);

        wrapper.SetMarkedForGC();
        wrapper.CheckHashed(hash, true);
    }
}

TEST_F(MarkWordTest, MarkWithGCWithRandValues)
{
    CheckMarkingWithGC<RandomTestValuesGetter>();
}

TEST_F(MarkWordTest, MarkWithGCWithMaxValues)
{
    CheckMarkingWithGC<MaxTestValuesGetter>();
}

template <class Getter>
void MarkWordTest::CheckReadBarrierSet()
{
    Getter param_getter;

    // with unlocked
    {
        MarkWordWrapper<Getter> wrapper;

        wrapper.SetReadBarrier();
        wrapper.CheckUnlocked(false, true);
    }

    // with lightweight locked
    {
        MarkWordWrapper<Getter> wrapper;
        auto t_id = param_getter.GetThreadId();
        auto l_count = param_getter.GetLockCount();
        wrapper.DecodeLightLock(t_id, l_count);

        wrapper.SetReadBarrier();
        wrapper.CheckLightweightLock(t_id, l_count, false, true);
    }

    // with heavyweight locked
    {
        MarkWordWrapper<Getter> wrapper;
        auto m_id = param_getter.GetMonitorId();
        wrapper.DecodeHeavyLock(m_id);

        wrapper.SetReadBarrier();
        wrapper.CheckHeavyweightLock(m_id, false, true);
    }

    // with hashed
    {
        MarkWordWrapper<Getter> wrapper;
        auto hash = param_getter.GetHash();
        wrapper.DecodeHash(hash);

        wrapper.SetReadBarrier();
        wrapper.CheckHashed(hash, false, true);
    }
}

TEST_F(MarkWordTest, ReadBarrierSetWithRandValues)
{
    CheckReadBarrierSet<RandomTestValuesGetter>();
}

TEST_F(MarkWordTest, ReadBarrierSetWithMaxValues)
{
    CheckReadBarrierSet<MaxTestValuesGetter>();
}

}  //  namespace panda
