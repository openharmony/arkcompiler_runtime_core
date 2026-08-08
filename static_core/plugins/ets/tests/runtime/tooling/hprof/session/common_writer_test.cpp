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
static constexpr uint32_t DUMP_OBJECT_COUNT = 10U;
static constexpr uint32_t DUMP_CLASS_COUNT = 3U;
static constexpr uint32_t RECORD_COUNT_OBJECTS = 100U;
static constexpr uint32_t RECORD_COUNT_CLASSES = 20U;
static constexpr uint32_t SUMMARY_DYNAMIC_OBJECT_COUNT = 5U;
static constexpr uint32_t SUMMARY_STATIC_OBJECT_COUNT = 5U;
static constexpr uint32_t FIELD_VALUES_DYNAMIC_OBJECT_COUNT = 7U;
static constexpr uint32_t BATCHED_XREF_EDGE_COUNT = 3U;
static constexpr uint32_t XREF_SOURCE_NODE_ID = 0x1000U;
static constexpr uint32_t XREF_TARGET_NODE_ID = 0x2000U;

class CommonWriterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        path_ = CreateTempPath();
        stream_ = std::make_unique<OutputStream>(path_);
        writer_ = std::make_unique<CommonWriter>(stream_.get());
    }

    void TearDown() override
    {
        writer_.reset();
        stream_.reset();
        RemoveTempFile(path_);
    }

    void FinalizeAndRead()
    {
        Writer().EndRecord();  // Flush any pending record.
        stream_->Close();
        data_ = ReadFileBack(path_);
    }

    CommonWriter &Writer()
    {
        return *writer_;
    }

    const std::vector<uint8_t> &Data() const
    {
        return data_;
    }

private:
    std::string path_;
    std::unique_ptr<OutputStream> stream_;
    std::unique_ptr<CommonWriter> writer_;
    std::vector<uint8_t> data_;
};

// --- WriteFileHeader ---

TEST_F(CommonWriterTest, WriteFileHeader_TotalSize)
{
    Writer().WriteFileHeader(Language::DYNAMIC, 0, 0);
    FinalizeAndRead();
    ASSERT_GE(Data().size(), HYBRID_DUMP_HEADER_SIZE);
    ASSERT_EQ(Data().size(), HYBRID_DUMP_HEADER_SIZE);  // Header only, no records.
}

TEST_F(CommonWriterTest, WriteFileHeader_Version)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    FinalizeAndRead();
    // Version string at offset 0: "3.0.0\0\0\0" (8 bytes)
    ASSERT_EQ(memcmp(Data().data(), HYBRID_DUMP_VERSION.data(), HYBRID_DUMP_VERSION_SIZE), 0);
}

TEST_F(CommonWriterTest, WriteFileHeader_IdentifierSize)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    FinalizeAndRead();
    // identifierSize at offset HDR_IDENTIFIER_SIZE_OFF, should be 4 (static-side nodeIds are u32)
    ASSERT_EQ(ReadU32LE(Data(), HDR_IDENTIFIER_SIZE_OFF), STATIC_OBJECT_ID_SIZE);
}

TEST_F(CommonWriterTest, WriteFileHeader_Timestamp)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    FinalizeAndRead();
    // timestamp at offset HDR_TIMESTAMP_OFF, should be nonzero (wall-clock ms)
    uint64_t ts = ReadU64LE(Data(), HDR_TIMESTAMP_OFF);
    ASSERT_NE(ts, 0U);
}

TEST_F(CommonWriterTest, WriteFileHeader_Language)
{
    Writer().WriteFileHeader(Language::HYBRID, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    FinalizeAndRead();
    // language at offset HDR_LANGUAGE_OFF as uint8_t (1 byte)
    ASSERT_EQ(Data().at(HDR_LANGUAGE_OFF), static_cast<uint8_t>(Language::HYBRID));
}

TEST_F(CommonWriterTest, WriteFileHeader_FeatureFlags)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    FinalizeAndRead();
    // featureFlags at offset HDR_FEATURE_FLAGS_OFF, should be HYBRID_DUMP_FEATURE_FLAGS (0)
    ASSERT_EQ(ReadU32LE(Data(), HDR_FEATURE_FLAGS_OFF), HYBRID_DUMP_FEATURE_FLAGS);
}

TEST_F(CommonWriterTest, WriteFileHeader_HeaderSize)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    FinalizeAndRead();
    // headerSize at offset HDR_HEADER_SIZE_OFF, should be HYBRID_DUMP_HEADER_SIZE (33)
    ASSERT_EQ(ReadU32LE(Data(), HDR_HEADER_SIZE_OFF), HYBRID_DUMP_HEADER_SIZE);
}

