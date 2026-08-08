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
static constexpr size_t BELOW_MINIMUM_BUFFER_SIZE = 5U;
static constexpr size_t WRITE_BYTES_TEST_SIZE = 100U;
static constexpr size_t NULL_STREAM_PAYLOAD_SIZE = 10U;
static constexpr uint64_t SECOND_ITEM_VALUE_MULTIPLIER = 2ULL;
static constexpr uint8_t PRIMARY_RECORD_TAG = 0x01U;
static constexpr uint8_t SECONDARY_RECORD_TAG = 0x02U;
static constexpr uint8_t COUNT_TEST_RECORD_TAG = 0x05U;
static constexpr uint8_t SINGLE_BYTE_VALUE = 0xABU;
static constexpr uint8_t FIRST_PAYLOAD_BYTE = 0xAAU;
static constexpr uint8_t SECOND_PAYLOAD_BYTE = 0xBBU;
static constexpr uint8_t NULL_DATA_FOLLOWUP_BYTE = 0xCCU;
static constexpr uint8_t LARGE_BLOCK_FILL_BYTE = 0xDDU;
static constexpr uint8_t OVERFLOW_BODY_FILL_BYTE = 0x55U;
static constexpr uint8_t MIXED_BLOCK_FILL_BYTE = 0x77U;
static constexpr uint16_t SAMPLE_U16_VALUE = 0x1234U;
static constexpr uint16_t MIXED_WRITE_U16_VALUE = 0x0202U;
static constexpr uint32_t SAMPLE_U32_VALUE = 0x01020304U;
static constexpr uint32_t BACKFILL_U32_VALUE = 0xDEADBEEFU;
static constexpr uint32_t MIXED_WRITE_U32_VALUE = 0x03030303U;
static constexpr uint32_t FIRST_COUNTED_ITEM_VALUE = 0x11111111U;
static constexpr uint32_t SECOND_COUNTED_ITEM_VALUE = 0x22222222U;
static constexpr uint32_t THIRD_COUNTED_ITEM_VALUE = 0x33333333U;
static constexpr uint64_t SAMPLE_U64_VALUE = 0x0102030405060708ULL;
static constexpr uint64_t MIXED_WRITE_U64_VALUE = 0x0404040404040404ULL;
static constexpr uint8_t FIRST_RECORD_TAG = 0xE0U;
static constexpr uint8_t SECOND_RECORD_TAG = 0xE1U;
static constexpr uint64_t FIRST_RECORD_U64_VALUE = 0x123456789ABCDEF0ULL;
static constexpr uint64_t SECOND_RECORD_FIRST_U64_VALUE = 0x1111111111111111ULL;
static constexpr uint64_t SECOND_RECORD_SECOND_U64_VALUE = 0x2222222222222222ULL;

/**
 * TestWriter exposes AbstractWriter's protected methods for unit testing.
 * In the production design, callers use tag-specific WriteXXXItem methods
 * on derived writers (CommonWriter, StaticWriter) which internally call
 * FinishItem and WriteU*. This TestWriter bypasses that encapsulation
 * so we can directly test AbstractWriter's flush/count/buffer mechanics.
 */
class TestWriter : public AbstractWriter {
public:
    using AbstractWriter::FinishItem;
    using AbstractWriter::WriteBytes;
    using AbstractWriter::WriteU16;
    using AbstractWriter::WriteU32;
    using AbstractWriter::WriteU64;
    using AbstractWriter::WriteU8;

    explicit TestWriter(OutputStream *stream, size_t bufferSize = DEFAULT_BUFFER_SIZE)
        : AbstractWriter(stream, bufferSize)
    {
    }
};

class AbstractWriterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        path_ = CreateTempPath();
        stream_ = std::make_unique<OutputStream>(path_);
        writer_ = std::make_unique<TestWriter>(stream_.get());
    }

    void TearDown() override
    {
        writer_.reset();
        stream_->Close();
        stream_.reset();
        RemoveTempFile(path_);
    }

    // Flush OutputStream and read file back for verification.
    void FlushAndRead()
    {
        stream_->Flush();
        data_ = ReadFileBack(path_);
    }

    TestWriter &Writer()
    {
        return *writer_;
    }

    OutputStream &Stream()
    {
        return *stream_;
    }

    const std::vector<uint8_t> &Data() const
    {
        return data_;
    }

    void SetData(std::vector<uint8_t> data)
    {
        data_ = std::move(data);
    }

    void DestroyWriter()
    {
        writer_.reset();
    }

private:
    std::string path_;
    std::unique_ptr<OutputStream> stream_;
    std::unique_ptr<TestWriter> writer_;
    std::vector<uint8_t> data_;
};

// --- Construction / NonCopyable / Minimum buffer ---

TEST_F(AbstractWriterTest, Constructor_WithStream)
{
    SUCCEED();
}

TEST_F(AbstractWriterTest, Constructor_MinBufferClamped)
{
    // Use a separate path to avoid conflict with the fixture stream.
    std::string separatePath = CreateTempPath();
    OutputStream os(separatePath);
    TestWriter w(&os, BELOW_MINIMUM_BUFFER_SIZE);  // bufferSize clamped to at least RECORD_HEADER_SIZE(17)
    w.BeginRecord(PRIMARY_RECORD_TAG);
    w.WriteU8(FIRST_PAYLOAD_BYTE);
    w.FinishItem();  // mark item as complete
    w.EndRecord();
    os.Close();  // flush buffered data to disk
    SetData(ReadFileBack(separatePath));
    ASSERT_GE(Data().size(), RECORD_HEADER_SIZE + 1U);
    ASSERT_EQ(Data().at(TAG_OFFSET), PRIMARY_RECORD_TAG);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), FIRST_PAYLOAD_BYTE);
    RemoveTempFile(separatePath);
}

TEST_F(AbstractWriterTest, Constructor_NullStream)
{
    TestWriter w(nullptr);
    w.BeginRecord(PRIMARY_RECORD_TAG);
    w.WriteU8(PRIMARY_RECORD_TAG);
    w.FinishItem();
    w.EndRecord();
    SUCCEED();
}

// --- WriteU8 ---

TEST_F(AbstractWriterTest, WriteU8_SingleByte)
{
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteU8(SINGLE_BYTE_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + 1U);
    ASSERT_EQ(Data().at(TAG_OFFSET), PRIMARY_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), SIZE_OFFSET), 1U);
    ASSERT_EQ(ReadU32LE(Data(), COUNT_OFFSET), 1U);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), SINGLE_BYTE_VALUE);
}

TEST_F(AbstractWriterTest, WriteU8_Range)
{
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteU8(0);
    Writer().WriteU8(std::numeric_limits<uint8_t>::max());
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), 0);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE + 1U), std::numeric_limits<uint8_t>::max());
}

// --- WriteU16 ---

TEST_F(AbstractWriterTest, WriteU16)
{
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteU16(SAMPLE_U16_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_EQ(ReadU16LE(Data(), RECORD_HEADER_SIZE), SAMPLE_U16_VALUE);
}

// --- WriteU32 ---

TEST_F(AbstractWriterTest, WriteU32)
{
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteU32(SAMPLE_U32_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_EQ(ReadU32LE(Data(), RECORD_HEADER_SIZE), SAMPLE_U32_VALUE);
}

// --- WriteU64 ---

TEST_F(AbstractWriterTest, WriteU64)
{
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteU64(SAMPLE_U64_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_EQ(ReadU64LE(Data(), RECORD_HEADER_SIZE), SAMPLE_U64_VALUE);
}

// --- WriteBytes ---

TEST_F(AbstractWriterTest, WriteBytes_ZeroSize)
{
    // BeginRecord + no body + EndRecord -> empty buffer -> no output on disk.
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_TRUE(Data().empty());
}

TEST_F(AbstractWriterTest, WriteBytes_SmallBlock)
{
    std::array<uint8_t, WRITE_BYTES_TEST_SIZE> buffer {};
    buffer.fill(FIRST_PAYLOAD_BYTE);
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteBytes(buffer.data(), buffer.size());
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + WRITE_BYTES_TEST_SIZE);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), FIRST_PAYLOAD_BYTE);
}

