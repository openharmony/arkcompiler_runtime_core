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

#include "libarkbase/os/thread.h"

#include <array>
#include <unistd.h>

namespace ark::tooling::hprof::test {

// Test data constants
static constexpr size_t SMALL_WRITE_SIZE = 100;
static constexpr size_t LARGE_WRITE_SIZE = 100 * 1024;
static constexpr size_t MEDIUM_WRITE_SIZE = 50;
static constexpr size_t EXACT_CAPACITY_BUFFER_SIZE = 64 * 1024;
static constexpr int CONCURRENT_WRITER_THREAD_COUNT = 4;
static constexpr int WRITES_PER_CONCURRENT_THREAD = 1000;
static constexpr int MIXED_WRITE_SMALL_BYTE_COUNT = 100;
static constexpr int MIXED_WRITE_LARGE_BLOCK_SIZE = 1024;
static constexpr int FLUSH_CYCLE_COUNT = 10;
static constexpr uint8_t TEST_BYTE_11 = 0x11;
static constexpr uint8_t TEST_BYTE_22 = 0x22;
static constexpr uint8_t TEST_BYTE_42 = 0x42;
static constexpr uint8_t TEST_BYTE_55 = 0x55;
static constexpr uint8_t TEST_BYTE_77 = 0x77;
static constexpr uint8_t TEST_BYTE_88 = 0x88;
static constexpr uint8_t TEST_BYTE_AA = 0xAA;
static constexpr uint8_t TEST_BYTE_BB = 0xBB;
static constexpr uint8_t TEST_BYTE_EE = 0xEE;
static constexpr size_t EXPECTED_MIXED_OUTPUT_SIZE = MIXED_WRITE_SMALL_BYTE_COUNT + MIXED_WRITE_LARGE_BLOCK_SIZE;
static constexpr size_t SEQUENCE_THIRD_BYTE_INDEX = 2U;
static constexpr uint8_t SEQUENCE_FIRST_BYTE = 0x01U;
static constexpr uint8_t SEQUENCE_SECOND_BYTE = 0x02U;
static constexpr uint8_t SEQUENCE_THIRD_BYTE = 0x03U;

class OutputStreamTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        path_ = CreateTempPath();
    }
    void TearDown() override
    {
        RemoveTempFile(path_);
    }
    // Create an OutputStream via path, write data, close, read back.
    void WriteAndRead(const std::vector<uint8_t> &data)
    {
        OutputStream os(path_);
        os.Write(data.data(), data.size());
        os.Close();
        result_ = ReadFileBack(path_);
    }

    const std::string &Path() const
    {
        return path_;
    }

    const std::vector<uint8_t> &Result() const
    {
        return result_;
    }

    void SetResult(std::vector<uint8_t> result)
    {
        result_ = std::move(result);
    }

private:
    std::string path_;
    std::vector<uint8_t> result_;
};

// --- Construction / NonCopyable ---

TEST_F(OutputStreamTest, Constructor_Path_CreatesFile)
{
    OutputStream os(Path());
    ASSERT_TRUE(ReadFileBack(Path()).empty());  // file created but empty (buffered)
}

TEST_F(OutputStreamTest, Constructor_FdMode)
{
    auto file = OpenWriteOnlyFile(Path());
    ASSERT_NE(file, nullptr);
    OutputStream os(fileno(file.get()));
    uint8_t byte = TEST_BYTE_AA;
    os.Write(&byte, sizeof(uint8_t));
    os.Close();
    SetResult(ReadFileBack(Path()));
    ASSERT_EQ(Result().size(), sizeof(byte));
}

TEST_F(OutputStreamTest, NonCopyable)
{
    static_assert(!std::is_copy_constructible_v<OutputStream>);
    static_assert(!std::is_copy_assignable_v<OutputStream>);
    static_assert(!std::is_move_constructible_v<OutputStream>);
    static_assert(!std::is_move_assignable_v<OutputStream>);
    SUCCEED();
}

// --- Write basics ---

