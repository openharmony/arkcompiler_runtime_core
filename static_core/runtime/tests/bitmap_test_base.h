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

#include <cstdlib>
#include <memory>
#include <vector>

#include "runtime/mem/gc/bitmap.h"

namespace panda::mem {

class BitmapTest : public testing::Test {
public:
    static constexpr ObjectPointerType HEAP_STARTING_ADDRESS = static_cast<ObjectPointerType>(0x10000000);
};

using BitmapWordType = panda::mem::Bitmap::BitmapWordType;

class BitmapVerify {
public:
    using BitmapType = MemBitmap<DEFAULT_ALIGNMENT_IN_BYTES>;
    BitmapVerify(BitmapType *bitmap_arg, void *begin_arg, void *end_arg)
        : bitmap_(bitmap_arg), begin_(begin_arg), end_(end_arg)
    {
    }

    void operator()(void *obj)
    {
        EXPECT_TRUE(obj >= begin_);
        EXPECT_TRUE(obj <= end_);
        EXPECT_EQ(bitmap_->Test(obj), ((BitmapType::ToPointerType(obj) & ADDRESS_MASK_TO_SET) != 0));
    }

    static constexpr BitmapWordType ADDRESS_MASK_TO_SET = 0xF;

private:
    BitmapType *const bitmap_;
    void *begin_;
    void *end_;
};

static inline size_t FnRounddown(size_t val, size_t alignment)
{
    size_t mask = ~((static_cast<size_t>(1) * alignment) - 1);
    return val & mask;
}

template <size_t K_ALIGNMENT, typename TestFn>
static void RunTest(TestFn &&fn)
{
    auto heap_begin = BitmapTest::HEAP_STARTING_ADDRESS;
    const size_t heap_capacity = 16_MB;

#ifdef PANDA_NIGHTLY_TEST_ON
    // NOLINTNEXTLINE(cert-msc51-cpp)
    std::srand(time(nullptr));
#else
    // NOLINTNEXTLINE(cert-msc51-cpp,readability-magic-numbers)
    std::srand(0x1234);
#endif

    constexpr int TEST_REPEAT = 1;
    for (int i = 0; i < TEST_REPEAT; ++i) {
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        auto bm_ptr = std::make_unique<BitmapWordType[]>((heap_capacity >> Bitmap::LOG_BITSPERWORD) / K_ALIGNMENT);
        MemBitmap<K_ALIGNMENT> bm(ToVoidPtr(heap_begin), heap_capacity, bm_ptr.get());

        constexpr int NUM_BITS_TO_MODIFY = 1000;
        for (int j = 0; j < NUM_BITS_TO_MODIFY; ++j) {
            // NOLINTNEXTLINE(cert-msc50-cpp)
            size_t offset = FnRounddown(std::rand() % heap_capacity, K_ALIGNMENT);
            // NOLINTNEXTLINE(cert-msc50-cpp)
            bool set = std::rand() % 2 == 1;

            if (set) {
                bm.Set(ToVoidPtr(heap_begin + offset));
            } else {
                bm.Clear(ToVoidPtr(heap_begin + offset));
            }
        }

        constexpr int NUM_TEST_RANGES = 50;
        for (int j = 0; j < NUM_TEST_RANGES; ++j) {
            // NOLINTNEXTLINE(cert-msc50-cpp)
            const size_t offset = FnRounddown(std::rand() % heap_capacity, K_ALIGNMENT);
            const size_t remain = heap_capacity - offset;
            // NOLINTNEXTLINE(cert-msc50-cpp)
            const size_t end = offset + FnRounddown(std::rand() % (remain + 1), K_ALIGNMENT);

            size_t manual = 0;
            for (ObjectPointerType k = offset; k < end; k += K_ALIGNMENT) {
                if (bm.Test(ToVoidPtr(heap_begin + k))) {
                    manual++;
                }
            }

            fn(&bm, heap_begin + offset, heap_begin + end, manual);
        }
    }
}

template <size_t K_ALIGNMENT>
static void RunTestCount()
{
    auto count_test_fn = [](MemBitmap<K_ALIGNMENT> *bitmap, ObjectPointerType begin, ObjectPointerType end,
                            size_t manual_count) {
        size_t count = 0;
        auto count_fn = [&count, begin, end]([[maybe_unused]] void *obj) {
            auto p = ToObjPtr(obj);
            if (p >= begin && p < end) {
                count++;
            }
        };
        bitmap->IterateOverMarkedChunkInRange(ToVoidPtr(begin), ToVoidPtr(end), count_fn);
        EXPECT_EQ(count, manual_count);
    };
    RunTest<K_ALIGNMENT>(count_test_fn);
}

template <size_t K_ALIGNMENT>
void RunTestOrder()
{
    auto order_test_fn = [](MemBitmap<K_ALIGNMENT> *bitmap, ObjectPointerType begin, ObjectPointerType end,
                            size_t manual_count) {
        void *last_ptr = nullptr;
        auto order_check = [&last_ptr](void *obj) {
            EXPECT_LT(last_ptr, obj);
            last_ptr = obj;
        };

        // Test complete walk.
        bitmap->IterateOverChunks(order_check);
        if (manual_count > 0) {
            EXPECT_NE(nullptr, last_ptr);
        }

        // Test range.
        last_ptr = nullptr;
        bitmap->IterateOverMarkedChunkInRange(ToVoidPtr(begin), ToVoidPtr(end), order_check);
        if (manual_count > 0) {
            EXPECT_NE(nullptr, last_ptr);
        }
    };
    RunTest<K_ALIGNMENT>(order_test_fn);
}

TEST_F(BitmapTest, AtomicClearSetTest)
{
    void *object = ToVoidPtr(HEAP_STARTING_ADDRESS);
    const size_t sz = 1_MB;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    auto bm_ptr = std::make_unique<BitmapWordType[]>(sz >> MemBitmap<>::LOG_BITSPERWORD);
    MemBitmap<> bm(ToVoidPtr(HEAP_STARTING_ADDRESS), sz, bm_ptr.get());

    // Set bit
    ASSERT_TRUE(bm.Test(object) == bm.AtomicTestAndSet(object));
    ASSERT_TRUE(bm.Test(object));
    ASSERT_TRUE(bm.AtomicTest(object));

    // Clear bit
    ASSERT_TRUE(bm.Test(object) == bm.AtomicTestAndClear(object));
    ASSERT_TRUE(!bm.Test(object));
    ASSERT_TRUE(!bm.AtomicTest(object));
}

}  // namespace panda::mem