// A block larger than the buffer capacity must trigger at least one flush
// cycle rather than overflowing the in-memory buffer.
TEST_F(AbstractWriterTest, WriteBytes_LargeBlock_MultipleFlushes)
{
    // DEFAULT_BUFFER_SIZE = 64KB; write 128KB to trigger 2+ flushes
    constexpr size_t MULTI_FLUSH_BLOCK_SIZE = 128 * 1024;
    std::vector<uint8_t> body(MULTI_FLUSH_BLOCK_SIZE, LARGE_BLOCK_FILL_BYTE);
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteBytes(body.data(), body.size());
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    // Verify the full 128KB was written (flush happened at least once)
    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + MULTI_FLUSH_BLOCK_SIZE);
    ASSERT_EQ(Data().at(TAG_OFFSET), PRIMARY_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), SIZE_OFFSET), MULTI_FLUSH_BLOCK_SIZE);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), LARGE_BLOCK_FILL_BYTE);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE + MULTI_FLUSH_BLOCK_SIZE - 1U), LARGE_BLOCK_FILL_BYTE);
}

// --- BeginRecord / EndRecord (timestamp + size backfill) ---

TEST_F(AbstractWriterTest, BeginEndRecord_SizeBackfill)
{
    Writer().BeginRecord(SINGLE_BYTE_VALUE);
    Writer().WriteU32(BACKFILL_U32_VALUE);
    Writer().WriteU16(SAMPLE_U16_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    static constexpr size_t BACKFILL_BODY_SIZE = sizeof(uint32_t) + sizeof(uint16_t);
    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + BACKFILL_BODY_SIZE);
    ASSERT_EQ(Data().at(TAG_OFFSET), SINGLE_BYTE_VALUE);
    ASSERT_NE(ReadU64LE(Data(), TIMESTAMP_OFFSET), 0U);
    ASSERT_EQ(ReadU32LE(Data(), SIZE_OFFSET), BACKFILL_BODY_SIZE);
    ASSERT_EQ(ReadU32LE(Data(), COUNT_OFFSET), 1U);
    ASSERT_EQ(ReadU32LE(Data(), RECORD_HEADER_SIZE), BACKFILL_U32_VALUE);
    ASSERT_EQ(ReadU16LE(Data(), RECORD_HEADER_SIZE + sizeof(uint32_t)), SAMPLE_U16_VALUE);
}

TEST_F(AbstractWriterTest, BeginEndRecord_EmptyBody_NoOutput)
{
    // BeginRecord then EndRecord with no body writes -> empty buffer -> no record on disk.
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().EndRecord();
    FlushAndRead();
    ASSERT_TRUE(Data().empty());  // empty body -> EndRecord does nothing
}

TEST_F(AbstractWriterTest, BeginEndRecord_MultipleRecords)
{
    Writer().BeginRecord(FIRST_RECORD_TAG);
    Writer().WriteU8(PRIMARY_RECORD_TAG);
    Writer().WriteU64(FIRST_RECORD_U64_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();

    Writer().BeginRecord(SECOND_RECORD_TAG);
    Writer().WriteU64(SECOND_RECORD_FIRST_U64_VALUE);
    Writer().WriteU64(SECOND_RECORD_SECOND_U64_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    // Record 1: RECORD_HEADER_SIZE + 9 = 26 bytes; Record 2: RECORD_HEADER_SIZE + 16 = 33 bytes; Total = 59
    size_t hdr1 = 0;
    // Second record starts after first: RECORD_HEADER_SIZE (17) + 1-byte body for first item
    static constexpr size_t FIRST_RECORD_BODY_SIZE = 9U;
    size_t hdr2 = RECORD_HEADER_SIZE + FIRST_RECORD_BODY_SIZE;
    ASSERT_EQ(Data().size(), (RECORD_HEADER_SIZE + FIRST_RECORD_BODY_SIZE) + (RECORD_HEADER_SIZE + 16U));
    ASSERT_EQ(Data().at(hdr1 + TAG_OFFSET), FIRST_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), hdr1 + SIZE_OFFSET), FIRST_RECORD_BODY_SIZE);
    ASSERT_EQ(ReadU32LE(Data(), hdr1 + COUNT_OFFSET), 1U);
    ASSERT_EQ(Data().at(hdr2 + TAG_OFFSET), SECOND_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), hdr2 + SIZE_OFFSET), 2U * sizeof(uint64_t));
    ASSERT_EQ(ReadU32LE(Data(), hdr2 + COUNT_OFFSET), 1U);
}

