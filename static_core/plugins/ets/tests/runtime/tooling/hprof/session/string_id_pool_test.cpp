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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test_common.h"

namespace ark::tooling::hprof::test {

// Test data constants
static constexpr size_t TEST_LONG_STRING_SIZE = 256 * 1024;

class StringIdPoolTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        pool_ = std::make_unique<StringIdPool>();
        path_ = CreateTempPath();
    }
    void TearDown() override
    {
        writer_.reset();
        os_.reset();
        pool_.reset();
        RemoveTempFile(path_);
    }
    // Create an OutputStream + CommonWriter for WriteStringPool tests.
    void CreateWriter()
    {
        os_ = std::make_unique<OutputStream>(path_);
        writer_ = std::make_unique<CommonWriter>(os_.get());
    }
    // Close the stream, then read file back. EndRecord already writes to stream.
    void FinalizeAndRead()
    {
        os_->Close();
        data_ = ReadFileBack(path_);
    }
    void CleanupWriter()
    {
        writer_.reset();
        os_.reset();
    }

    StringIdPool *Pool() const
    {
        return pool_.get();
    }

    OutputStream *Stream() const
    {
        return os_.get();
    }

    CommonWriter *Writer() const
    {
        return writer_.get();
    }

    const std::string &Path() const
    {
        return path_;
    }

    const std::vector<uint8_t> &Data() const
    {
        return data_;
    }

private:
    std::unique_ptr<StringIdPool> pool_;
    std::string path_;
    std::unique_ptr<OutputStream> os_;
    std::unique_ptr<CommonWriter> writer_;
    std::vector<uint8_t> data_;
};

// --- Basic operations ---

TEST_F(StringIdPoolTest, AddString_FirstReturnsZero)
{
    ASSERT_EQ(Pool()->AddString("hello"), 0U);
}

TEST_F(StringIdPoolTest, AddString_ContinuousIDs)
{
    ASSERT_EQ(Pool()->AddString("a"), 0U);
    ASSERT_EQ(Pool()->AddString("b"), 1U);
    ASSERT_EQ(Pool()->AddString("c"), 2U);
}

TEST_F(StringIdPoolTest, AddString_Deduplication)
{
    ASSERT_EQ(Pool()->AddString("foo"), 0U);
    ASSERT_EQ(Pool()->AddString("foo"), 0U);
    ASSERT_EQ(Pool()->AddString("foo"), 0U);
}

TEST_F(StringIdPoolTest, AddString_DeduplicationKeepsFirstID)
{
    ASSERT_EQ(Pool()->AddString("foo"), 0U);
    ASSERT_EQ(Pool()->AddString("bar"), 1U);
    ASSERT_EQ(Pool()->AddString("foo"), 0U);
}

TEST_F(StringIdPoolTest, GetStringId_Found)
{
    StringId id = Pool()->AddString("hello");
    ASSERT_EQ(Pool()->GetStringId("hello"), id);
}

TEST_F(StringIdPoolTest, GetStringId_NotFound)
{
    ASSERT_EQ(Pool()->GetStringId("nonexistent"), INVALID_STRING_ID);
    ASSERT_EQ(INVALID_STRING_ID, UINT32_MAX);
}

TEST_F(StringIdPoolTest, GetStringById_ReverseLookup)
{
    Pool()->AddString("x");
    Pool()->AddString("y");
    ASSERT_EQ(Pool()->GetStringById(0), "x");
    ASSERT_EQ(Pool()->GetStringById(1), "y");
}

TEST_F(StringIdPoolTest, AddString_EmptyString)
{
    ASSERT_EQ(Pool()->AddString(""), 0U);
    ASSERT_EQ(Pool()->AddString(""), 0U);
}

TEST_F(StringIdPoolTest, AddString_LongString)
{
    std::string longStr(TEST_LONG_STRING_SIZE, 'X');
    ASSERT_EQ(Pool()->AddString(longStr), 0U);
}

TEST_F(StringIdPoolTest, GetStringById_OutOfBounds)
{
    ASSERT_EQ(Pool()->GetStringById(INVALID_STRING_ID), "");
    ASSERT_EQ(Pool()->GetStringById(999U), "");
    Pool()->AddString("only");
    ASSERT_EQ(Pool()->GetStringById(1U), "");
}

// --- Freeze / Unfreeze ---

TEST_F(StringIdPoolTest, Freeze_RejectsAddString)
{
    Pool()->AddString("before");
    Pool()->Freeze();
    ASSERT_EQ(Pool()->AddString("after"), INVALID_STRING_ID);
    ASSERT_EQ(Pool()->GetStringId("before"), 0U);  // existing still accessible
}

TEST_F(StringIdPoolTest, Unfreeze_AllowsAddStringAgain)
{
    Pool()->AddString("first");
    Pool()->Freeze();
    ASSERT_EQ(Pool()->AddString("blocked"), INVALID_STRING_ID);
    Pool()->Unfreeze();
    ASSERT_EQ(Pool()->AddString("second"), 1U);
}

TEST_F(StringIdPoolTest, IsFrozen_InitialState)
{
    ASSERT_FALSE(Pool()->IsFrozen());
}

TEST_F(StringIdPoolTest, IsFrozen_AfterFreezeAndUnfreeze)
{
    Pool()->Freeze();
    ASSERT_TRUE(Pool()->IsFrozen());
    Pool()->Unfreeze();
    ASSERT_FALSE(Pool()->IsFrozen());
}