TEST_F(OutputStreamTest, Write_SingleByte)
{
    uint8_t data = TEST_BYTE_42;
    OutputStream os(Path());
    os.Write(&data, sizeof(uint8_t));
    os.Close();
    SetResult(ReadFileBack(Path()));
    ASSERT_EQ(Result().size(), sizeof(uint8_t));
    ASSERT_EQ(Result()[0], TEST_BYTE_42);
}

TEST_F(OutputStreamTest, Write_SmallBlock)
{
    std::vector<uint8_t> data(SMALL_WRITE_SIZE, TEST_BYTE_55);
    WriteAndRead(data);
    ASSERT_EQ(Result().size(), SMALL_WRITE_SIZE);
    ASSERT_EQ(Result()[0], TEST_BYTE_55);
}

// Writing exactly the buffer capacity fills it to the threshold and forces a
// flush (boundary condition: size == capacity).
TEST_F(OutputStreamTest, Write_ExactBufferSize_Flushes)
{
    // DEFAULT_BUFFER_SIZE = 64KB (see output_stream.h)
    std::vector<uint8_t> data(EXACT_CAPACITY_BUFFER_SIZE, TEST_BYTE_EE);
    OutputStream os(Path());
    os.Write(data.data(), data.size());
    os.Close();
    SetResult(ReadFileBack(Path()));

    // Exact buffer size triggers flush; all 64KB should be written
    ASSERT_EQ(Result().size(), EXACT_CAPACITY_BUFFER_SIZE);
    ASSERT_EQ(Result()[0], TEST_BYTE_EE);
    ASSERT_EQ(Result()[Result().size() - 1U], TEST_BYTE_EE);
}

TEST_F(OutputStreamTest, Write_LargeBlock_BeyondBufferSize)
{
    // DEFAULT_BUFFER_SIZE = 64KB (see output_stream.h); write LARGE_WRITE_SIZE to trigger auto-flush
    std::vector<uint8_t> data(LARGE_WRITE_SIZE, TEST_BYTE_BB);
    WriteAndRead(data);
    ASSERT_EQ(Result().size(), LARGE_WRITE_SIZE);
    ASSERT_EQ(Result()[0], TEST_BYTE_BB);
    ASSERT_EQ(Result()[Result().size() - 1U], TEST_BYTE_BB);
}

TEST_F(OutputStreamTest, Write_WithHeader)
{
    std::array<uint8_t, 3U> header = {SEQUENCE_FIRST_BYTE, SEQUENCE_SECOND_BYTE, SEQUENCE_THIRD_BYTE};
    std::array<uint8_t, 2U> body = {TEST_BYTE_AA, TEST_BYTE_BB};
    OutputStream os(Path());
    os.Write(body.data(), body.size(), header.data(), header.size());
    os.Close();
    SetResult(ReadFileBack(Path()));
    ASSERT_EQ(Result().size(), header.size() + body.size());
    ASSERT_EQ(Result()[0], SEQUENCE_FIRST_BYTE);
    ASSERT_EQ(Result()[1], SEQUENCE_SECOND_BYTE);
    ASSERT_EQ(Result()[SEQUENCE_THIRD_BYTE_INDEX], SEQUENCE_THIRD_BYTE);
    ASSERT_EQ(Result()[header.size()], TEST_BYTE_AA);
    ASSERT_EQ(Result()[header.size() + sizeof(uint8_t)], TEST_BYTE_BB);
}

// --- Write edge cases ---

TEST_F(OutputStreamTest, Write_ZeroSize_NoOutput)
{
    OutputStream os(Path());
    uint8_t dummy = 0;
    os.Write(&dummy, 0);  // zero-size write -> no data
    os.Close();
    SetResult(ReadFileBack(Path()));
    ASSERT_EQ(Result().size(), 0U);
}

TEST_F(OutputStreamTest, Write_NullStream_NoCrash)
{
    // OutputStream requires a valid path, so we test invalid fd instead
    OutputStream os(-1);  // invalid fd
    uint8_t data = TEST_BYTE_42;
    ASSERT_FALSE(os.Write(&data, sizeof(uint8_t)));
}