// --- Buffer exceeded: FinishItem flush + continuation ---

TEST_F(AbstractWriterTest, FlushOnOverflow_SmallOvershoot)
{
    constexpr size_t SINGLE_CHUNK_BODY_SIZE = 9000;
    std::vector<uint8_t> body(SINGLE_CHUNK_BODY_SIZE, OVERFLOW_BODY_FILL_BYTE);
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteBytes(body.data(), body.size());
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    // 9000 bytes fits in 64KB buffer - no flush during writes.
    // EndRecord flushes the entire record as one chunk.
    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + SINGLE_CHUNK_BODY_SIZE);
    ASSERT_EQ(Data().at(TAG_OFFSET), PRIMARY_RECORD_TAG);
    ASSERT_NE(ReadU64LE(Data(), TIMESTAMP_OFFSET), 0U);
    ASSERT_EQ(ReadU32LE(Data(), SIZE_OFFSET), SINGLE_CHUNK_BODY_SIZE);
    ASSERT_EQ(ReadU32LE(Data(), COUNT_OFFSET), 1U);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), OVERFLOW_BODY_FILL_BYTE);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE + SINGLE_CHUNK_BODY_SIZE - 1U), OVERFLOW_BODY_FILL_BYTE);
}

TEST_F(AbstractWriterTest, FinishItem_FlushOnThreshold)
{
    // Write items until buffer exceeds threshold, triggering flush at FinishItem().
    // Each item is 17 bytes (WriteU64 + WriteU64 + WriteU8).
    // Use a small buffer (512 bytes, overriding DEFAULT_BUFFER_SIZE - see abstract_writer.h) so flush triggers quickly:
    // 30 items = 510 bytes < 512 (no flush), 31 items = 527 bytes >= 512 (flush).
    // After flush, remaining items go into a new continuation record.
    std::string separatePath = CreateTempPath();
    OutputStream os(separatePath);
    constexpr size_t THRESHOLD_TEST_BUFFER_SIZE = 512;
    TestWriter w(&os, THRESHOLD_TEST_BUFFER_SIZE);

    constexpr size_t FIRST_CHUNK_ITEM_COUNT = 31;  // triggers flush at FinishItem (527 >= 512)
    constexpr size_t TOTAL_ITEM_COUNT = 32;        // 31 in chunk1 + 1 in chunk2
    constexpr size_t SERIALIZED_ITEM_SIZE = 17;    // WriteU64 + WriteU64 + WriteU8
    w.BeginRecord(PRIMARY_RECORD_TAG);
    for (size_t i = 0; i < TOTAL_ITEM_COUNT; i++) {
        w.WriteU64(static_cast<uint64_t>(i));
        w.WriteU64(static_cast<uint64_t>(i) * SECOND_ITEM_VALUE_MULTIPLIER);
        w.WriteU8(static_cast<uint8_t>(i));
        w.FinishItem();
    }
    w.EndRecord();
    os.Close();
    SetData(ReadFileBack(separatePath));

    // Chunk 1: itemsInChunk1 items, body = itemsInChunk1 * itemBodySize = 527 bytes
    size_t chunk1Hdr = 0;
    size_t chunk1BodyStart = RECORD_HEADER_SIZE;
    uint32_t chunk1BodySize = ReadU32LE(Data(), chunk1Hdr + SIZE_OFFSET);
    uint32_t chunk1Count = ReadU32LE(Data(), chunk1Hdr + COUNT_OFFSET);
    ASSERT_EQ(chunk1BodySize, FIRST_CHUNK_ITEM_COUNT * SERIALIZED_ITEM_SIZE);
    ASSERT_EQ(chunk1Count, FIRST_CHUNK_ITEM_COUNT);
    ASSERT_EQ(Data().at(chunk1Hdr + TAG_OFFSET), PRIMARY_RECORD_TAG);

    // Chunk 2 (continuation): 1 item, body = itemBodySize = 17 bytes
    size_t chunk2Hdr = chunk1BodyStart + chunk1BodySize;
    uint32_t chunk2BodySize = ReadU32LE(Data(), chunk2Hdr + SIZE_OFFSET);
    uint32_t chunk2Count = ReadU32LE(Data(), chunk2Hdr + COUNT_OFFSET);
    ASSERT_EQ(chunk2BodySize, SERIALIZED_ITEM_SIZE);
    ASSERT_EQ(chunk2Count, 1U);
    ASSERT_EQ(Data().at(chunk2Hdr + TAG_OFFSET), PRIMARY_RECORD_TAG);

    // Same tag in both chunks
    ASSERT_EQ(Data().at(chunk1Hdr + TAG_OFFSET), Data().at(chunk2Hdr + TAG_OFFSET));

    // Total size on disk: (RECORD_HEADER_SIZE + chunk1BodySize) + (RECORD_HEADER_SIZE + itemBodySize)
    ASSERT_EQ(Data().size(), (RECORD_HEADER_SIZE + FIRST_CHUNK_ITEM_COUNT * SERIALIZED_ITEM_SIZE) +
                                 (RECORD_HEADER_SIZE + SERIALIZED_ITEM_SIZE));
    RemoveTempFile(separatePath);
}