TEST_F(CommonWriterTest, WriteFileHeader_RecordCount)
{
    Writer().WriteFileHeader(Language::DYNAMIC, RECORD_COUNT_OBJECTS, RECORD_COUNT_CLASSES);
    FinalizeAndRead();
    // recordCount at offset HDR_RECORD_COUNT_OFF = totalObj + totalClass
    ASSERT_EQ(ReadU32LE(Data(), HDR_RECORD_COUNT_OFF), RECORD_COUNT_OBJECTS + RECORD_COUNT_CLASSES);
}

// --- WriteHeapSummary ---

TEST_F(CommonWriterTest, WriteHeapSummary_Tag)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    Writer().WriteHeapSummary(DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT, SUMMARY_DYNAMIC_OBJECT_COUNT,
                              SUMMARY_STATIC_OBJECT_COUNT);
    FinalizeAndRead();

    RecordInfo summary = FindRecordAfterHeader(Data(), TAG_HEAP_SUMMARY);
    ASSERT_EQ(summary.tag, TAG_HEAP_SUMMARY);
}

TEST_F(CommonWriterTest, WriteHeapSummary_BodySize)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    Writer().WriteHeapSummary(DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT, SUMMARY_DYNAMIC_OBJECT_COUNT,
                              SUMMARY_STATIC_OBJECT_COUNT);
    FinalizeAndRead();

    RecordInfo summary = FindRecordAfterHeader(Data(), TAG_HEAP_SUMMARY);
    ASSERT_EQ(summary.bodySize, HEAP_SUMMARY_BODY_SIZE);
}

TEST_F(CommonWriterTest, WriteHeapSummary_FieldValues)
{
    Writer().WriteFileHeader(Language::DYNAMIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);
    Writer().WriteHeapSummary(DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT, FIELD_VALUES_DYNAMIC_OBJECT_COUNT, DUMP_CLASS_COUNT);
    FinalizeAndRead();

    RecordInfo summary = FindRecordAfterHeader(Data(), TAG_HEAP_SUMMARY);

    // Field layout (offsets from dump_format.h):
    //   HS_TOTAL_LIVE_BYTES_OFF totalLiveBytes
    //   HS_TOTAL_LIVE_INST_OFF  totalLiveInstances
    //   HS_TOTAL_ALLOC_OFF      totalAllocated
    //   HS_TOTAL_INST_ALLOC_OFF totalInstancesAllocated
    //   HS_STATIC_OBJ_OFF       staticObjCount
    //   HS_DYNAMIC_OBJ_OFF      dynamicObjCount
    //   HS_CLASS_COUNT_OFF      classCount
    ASSERT_EQ(ReadU64LE(Data(), summary.bodyStart + HS_TOTAL_LIVE_BYTES_OFF), 0U);
    ASSERT_EQ(ReadU64LE(Data(), summary.bodyStart + HS_TOTAL_LIVE_INST_OFF), static_cast<uint64_t>(DUMP_OBJECT_COUNT));
    ASSERT_EQ(ReadU64LE(Data(), summary.bodyStart + HS_TOTAL_ALLOC_OFF), 0U);
    ASSERT_EQ(ReadU64LE(Data(), summary.bodyStart + HS_TOTAL_INST_ALLOC_OFF), 0U);
    ASSERT_EQ(ReadU64LE(Data(), summary.bodyStart + HS_STATIC_OBJ_OFF), static_cast<uint64_t>(DUMP_CLASS_COUNT));
    ASSERT_EQ(ReadU64LE(Data(), summary.bodyStart + HS_DYNAMIC_OBJ_OFF),
              static_cast<uint64_t>(FIELD_VALUES_DYNAMIC_OBJECT_COUNT));
    ASSERT_EQ(ReadU64LE(Data(), summary.bodyStart + HS_CLASS_COUNT_OFF), static_cast<uint64_t>(DUMP_CLASS_COUNT));
}

// --- WriteXRefEdge (nodeId-based identifiers, both endpoints 4-byte nodeIds) ---