TEST_F(OutputStreamTest, Write_HeaderSizePositive_HeaderNull_ReturnsFalse)
{
    OutputStream os(Path());
    uint8_t body = TEST_BYTE_42;
    constexpr size_t REQUESTED_HEADER_SIZE = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t);
    // headerSize>0 but header=null -> should return false
    ASSERT_FALSE(os.Write(&body, sizeof(uint8_t), nullptr, REQUESTED_HEADER_SIZE));
}

TEST_F(OutputStreamTest, Write_SizePositive_DataNull_ReturnsFalse)
{
    OutputStream os(Path());
    ASSERT_FALSE(os.Write(nullptr, MEDIUM_WRITE_SIZE));  // size>0 but data=null
}

#if defined(__linux__)
TEST_F(OutputStreamTest, WriteFailureIsStickyAndObservable)
{
    auto file = OpenWriteOnlyFile("/dev/full");
    ASSERT_NE(file, nullptr);
    OutputStream os(fileno(file.get()), 1);
    uint8_t data = TEST_BYTE_42;

    EXPECT_FALSE(os.Write(&data, sizeof(data)));
    EXPECT_FALSE(os.Flush());
    EXPECT_FALSE(os.Write(&data, sizeof(data)));
}
#endif

// --- Multiple writes (accumulate in buffer) ---

TEST_F(OutputStreamTest, MultipleWrites_Accumulate)
{
    OutputStream os(Path());
    os.Write(&SEQUENCE_FIRST_BYTE, sizeof(uint8_t));
    os.Write(&SEQUENCE_SECOND_BYTE, sizeof(uint8_t));
    os.Write(&SEQUENCE_THIRD_BYTE, sizeof(uint8_t));
    os.Close();
    SetResult(ReadFileBack(Path()));
    ASSERT_EQ(Result().size(), sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t));
    ASSERT_EQ(Result()[0], SEQUENCE_FIRST_BYTE);
    ASSERT_EQ(Result()[1], SEQUENCE_SECOND_BYTE);
    ASSERT_EQ(Result()[SEQUENCE_THIRD_BYTE_INDEX], SEQUENCE_THIRD_BYTE);
}

// --- Close / Flush ---

TEST_F(OutputStreamTest, Close_FlushesAllData)
{
    OutputStream os(Path());
    std::vector<uint8_t> data(MEDIUM_WRITE_SIZE, TEST_BYTE_77);
    os.Write(data.data(), data.size());
    // Data is buffered - file should be empty or partial
    os.Close();
    SetResult(ReadFileBack(Path()));
    ASSERT_EQ(Result().size(), MEDIUM_WRITE_SIZE);
    ASSERT_EQ(Result()[0], TEST_BYTE_77);
}

TEST_F(OutputStreamTest, Flush_ExplicitFlush)
{
    OutputStream os(Path());
    std::vector<uint8_t> data(MEDIUM_WRITE_SIZE, TEST_BYTE_88);
    os.Write(data.data(), data.size());
    ASSERT_TRUE(os.Flush());
    SetResult(ReadFileBack(Path()));
    ASSERT_EQ(Result().size(), MEDIUM_WRITE_SIZE);
    ASSERT_EQ(Result()[0], TEST_BYTE_88);
    os.Close();
}

TEST_F(OutputStreamTest, Close_DoubleClose_NoCrash)
{
    OutputStream os(Path());
    uint8_t data = TEST_BYTE_42;
    os.Write(&data, sizeof(uint8_t));
    os.Close();
    os.Close();  // second close - fd is already -1, should be safe
    SUCCEED();
}