TEST_F(AbstractWriterTest, FinishItem_CountField)
{
    // BeginRecord + 3 FinishItem calls -> count=3 in header
    Writer().BeginRecord(COUNT_TEST_RECORD_TAG);
    Writer().WriteU32(FIRST_COUNTED_ITEM_VALUE);
    Writer().FinishItem();
    Writer().WriteU32(SECOND_COUNTED_ITEM_VALUE);
    Writer().FinishItem();
    Writer().WriteU32(THIRD_COUNTED_ITEM_VALUE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    ASSERT_EQ(Data().at(TAG_OFFSET), COUNT_TEST_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), SIZE_OFFSET), 3U * sizeof(uint32_t));
    ASSERT_EQ(ReadU32LE(Data(), COUNT_OFFSET), 3U);
}

// --- Auto-EndRecord on BeginRecord ---

TEST_F(AbstractWriterTest, AutoFlush_PendingRecord)
{
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteU8(FIRST_PAYLOAD_BYTE);
    Writer().FinishItem();  // complete the first item
    // No EndRecord - BeginRecord auto-flushes the pending record.
    Writer().BeginRecord(SECONDARY_RECORD_TAG);
    Writer().WriteU8(SECOND_PAYLOAD_BYTE);
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    // Second record header offset: after first record (RECORD_HEADER_SIZE + 1-byte body)
    static constexpr size_t SECOND_RECORD_HEADER_OFFSET = RECORD_HEADER_SIZE + sizeof(uint8_t);

    ASSERT_EQ(Data().size(), SECOND_RECORD_HEADER_OFFSET + RECORD_HEADER_SIZE + sizeof(uint8_t));
    ASSERT_EQ(Data().at(TAG_OFFSET), PRIMARY_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), SIZE_OFFSET), sizeof(uint8_t));
    ASSERT_EQ(ReadU32LE(Data(), COUNT_OFFSET), 1U);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), FIRST_PAYLOAD_BYTE);
    ASSERT_EQ(Data().at(SECOND_RECORD_HEADER_OFFSET + TAG_OFFSET), SECONDARY_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), SECOND_RECORD_HEADER_OFFSET + SIZE_OFFSET), sizeof(uint8_t));
    ASSERT_EQ(ReadU32LE(Data(), SECOND_RECORD_HEADER_OFFSET + COUNT_OFFSET), 1U);
    ASSERT_EQ(Data().at(SECOND_RECORD_HEADER_OFFSET + RECORD_HEADER_SIZE), SECOND_PAYLOAD_BYTE);
}