TEST_F(StringIdPoolTest, Size_Empty)
{
    ASSERT_EQ(Pool()->Size(), 0U);
}

TEST_F(StringIdPoolTest, Size_AfterAdditions)
{
    Pool()->AddString("a");
    Pool()->AddString("b");
    ASSERT_EQ(Pool()->Size(), 2U);
}

// --- WriteStringPool (via CommonWriter) ---

TEST_F(StringIdPoolTest, WriteStringPool_Empty)
{
    CreateWriter();
    Pool()->Freeze();
    Pool()->ForEachString([](StringId id, const std::string &value) {
        (void)id;
        (void)value;
    });  // no strings
    CommonWriter cw(Stream());
    cw.WriteStringPool(Pool());
    FinalizeAndRead();
    // Empty pool (no strings added) should produce no string records.
    ASSERT_EQ(Data().size(), 0U);
    CleanupWriter();
}

TEST_F(StringIdPoolTest, WriteStringPool_SingleRecord)
{
    Pool()->AddString("hi");
    Pool()->Freeze();
    CreateWriter();
    Writer()->WriteStringPool(Pool());
    FinalizeAndRead();
    ASSERT_GE(Data().size(), 19U);
    ASSERT_EQ(Data().at(0), TAG_STRING_IN_UTF8);
    CleanupWriter();
}

TEST_F(StringIdPoolTest, WriteStringPool_MultipleRecords)
{
    Pool()->AddString("a");
    Pool()->AddString("bb");
    Pool()->AddString("ccc");
    Pool()->Freeze();
    CreateWriter();
    Writer()->WriteStringPool(Pool());
    FinalizeAndRead();
    ASSERT_EQ(Data().at(0), TAG_STRING_IN_UTF8);
    // Batched record: 1 record header (17) + 3 string items (9+10+11=30) = 47 bytes
    ASSERT_GE(Data().size(), RECORD_HEADER_SIZE + 30U);
    CleanupWriter();
}

TEST_F(StringIdPoolTest, WriteStringPool_NoAutoFlush)
{
    Pool()->AddString("x");
    Pool()->Freeze();
    CreateWriter();
    Writer()->WriteStringPool(Pool());
    // EndRecord inside WriteStringPool already writes each record to OutputStream.
    // Close the stream and read back to verify content.
    Stream()->Close();
    std::vector<uint8_t> result = ReadFileBack(Path());
    // "x" is 1 byte: record = 17 (RECORD_HEADER_SIZE) + 4 (id) + 4 (len) + 1 (data) = 26 bytes
    ASSERT_GE(result.size(), 22U);
    CleanupWriter();
}

// Once frozen the pool is read-only; concurrent GetStringById callers must not
// crash or interleave reads on the shared storage.
TEST_F(StringIdPoolTest, Freeze_MultiThreadedGetStringByIdSafe)
{
    // Insert strings, then freeze
    for (int i = 0; i < FROZEN_READ_ENTRY_COUNT; i++) {
        Pool()->AddString("str_" + std::to_string(i));
    }
    Pool()->Freeze();
    ASSERT_TRUE(Pool()->IsFrozen());

    // Concurrent GetStringById() on frozen pool - should not crash
    std::atomic<int> errorCount {0};
    auto threadFunc = [this, &errorCount]() {
        for (int i = 0; i < FROZEN_READ_ENTRY_COUNT; i++) {
            std::string result = Pool()->GetStringById(static_cast<StringId>(i));
            std::string expected = "str_" + std::to_string(i);
            if (result != expected) {
                errorCount++;
            }
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(FROZEN_READ_THREAD_COUNT);
    for (int t = 0; t < FROZEN_READ_THREAD_COUNT; t++) {
        threads.emplace_back(threadFunc);
    }
    for (auto &t : threads) {
        t.join();
    }
    // Atomic with relaxed order reason: joining all readers already synchronizes their counter updates.
    ASSERT_EQ(errorCount.load(std::memory_order_relaxed), 0);
}

TEST_F(StringIdPoolTest, Freeze_MultiThreadedGetStringIdSafe)
{
    // Insert strings, then freeze
    constexpr int STRING_ID_LOOKUP_ENTRY_COUNT = 50;
    for (int i = 0; i < STRING_ID_LOOKUP_ENTRY_COUNT; i++) {
        Pool()->AddString("key_" + std::to_string(i));
    }
    Pool()->Freeze();

    // Concurrent GetStringId() on frozen pool - should not crash
    std::atomic<int> errorCount {0};
    auto threadFunc = [this, &errorCount]() {
        for (int i = 0; i < STRING_ID_LOOKUP_ENTRY_COUNT; i++) {
            StringId id = Pool()->GetStringId("key_" + std::to_string(i));
            if (id != static_cast<StringId>(i)) {
                errorCount++;
            }
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(FROZEN_READ_THREAD_COUNT);
    for (int t = 0; t < FROZEN_READ_THREAD_COUNT; t++) {
        threads.emplace_back(threadFunc);
    }
    for (auto &t : threads) {
        t.join();
    }
    // Atomic with relaxed order reason: joining all readers already synchronizes their counter updates.
    ASSERT_EQ(errorCount.load(std::memory_order_relaxed), 0);
}

}  // namespace ark::tooling::hprof::test