// Concurrent Write callers are serialized by the stream's internal mutex; no
// crash and no interleaved/corrupted output.
TEST_F(OutputStreamTest, Write_MultiThreadedNoDataCorruption)
{
    OutputStream os(Path());
    std::atomic<bool> start {false};
    // Each thread writes a unique pattern: thread_id repeated writesPerThread times
    std::vector<std::thread> threads;
    threads.reserve(CONCURRENT_WRITER_THREAD_COUNT);
    for (int t = 0; t < CONCURRENT_WRITER_THREAD_COUNT; t++) {
        threads.emplace_back([&os, &start, t]() {
            // Atomic with acquire order reason: pairs with the release store that starts all writers.
            while (!start.load(std::memory_order_acquire)) {
                os::thread::Yield();
            }
            for (int i = 0; i < WRITES_PER_CONCURRENT_THREAD; i++) {
                auto byte = static_cast<uint8_t>(t + 1);
                os.Write(&byte, sizeof(uint8_t));
            }
        });
    }
    // Atomic with release order reason: publishes the start signal to every writer thread.
    start.store(true, std::memory_order_release);
    for (auto &t : threads) {
        t.join();
    }
    os.Close();
    SetResult(ReadFileBack(Path()));
    // Total bytes written = numThreads * writesPerThread
    ASSERT_EQ(Result().size(), static_cast<size_t>(CONCURRENT_WRITER_THREAD_COUNT * WRITES_PER_CONCURRENT_THREAD));
    // Each byte should be one of the valid thread patterns (1..numThreads)
    for (auto b : Result()) {
        ASSERT_GE(b, static_cast<uint8_t>(1));
        ASSERT_LE(b, static_cast<uint8_t>(CONCURRENT_WRITER_THREAD_COUNT));
    }
    // Count bytes per thread pattern - total count should match expected
    for (int t = 0; t < CONCURRENT_WRITER_THREAD_COUNT; t++) {
        auto pattern = static_cast<uint8_t>(t + 1);
        size_t count = 0;
        for (auto b : Result()) {
            if (b == pattern) {
                count++;
            }
        }
        ASSERT_EQ(count, static_cast<size_t>(WRITES_PER_CONCURRENT_THREAD));
    }
}

TEST_F(OutputStreamTest, Write_MultiThreadedMixedOperations)
{
    OutputStream os(Path());
    std::atomic<bool> start {false};
    // Thread 1: writes small blocks
    // Thread 2: writes large blocks
    // Thread 3: calls Flush periodically
    std::thread t1([&os, &start]() {
        // Atomic with acquire order reason: pairs with the release store that starts all workers.
        while (!start.load(std::memory_order_acquire)) {
            os::thread::Yield();
        }
        for (int i = 0; i < MIXED_WRITE_SMALL_BYTE_COUNT; i++) {
            uint8_t byte = TEST_BYTE_11;
            os.Write(&byte, sizeof(uint8_t));
        }
    });
    std::thread t2([&os, &start]() {
        // Atomic with acquire order reason: pairs with the release store that starts all workers.
        while (!start.load(std::memory_order_acquire)) {
            os::thread::Yield();
        }
        std::vector<uint8_t> largeBlock(MIXED_WRITE_LARGE_BLOCK_SIZE, TEST_BYTE_22);
        os.Write(largeBlock.data(), largeBlock.size());
    });
    std::thread t3([&os, &start]() {
        // Atomic with acquire order reason: pairs with the release store that starts all workers.
        while (!start.load(std::memory_order_acquire)) {
            os::thread::Yield();
        }
        for (int i = 0; i < FLUSH_CYCLE_COUNT; i++) {
            os::thread::NativeSleep(1U);
            os.Flush();
        }
    });
    // Atomic with release order reason: publishes the start signal to every worker thread.
    start.store(true, std::memory_order_release);
    t1.join();
    t2.join();
    t3.join();
    os.Close();
    SetResult(ReadFileBack(Path()));
    // 100 * 1-byte writes (0x11) + 1024-byte block (0x22) = 1124 total
    ASSERT_EQ(Result().size(), EXPECTED_MIXED_OUTPUT_SIZE);
    // Verify 0x22 block content
    size_t count22 = 0;
    for (auto b : Result()) {
        if (b == TEST_BYTE_22) {
            count22++;
        }
    }
    ASSERT_EQ(count22, static_cast<size_t>(MIXED_WRITE_LARGE_BLOCK_SIZE));
}

}  // namespace ark::tooling::hprof::test