// --- Null stream safety ---

TEST_F(AbstractWriterTest, NullStream_NoCrash)
{
    TestWriter w(nullptr);
    w.BeginRecord(PRIMARY_RECORD_TAG);
    w.WriteU8(PRIMARY_RECORD_TAG);
    w.WriteU16(MIXED_WRITE_U16_VALUE);
    w.WriteU32(MIXED_WRITE_U32_VALUE);
    w.WriteU64(MIXED_WRITE_U64_VALUE);
    std::array<uint8_t, NULL_STREAM_PAYLOAD_SIZE> buffer {};
    w.WriteBytes(buffer.data(), buffer.size());
    w.FinishItem();
    w.EndRecord();
    SUCCEED();
}

// --- Mixed writes (single chunk, fits in buffer) ---

TEST_F(AbstractWriterTest, MixedWriteSequence)
{
    Writer().BeginRecord(SECONDARY_RECORD_TAG);
    Writer().WriteU8(PRIMARY_RECORD_TAG);
    Writer().WriteU16(MIXED_WRITE_U16_VALUE);
    Writer().WriteU32(MIXED_WRITE_U32_VALUE);
    Writer().WriteU64(MIXED_WRITE_U64_VALUE);
    std::array<uint8_t, WRITE_BYTES_TEST_SIZE> buffer {};
    buffer.fill(MIXED_BLOCK_FILL_BYTE);
    Writer().WriteBytes(buffer.data(), buffer.size());
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    static constexpr size_t MIXED_BODY_SIZE =
        sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint64_t) + WRITE_BYTES_TEST_SIZE;
    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + MIXED_BODY_SIZE);
    ASSERT_EQ(Data().at(TAG_OFFSET), SECONDARY_RECORD_TAG);
    ASSERT_EQ(ReadU32LE(Data(), SIZE_OFFSET), MIXED_BODY_SIZE);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), PRIMARY_RECORD_TAG);
}

// --- Destructor auto-EndRecord (flush pending data on destruction) ---

TEST_F(AbstractWriterTest, Destructor_AutoFlushPendingRecord)
{
    // Begin a record and write an item, but do NOT call EndRecord.
    // The destructor should auto-flush the pending data.
    Writer().BeginRecord(COUNT_TEST_RECORD_TAG);
    Writer().WriteU8(FIRST_PAYLOAD_BYTE);
    Writer().WriteU8(SECOND_PAYLOAD_BYTE);
    Writer().FinishItem();  // mark item as complete so EndRecord will flush
    // Destroying the writer calls EndRecord(); flushing the stream then persists the record.
    DestroyWriter();
    FlushAndRead();

    // The record should have been flushed by the destructor
    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + 2U);
    ASSERT_EQ(Data().at(TAG_OFFSET), COUNT_TEST_RECORD_TAG);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), FIRST_PAYLOAD_BYTE);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE + 1U), SECOND_PAYLOAD_BYTE);
}

// --- WriteBytes nullptr data (edge case) ---

TEST_F(AbstractWriterTest, WriteBytes_NullData_NoWrite)
{
    Writer().BeginRecord(PRIMARY_RECORD_TAG);
    Writer().WriteBytes(nullptr, WRITE_BYTES_TEST_SIZE);  // null data -> no-op
    Writer().WriteU8(NULL_DATA_FOLLOWUP_BYTE);            // Only this byte is written to the item body.
    Writer().FinishItem();
    Writer().EndRecord();
    FlushAndRead();

    ASSERT_EQ(Data().size(), RECORD_HEADER_SIZE + 1U);
    ASSERT_EQ(Data().at(RECORD_HEADER_SIZE), NULL_DATA_FOLLOWUP_BYTE);
}

}  // namespace ark::tooling::hprof::test