TEST_F(CommonWriterTest, WriteXRefEdge_Tag)
{
    Writer().WriteFileHeader(Language::DYNAMIC, 0, 0);
    Writer().BeginRecord(TAG_XREF_EDGE);
    Writer().WriteXRefEdge(0x00000001U, 0x00000002U, XREF_DIR_DYN_TO_STA);
    Writer().EndRecord();
    FinalizeAndRead();

    RecordInfo xref = FindRecordAfterHeader(Data(), TAG_XREF_EDGE);
    ASSERT_EQ(xref.tag, TAG_XREF_EDGE);
}

TEST_F(CommonWriterTest, WriteXRefEdge_BodyLayout)
{
    Writer().WriteFileHeader(Language::DYNAMIC, 0, 0);
    Writer().BeginRecord(TAG_XREF_EDGE);
    Writer().WriteXRefEdge(XREF_SOURCE_NODE_ID, XREF_TARGET_NODE_ID, XREF_DIR_STA_TO_DYN);
    Writer().EndRecord();
    FinalizeAndRead();

    RecordInfo xref = FindRecordAfterHeader(Data(), TAG_XREF_EDGE);

    // Body: fromNodeId(4) + toNodeId(4) + direction(1) = XREF_EDGE_BODY_SIZE bytes, count=1
    ASSERT_EQ(xref.bodySize, XREF_EDGE_BODY_SIZE);
    ASSERT_EQ(xref.count, 1U);
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + XREF_FROM_OFF), XREF_SOURCE_NODE_ID);
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + XREF_TO_OFF), XREF_TARGET_NODE_ID);
    ASSERT_EQ(Data().at(xref.bodyStart + XREF_DIR_OFF), XREF_DIR_STA_TO_DYN);
}

TEST_F(CommonWriterTest, WriteXRefEdge_AllDirections)
{
    Writer().WriteFileHeader(Language::DYNAMIC, 0, 0);
    Writer().BeginRecord(TAG_XREF_EDGE);
    Writer().WriteXRefEdge(1U, 2U, XREF_DIR_DYN_TO_STA);
    Writer().WriteXRefEdge(3U, 4U, XREF_DIR_STA_TO_DYN);
    Writer().WriteXRefEdge(5U, 6U, XREF_DIR_BIDIR);
    Writer().EndRecord();
    FinalizeAndRead();

    RecordInfo xref = FindRecordAfterHeader(Data(), TAG_XREF_EDGE);
    ASSERT_EQ(xref.tag, TAG_XREF_EDGE);
    // All three edges batched into one record, count=3, body=3*XREF_EDGE_BODY_SIZE bytes
    ASSERT_EQ(xref.count, static_cast<uint32_t>(BATCHED_XREF_EDGE_COUNT));
    ASSERT_EQ(xref.bodySize, BATCHED_XREF_EDGE_COUNT * XREF_EDGE_BODY_SIZE);

    // Item layout: each item is XREF_EDGE_BODY_SIZE bytes (fromNodeId + toNodeId + direction)
    constexpr size_t FIRST_EDGE_OFFSET = 0;
    constexpr size_t SECOND_EDGE_OFFSET = XREF_EDGE_BODY_SIZE;
    constexpr size_t THIRD_EDGE_OFFSET = 2U * XREF_EDGE_BODY_SIZE;
    // Item 0: fromNodeId=1, toNodeId=2, direction=DYN_TO_STA
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + FIRST_EDGE_OFFSET + XREF_FROM_OFF), 1U);
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + FIRST_EDGE_OFFSET + XREF_TO_OFF), 2U);
    ASSERT_EQ(Data().at(xref.bodyStart + FIRST_EDGE_OFFSET + XREF_DIR_OFF), XREF_DIR_DYN_TO_STA);
    // Item 1: fromNodeId=3, toNodeId=4, direction=STA_TO_DYN
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + SECOND_EDGE_OFFSET + XREF_FROM_OFF), 3U);
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + SECOND_EDGE_OFFSET + XREF_TO_OFF), 4U);
    ASSERT_EQ(Data().at(xref.bodyStart + SECOND_EDGE_OFFSET + XREF_DIR_OFF), XREF_DIR_STA_TO_DYN);
    // Item 2: fromNodeId=5, toNodeId=6, direction=BIDIR
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + THIRD_EDGE_OFFSET + XREF_FROM_OFF), 5U);
    ASSERT_EQ(ReadU32LE(Data(), xref.bodyStart + THIRD_EDGE_OFFSET + XREF_TO_OFF), 6U);
    ASSERT_EQ(Data().at(xref.bodyStart + THIRD_EDGE_OFFSET + XREF_DIR_OFF), XREF_DIR_BIDIR);
}

}  // namespace ark::tooling::hprof::test
