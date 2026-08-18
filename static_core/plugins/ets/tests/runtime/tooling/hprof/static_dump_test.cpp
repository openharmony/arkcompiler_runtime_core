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

/**
 * @brief Comprehensive test suite for the static dump subsystem.
 *
 * Three test suites, all in one file, linking the full runtime:
 *
 *   StaticDumpWriterTest      - Writer output byte-layout verification
 *   StaticDumpUtilTest        - Type mapping, tagged-value encoding, and name conversion
 *   StaticDumpIntegrationTest - End-to-end via the real translator:
 *                               Runtime -> DumpBinary -> .rawheap ->
 *                               RawHeap::TranslateRawheap -> .heapsnapshot graph.
 *                               No hand-written binary parser; assertions are
 *                               expressed at the translated heapsnapshot-graph level.
 *                               StaticDumpTest.ets declarations cover class
 *                               metadata, primitive/reference/tagged values,
 *                               arrays, inheritance, and reachability.
 *
 * ObjectIdMap and StringIdPool are tested in the ETS hprof session test suite
 * (plugins/ets/tests/runtime/tooling/hprof/session/object_id_map_test.cpp, string_id_pool_test.cpp).
 */

#include "plugins/ets/runtime/tooling/hprof/static_dump.h"
#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <fcntl.h>
#include <iterator>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unistd.h>

#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"
#include "plugins/ets/runtime/tooling/hprof/session/common_writer.h"
#include "plugins/ets/runtime/tooling/hprof/session/string_id_pool.h"
#include "runtime/include/coretypes/tagged_value.h"

// Integration test headers (only used in StaticDumpIntegrationTest)
#ifdef STATIC_DUMP_TEST_ABC_DIR
#include "plugins/ets/runtime/tooling/hprof/heap_dump_coordinator.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/class_helper.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/logger.h"
// End-to-end .rawheap -> .heapsnapshot validation via the real translator
// (linked as rawheap_translate_static; see BUILD.gn).
#include "rawheap_translate.h"
#include "cJSON.h"
#endif

namespace ark::tooling::hprof {

using common::dump::DumpRequest;
#ifdef STATIC_DUMP_TEST_ABC_DIR
using common::dump::DumpExecutionMode;
#endif

// ============================================================================
// Shared helpers: pipe-based output stream + binary readers
// ============================================================================

namespace {

// Test data constants
constexpr uint32_t TEST_CLASS_SERIAL = 42;
constexpr uint32_t TEST_CLASS_OBJ_ADDR = 0xAABBCC11U;
constexpr uint32_t TEST_CLASS_NAME_ID_VAL = 100;
constexpr uint32_t TEST_CLASS_FLAGS_VAL = 0x00000010U;
constexpr uint32_t DUMP_OBJECT_COUNT = 100U;
constexpr uint32_t DUMP_CLASS_COUNT = 10U;
constexpr uint32_t TEST_STATIC_OBJ_COUNT = 50;
constexpr uint32_t TEST_DYNAMIC_OBJ_COUNT = 50;
constexpr uint32_t TEST_INSTANCE_SIZE = 32;
constexpr uint32_t TEST_ARRAY_INSTANCE_SIZE = 40;
constexpr uint32_t TEST_SMALL_INSTANCE_SIZE = 24;
constexpr uint32_t TEST_ARRAY_LENGTH_OBJ = 3;
constexpr uint32_t TEST_ARRAY_LENGTH_INT = 5;
constexpr uint32_t DUMP_RECORD_COUNT = DUMP_OBJECT_COUNT + DUMP_CLASS_COUNT;
constexpr size_t PIPE_READ_BUFFER_SIZE = 4096U;
constexpr size_t STRING_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint32_t);  // stringId + strLen
constexpr std::string_view HELLO_STRING = "Hello";
constexpr std::string_view WORLD_STRING = "World";
constexpr uint32_t TEST_STATIC_CLASS_INST_SIZE = 24;
constexpr uint32_t TEST_INSTANCE_FIELD_COUNT = 3;
constexpr uint32_t TEST_METHOD_ID_COUNT = 2;
constexpr uint32_t TEST_TAGGED_VALUE_COUNT = 6;
constexpr size_t TEST_TAGGED_REFERENCE_VALUE_COUNT = 2;
constexpr uint32_t TEST_ROOT_OBJECT_ID = 0x11223344U;
constexpr uint32_t TEST_DUMP_CLASS_ID = 0xCAFEBABEU;
constexpr uint32_t TEST_SUPER_CLASS_ID = 0xDEADBEEFU;
constexpr uint32_t TEST_STATIC_FIELD_NAME_ID = 10U;
constexpr uint32_t TEST_SECOND_STATIC_FIELD_NAME_ID = 11U;
constexpr uint32_t TEST_INSTANCE_FIELD_NAME_ID = 20U;
constexpr uint32_t TEST_SECOND_INSTANCE_FIELD_NAME_ID = 21U;
constexpr uint32_t TEST_FIRST_FIELD_OFFSET = 8U;
constexpr uint32_t TEST_SECOND_FIELD_OFFSET = 16U;
constexpr uint32_t TEST_STATIC_FIELD_VALUE = 0x77U;
constexpr uint32_t TEST_FIRST_METHOD_ID = 30U;
constexpr uint32_t TEST_SECOND_METHOD_ID = 31U;
constexpr uint32_t TEST_INSTANCE_OBJECT_ID = 0x11111111U;
constexpr uint32_t TEST_INSTANCE_CLASS_ID = 0x22222222U;
constexpr uint32_t TEST_INT_FIELD_VALUE = 0x12345678U;
constexpr uint32_t TEST_ARRAY_ELEMENT_1 = 0xAAA1U;
constexpr uint32_t TEST_ARRAY_ELEMENT_2 = 0xAAA2U;
constexpr uint32_t TEST_ARRAY_ELEMENT_3 = 0xAAA3U;
constexpr uint32_t TEST_ARRAY_OBJECT_ID = 0xBBBBBBBBU;
constexpr uint32_t TEST_ARRAY_CLASS_ID = 0xCCCCCCCCU;
constexpr uint32_t TEST_PRIMITIVE_ARRAY_OBJECT_ID = 0xDDDDDDDDU;
constexpr uint32_t TEST_PRIMITIVE_ARRAY_CLASS_ID = 0xEEEEEEEEU;
constexpr std::array<int32_t, TEST_ARRAY_LENGTH_INT> TEST_PRIMITIVE_ARRAY_VALUES = {10, 20, 30, 40, 50};
constexpr uintptr_t TEST_LIVE_OBJECT_ADDRESS = 0x1000U;
constexpr uintptr_t TEST_UNREACHABLE_OBJECT_ADDRESS = 0x2000U;
constexpr double TEST_DOUBLE_VALUE = 2.5;
#ifdef STATIC_DUMP_TEST_ABC_DIR
// POSIX rw-r--r-- mode for dump files created by tests.
constexpr mode_t TEST_DUMP_FILE_PERMS = 0644;
// Regression guard: the static dump historically orphaned inherited-field-
// reachable objects; stdlib-internal disconnected components keep a residual
// orphan population, so the bound is a ceiling rather than zero.
constexpr int ORPHAN_REGRESSION_CEILING = 1500;
// Minimum string-typed nodes that must carry real UTF-8 content.
constexpr int MIN_STRING_CONTENT_NODES = 4;
#endif

class TestOutputStream {
public:
    TestOutputStream()
    {
        OpenPipe();
    }

    ~TestOutputStream()
    {
        stream_.reset();
        ClosePipe();
    }

    // Non-copyable, non-movable: owns an OutputStream and two pipe fds.
    // Declared to satisfy Rule-of-5 (user-declared dtor).
    TestOutputStream(const TestOutputStream &) = delete;
    TestOutputStream &operator=(const TestOutputStream &) = delete;
    TestOutputStream(TestOutputStream &&) = delete;
    TestOutputStream &operator=(TestOutputStream &&) = delete;

    OutputStream *Get()
    {
        return stream_.get();
    }

    /** Flush the writer, close the write end, then read all captured bytes. */
    const std::vector<uint8_t> &CapturedBytes()
    {
        if (!captured_) {
            stream_->Close();
            ::close(writeFd_);
            writeFd_ = -1;

            buf_.clear();
            std::array<uint8_t, PIPE_READ_BUFFER_SIZE> buffer {};
            ssize_t bytesRead = 0;
            while ((bytesRead = ::read(readFd_, buffer.data(), buffer.size())) > 0) {
                buf_.insert(buf_.end(), buffer.begin(), std::next(buffer.begin(), bytesRead));
            }
            ::close(readFd_);
            readFd_ = -1;
            captured_ = true;
        }
        return buf_;
    }

    void Reset()
    {
        stream_.reset();
        ClosePipe();
        OpenPipe();
        captured_ = false;
        buf_.clear();
    }

    size_t Size() const
    {
        return captured_ ? buf_.size() : 0;
    }

private:
    void OpenPipe()
    {
        std::array<int, 2U> fds {};
        pipe2(fds.data(), O_CLOEXEC);
        readFd_ = fds[0];
        writeFd_ = fds[1];
        stream_ = std::make_unique<OutputStream>(writeFd_);
    }

    void ClosePipe()
    {
        if (writeFd_ >= 0) {
            ::close(writeFd_);
            writeFd_ = -1;
        }
        if (readFd_ >= 0) {
            ::close(readFd_);
            readFd_ = -1;
        }
    }

    std::unique_ptr<OutputStream> stream_ {};
    int readFd_ = -1;
    int writeFd_ = -1;
    std::vector<uint8_t> buf_ {};
    bool captured_ = false;
};

template <typename T>
T ReadUnsignedLE(const std::vector<uint8_t> &data, size_t offset)
{
    static_assert(std::is_unsigned_v<T>);
    uint64_t value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
        value |= static_cast<uint64_t>(data.at(offset + i)) << (i * std::numeric_limits<uint8_t>::digits);
    }
    return static_cast<T>(value);
}

uint16_t ReadU16LE(const std::vector<uint8_t> &bytes, size_t offset)
{
    return ReadUnsignedLE<uint16_t>(bytes, offset);
}

uint32_t ReadU32LE(const std::vector<uint8_t> &bytes, size_t offset)
{
    return ReadUnsignedLE<uint32_t>(bytes, offset);
}

uint64_t ReadU64LE(const std::vector<uint8_t> &bytes, size_t offset)
{
    return ReadUnsignedLE<uint64_t>(bytes, offset);
}

/**
 * Parse all records from a raw byte buffer, returning per-tag bodies.
 * Each entry: (tag, bodyBytes, itemCount).
 * Skips the file header (first HYBRID_DUMP_FILE_HEADER_SIZE bytes).
 */
struct ParsedRecord {
    uint8_t tag = 0;
    uint32_t bodyLength = 0;
    uint32_t itemCount = 0;
    std::vector<uint8_t> body {};
};

std::vector<ParsedRecord> ParseRecords(const std::vector<uint8_t> &data, size_t headerSize = 0)
{
    std::vector<ParsedRecord> records;
    size_t offset = headerSize;
    while (offset + RECORD_HEADER_SIZE <= data.size()) {
        ParsedRecord rec {};
        rec.tag = data.at(offset + TAG_OFFSET);
        rec.bodyLength = ReadU32LE(data, offset + SIZE_OFFSET);
        rec.itemCount = ReadU32LE(data, offset + COUNT_OFFSET);
        size_t nextOffset = offset + RECORD_HEADER_SIZE + rec.bodyLength;
        if (nextOffset > data.size()) {
            break;
        }
        rec.body.assign(data.begin() + offset + RECORD_HEADER_SIZE,
                        data.begin() + offset + RECORD_HEADER_SIZE + rec.bodyLength);
        records.push_back(std::move(rec));
        offset = nextOffset;
    }
    return records;
}

}  // namespace

// ============================================================================
// StaticDumpWriterTest - Writer output byte-layout verification
// ============================================================================

class StaticDumpWriterTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        testStream_ = std::make_unique<TestOutputStream>();
        writer_ = std::make_unique<StaticWriter>(testStream_->Get());
    }

    void TearDown() override
    {
        writer_.reset();
        testStream_.reset();
    }

    const std::vector<uint8_t> &Bytes()
    {
        return testStream_->CapturedBytes();
    }
    void ResetStream()
    {
        testStream_->Reset();
    }

    uint8_t U8At(size_t off)
    {
        return Bytes().at(off);
    }
    uint16_t U16At(size_t off)
    {
        return ReadU16LE(Bytes(), off);
    }
    uint32_t U32At(size_t off)
    {
        return ReadU32LE(Bytes(), off);
    }
    uint64_t U64At(size_t off)
    {
        return ReadU64LE(Bytes(), off);
    }

    // Assert the fixed prefix of a CLASS_DUMP record body.
    void ExpectClassDumpFixedPrefix(size_t bodyOff, uint32_t classObj, uint32_t superCls, uint32_t instSize,
                                    uint16_t staticCount)
    {
        EXPECT_EQ(U32At(bodyOff + CD_CLASSOBJ_OFF), classObj);
        EXPECT_EQ(U32At(bodyOff + CD_STACKTRACE_OFF), 0U);
        EXPECT_EQ(U32At(bodyOff + CD_SUPERCLASS_OFF), superCls);
        EXPECT_EQ(U32At(bodyOff + CD_CLASSLOADER_OFF), 0U);  // classLoaderId = 0
        EXPECT_EQ(U32At(bodyOff + CD_INSTSIZE_OFF), instSize);
        EXPECT_EQ(U16At(bodyOff + CD_STATIC_COUNT_OFF), staticCount);
    }

    // Assert one field descriptor (nameId, type, offset, flags) at `off`.
    void ExpectFieldDescriptor(size_t off, uint32_t nameId, FieldType type, uint32_t fieldOff, FieldFlags flags)
    {
        EXPECT_EQ(U32At(off + FD_NAMEID_OFF), nameId);
        EXPECT_EQ(U8At(off + FD_TYPE_OFF), static_cast<uint8_t>(type));
        EXPECT_EQ(U32At(off + FD_OFFSET_OFF), fieldOff);
        EXPECT_EQ(U16At(off + FD_FLAGS_OFF), static_cast<uint16_t>(flags));
    }

    void ExpectClassDumpItemLayout(size_t expectedBody)
    {
        ASSERT_GE(Bytes().size(), RECORD_HEADER_SIZE + expectedBody);
        EXPECT_EQ(U32At(SIZE_OFFSET), expectedBody);

        size_t bodyOffset = RECORD_HEADER_SIZE;
        ExpectClassDumpFixedPrefix(bodyOffset, TEST_DUMP_CLASS_ID, TEST_SUPER_CLASS_ID, TEST_STATIC_CLASS_INST_SIZE,
                                   1U);

        size_t descriptorOffset = bodyOffset + CLASS_DUMP_FIXED_BODY_SIZE;
        ExpectFieldDescriptor(descriptorOffset, TEST_STATIC_FIELD_NAME_ID, FieldType::INT, 0U, FieldFlags::IS_STATIC);

        size_t instanceCountOffset = descriptorOffset + FIELD_DESCRIPTOR_SIZE;
        EXPECT_EQ(U16At(instanceCountOffset), 2U);

        size_t firstInstanceDescriptorOffset = instanceCountOffset + sizeof(uint16_t);
        ExpectFieldDescriptor(firstInstanceDescriptorOffset, TEST_INSTANCE_FIELD_NAME_ID, FieldType::OBJECT,
                              TEST_FIRST_FIELD_OFFSET, FieldFlags::IS_PUBLIC);

        size_t secondInstanceDescriptorOffset = firstInstanceDescriptorOffset + FIELD_DESCRIPTOR_SIZE;
        ExpectFieldDescriptor(secondInstanceDescriptorOffset, TEST_SECOND_INSTANCE_FIELD_NAME_ID, FieldType::INT,
                              TEST_SECOND_FIELD_OFFSET, FieldFlags::IS_PUBLIC);

        size_t staticValueCountOffset = secondInstanceDescriptorOffset + FIELD_DESCRIPTOR_SIZE;
        EXPECT_EQ(U16At(staticValueCountOffset), 1U);
        size_t staticValueOffset = staticValueCountOffset + sizeof(uint16_t);
        EXPECT_EQ(U8At(staticValueOffset), static_cast<uint8_t>(FieldType::INT));
        EXPECT_EQ(U32At(staticValueOffset + sizeof(uint8_t)), TEST_STATIC_FIELD_VALUE);

        size_t methodCountOffset = staticValueOffset + sizeof(uint8_t) + sizeof(uint32_t);
        EXPECT_EQ(U16At(methodCountOffset), 2U);
        size_t methodIdOffset = methodCountOffset + sizeof(uint16_t);
        EXPECT_EQ(U32At(methodIdOffset), TEST_FIRST_METHOD_ID);
        EXPECT_EQ(U32At(methodIdOffset + sizeof(uint32_t)), TEST_SECOND_METHOD_ID);
    }

    StaticWriter &Writer()
    {
        return *writer_;
    }

private:
    std::unique_ptr<TestOutputStream> testStream_;
    std::unique_ptr<StaticWriter> writer_;
};

// Test-only access to StaticDump's output ownership. Production output is
// acquired internally by StaticDump through FaultLogger; integration tests
// inject a regular file so the translator can inspect the generated rawheap.
class StaticDumpTest {
public:
    static void SetOutput(StaticDump *dump, int fd)
    {
        dump->staticStream_ = new OutputStream(fd, OutputStream::DEFAULT_BUFFER_SIZE, true);
        dump->writer_ = new StaticWriter(dump->staticStream_);
    }

    static FieldValueData EncodeTaggedValue(StaticDump *dump, coretypes::TaggedValue value)
    {
        return dump->EncodeTaggedValue(value);
    }
};

TEST_F(StaticDumpWriterTest, WriteRootItem_ByteLayout)
{
    Writer().BeginRecord(TAG_ROOT_RECORD);
    Writer().WriteRootItem(TEST_ROOT_OBJECT_ID);
    Writer().EndRecord();

    auto bytes = Bytes();
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE + ROOT_RECORD_BODY_SIZE);

    // Record header
    EXPECT_EQ(U8At(TAG_OFFSET), TAG_ROOT_RECORD);
    EXPECT_NE(U64At(TIMESTAMP_OFFSET), 0ULL);  // auto-generated, nonzero
    EXPECT_EQ(U32At(SIZE_OFFSET), ROOT_RECORD_BODY_SIZE);
    EXPECT_EQ(U32At(COUNT_OFFSET), 1U);

    // Body: rootType(1) + objectId(4)
    size_t bodyOff = RECORD_HEADER_SIZE;
    EXPECT_EQ(U8At(bodyOff + ROOT_TYPE_OFF), static_cast<uint8_t>(RootType::STATIC_OBJECT));
    EXPECT_EQ(U32At(bodyOff + ROOT_OBJADDR_OFF), TEST_ROOT_OBJECT_ID);
}

TEST_F(StaticDumpWriterTest, WriteLoadClassItem_ByteLayout)
{
    Writer().BeginRecord(TAG_LOAD_CLASS);
    Writer().WriteLoadClassItem(TEST_CLASS_SERIAL, TEST_CLASS_OBJ_ADDR, TEST_CLASS_NAME_ID_VAL, TEST_CLASS_FLAGS_VAL);
    Writer().EndRecord();

    auto bytes = Bytes();
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE + LOAD_CLASS_BODY_SIZE);

    // Record header
    EXPECT_EQ(U8At(TAG_OFFSET), TAG_LOAD_CLASS);
    EXPECT_EQ(U32At(SIZE_OFFSET), LOAD_CLASS_BODY_SIZE);
    EXPECT_EQ(U32At(COUNT_OFFSET), 1U);

    // Body: classSerial(4) + classObjectId(4) + stackTraceSerial(4) +
    //        classNameId(4) + language(1) + classFlags(4)
    size_t bodyOff = RECORD_HEADER_SIZE;
    EXPECT_EQ(U32At(bodyOff + LC_SERIAL_OFF), TEST_CLASS_SERIAL);
    EXPECT_EQ(U32At(bodyOff + LC_CLASSOBJ_OFF), TEST_CLASS_OBJ_ADDR);
    EXPECT_EQ(U32At(bodyOff + LC_STACKTRACE_OFF), 0U);  // stackTraceSerial = 0
    EXPECT_EQ(U32At(bodyOff + LC_NAMEID_OFF), TEST_CLASS_NAME_ID_VAL);
    EXPECT_EQ(U8At(bodyOff + LC_LANGUAGE_OFF), static_cast<uint8_t>(Language::STATIC));
    EXPECT_EQ(U32At(bodyOff + LC_FLAGS_OFF), TEST_CLASS_FLAGS_VAL);
}

TEST_F(StaticDumpWriterTest, WriteClassDumpItem_ByteLayout)
{
    const std::array<ClassFieldData, 1U> staticFields = {{
        {TEST_STATIC_FIELD_NAME_ID, static_cast<uint8_t>(FieldType::INT), 0,
         static_cast<uint16_t>(FieldFlags::IS_STATIC)},
    }};
    constexpr size_t LOCAL_INSTANCE_FIELD_COUNT = 2;
    const std::array<ClassFieldData, LOCAL_INSTANCE_FIELD_COUNT> instanceFields = {{
        {TEST_INSTANCE_FIELD_NAME_ID, static_cast<uint8_t>(FieldType::OBJECT), TEST_FIRST_FIELD_OFFSET,
         static_cast<uint16_t>(FieldFlags::IS_PUBLIC)},
        {TEST_SECOND_INSTANCE_FIELD_NAME_ID, static_cast<uint8_t>(FieldType::INT), TEST_SECOND_FIELD_OFFSET,
         static_cast<uint16_t>(FieldFlags::IS_PUBLIC)},
    }};
    // One static field value (INT), parallel to the single static field descriptor.
    const std::array<FieldValueData, 1U> staticValues = {{
        {static_cast<uint8_t>(FieldType::INT), TEST_STATIC_FIELD_VALUE},
    }};
    // Two declared method-name ids.
    const std::array<uint32_t, TEST_METHOD_ID_COUNT> methodIds = {TEST_FIRST_METHOD_ID, TEST_SECOND_METHOD_ID};

    Writer().BeginRecord(TAG_STATIC_CLASS_DUMP);
    Writer().WriteClassDumpItem(TEST_DUMP_CLASS_ID, TEST_SUPER_CLASS_ID, TEST_STATIC_CLASS_INST_SIZE,
                                staticFields.data(), static_cast<uint16_t>(staticFields.size()), instanceFields.data(),
                                static_cast<uint16_t>(instanceFields.size()), staticValues.data(),
                                static_cast<uint16_t>(staticValues.size()), methodIds.data(),
                                static_cast<uint16_t>(methodIds.size()));
    Writer().EndRecord();

    // Fixed prefix = 22, static descriptors = 1*11 = 11,
    // instance count(2) + descriptors = 2*11 = 22,
    // staticValueCount(2) + 1 INT value [type(1)+value(4)] = 7,
    // methodCount(2) + 2 method ids (4 each) = 10.
    // Total body = 22 + 11 + 2 + 22 + 2 + 5 + 2 + 8 = 74
    size_t staticValueSection = sizeof(uint16_t) + (sizeof(uint8_t) + sizeof(uint32_t));  // 2 + 5 = 7
    size_t methodSection = sizeof(uint16_t) + 2 * sizeof(uint32_t);                       // 2 + 8 = 10
    size_t expectedBody = CLASS_DUMP_FIXED_BODY_SIZE + FIELD_DESCRIPTOR_SIZE + FIELD_DESCRIPTOR_SIZE +
                          sizeof(uint16_t) + FIELD_DESCRIPTOR_SIZE + staticValueSection + methodSection;
    ExpectClassDumpItemLayout(expectedBody);
}

// A class record with methodCount=0 must omit the method section entirely.
// A class with no declared methods should omit the method-count/method-ids section.
TEST_F(StaticDumpWriterTest, WriteClassDumpItem_MethodCountZero_NoMethodSection)
{
    const std::array<ClassFieldData, 1U> staticFields = {{
        {TEST_STATIC_FIELD_NAME_ID, static_cast<uint8_t>(FieldType::INT), 0,
         static_cast<uint16_t>(FieldFlags::IS_STATIC)},
    }};
    const std::array<ClassFieldData, 1U> instanceFields = {{
        {TEST_INSTANCE_FIELD_NAME_ID, static_cast<uint8_t>(FieldType::OBJECT), TEST_FIRST_FIELD_OFFSET,
         static_cast<uint16_t>(FieldFlags::IS_PUBLIC)},
    }};
    const std::array<FieldValueData, 1U> staticValues = {{
        {static_cast<uint8_t>(FieldType::INT), TEST_STATIC_FIELD_VALUE},
    }};

    Writer().BeginRecord(TAG_STATIC_CLASS_DUMP);
    Writer().WriteClassDumpItem(TEST_DUMP_CLASS_ID, TEST_SUPER_CLASS_ID, TEST_STATIC_CLASS_INST_SIZE,
                                staticFields.data(), static_cast<uint16_t>(staticFields.size()), instanceFields.data(),
                                static_cast<uint16_t>(instanceFields.size()), staticValues.data(),
                                static_cast<uint16_t>(staticValues.size()), nullptr, 0);
    Writer().EndRecord();

    auto bytes = Bytes();
    // Body = fixed(22) + staticDesc(11) + instCount(2) + instDesc(11) +
    //        staticValCount(2) + staticVal(5) + methodCount(2)
    // methodCount is written even if 0, but no method ids follow
    size_t expectedBody = CLASS_DUMP_FIXED_BODY_SIZE + FIELD_DESCRIPTOR_SIZE + sizeof(uint16_t) +
                          FIELD_DESCRIPTOR_SIZE + sizeof(uint16_t) + (sizeof(uint8_t) + sizeof(uint32_t)) +
                          sizeof(uint16_t);  // methodCount only, no ids
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE + expectedBody);
    EXPECT_EQ(U32At(SIZE_OFFSET), expectedBody);

    // Verify methodCount field is 0
    size_t bodyOff = RECORD_HEADER_SIZE;
    size_t staticValCountOff =
        bodyOff + CLASS_DUMP_FIXED_BODY_SIZE + FIELD_DESCRIPTOR_SIZE + sizeof(uint16_t) + FIELD_DESCRIPTOR_SIZE;
    size_t staticValOff = staticValCountOff + sizeof(uint16_t);
    size_t methodCountOff = staticValOff + sizeof(uint8_t) + sizeof(uint32_t);
    EXPECT_EQ(U16At(methodCountOff), 0U);  // methodCount = 0
}

// A class may have static fields but not all have values (e.g., uninitialized statics).
TEST_F(StaticDumpWriterTest, WriteClassDumpItem_StaticValueCountLessThanStaticCount)
{
    // 2 static field descriptors, but only 1 has a value
    const std::array<ClassFieldData, 2U> staticFields = {{
        {TEST_STATIC_FIELD_NAME_ID, static_cast<uint8_t>(FieldType::INT), 0,
         static_cast<uint16_t>(FieldFlags::IS_STATIC)},
        {TEST_SECOND_STATIC_FIELD_NAME_ID, static_cast<uint8_t>(FieldType::OBJECT), sizeof(uint32_t),
         static_cast<uint16_t>(FieldFlags::IS_STATIC)},
    }};
    const std::array<FieldValueData, 1U> staticValues = {{
        {static_cast<uint8_t>(FieldType::INT), TEST_STATIC_FIELD_VALUE},
    }};
    const std::array<uint32_t, 1U> methodIds = {TEST_FIRST_METHOD_ID};

    Writer().BeginRecord(TAG_STATIC_CLASS_DUMP);
    Writer().WriteClassDumpItem(TEST_DUMP_CLASS_ID, TEST_SUPER_CLASS_ID, TEST_STATIC_CLASS_INST_SIZE,
                                staticFields.data(), static_cast<uint16_t>(staticFields.size()), nullptr, 0,
                                staticValues.data(), static_cast<uint16_t>(staticValues.size()), methodIds.data(),
                                static_cast<uint16_t>(methodIds.size()));
    Writer().EndRecord();

    auto bytes = Bytes();
    // Body = fixed(22) + staticDesc(2*11=22) + instCount(2) + staticValCount(2) +
    //        staticVal(5) + methodCount(2) + methodId(4)
    size_t expectedBody = CLASS_DUMP_FIXED_BODY_SIZE + 2 * FIELD_DESCRIPTOR_SIZE +
                          sizeof(uint16_t) +                                         // instance field count (0)
                          sizeof(uint16_t) + (sizeof(uint8_t) + sizeof(uint32_t)) +  // static value section
                          sizeof(uint16_t) + sizeof(uint32_t);                       // method section
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE + expectedBody);

    // Verify staticCount=2 in header, staticValueCount=1 in value section
    size_t bodyOff = RECORD_HEADER_SIZE;
    EXPECT_EQ(U16At(bodyOff + CD_STATIC_COUNT_OFF), 2U);  // static field descriptors count

    size_t staticValCountOff = bodyOff + CLASS_DUMP_FIXED_BODY_SIZE + 2 * FIELD_DESCRIPTOR_SIZE + sizeof(uint16_t);
    EXPECT_EQ(U16At(staticValCountOff), 1U);  // staticValueCount = 1
}

TEST_F(StaticDumpWriterTest, WriteInstanceDumpItem_ByteLayout)
{
    // BOOLEAN(1 byte) + INT(4 bytes) + OBJECT(4 bytes)
    const std::array<FieldValueData, TEST_INSTANCE_FIELD_COUNT> fields = {{
        {static_cast<uint8_t>(FieldType::BOOLEAN), 1},
        {static_cast<uint8_t>(FieldType::INT), TEST_INT_FIELD_VALUE},
        {static_cast<uint8_t>(FieldType::OBJECT), TEST_CLASS_OBJ_ADDR},
    }};

    Writer().BeginRecord(TAG_STATIC_INSTANCE_DUMP);
    Writer().WriteInstanceDumpItem(TEST_INSTANCE_OBJECT_ID, TEST_INSTANCE_CLASS_ID, TEST_INSTANCE_SIZE, fields.data(),
                                   static_cast<uint16_t>(fields.size()));
    Writer().EndRecord();

    auto bytes = Bytes();
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE);

    size_t bodyOff = RECORD_HEADER_SIZE;
    // Fixed prefix: objectId(4) + classObjectId(4) + stackTrace(4)
    // + instanceSize(4) + fieldCount(2) = 18
    EXPECT_EQ(U32At(bodyOff + ID_OBJADDR_OFF), TEST_INSTANCE_OBJECT_ID);
    EXPECT_EQ(U32At(bodyOff + ID_CLASSOBJ_OFF), TEST_INSTANCE_CLASS_ID);
    EXPECT_EQ(U32At(bodyOff + ID_STACKTRACE_OFF), 0U);  // stackTraceSerial
    EXPECT_EQ(U32At(bodyOff + ID_INSTSIZE_OFF), TEST_INSTANCE_SIZE);
    EXPECT_EQ(U16At(bodyOff + ID_FIELD_COUNT_OFF), TEST_INSTANCE_FIELD_COUNT);

    // Field values start at bodyOff + 18
    size_t valOff = bodyOff + INSTANCE_DUMP_FIXED_BODY_SIZE;
    // BOOLEAN: type(1) + value(1)
    EXPECT_EQ(U8At(valOff), static_cast<uint8_t>(FieldType::BOOLEAN));
    EXPECT_EQ(U8At(valOff + 1), 1U);
    valOff += sizeof(uint8_t) + sizeof(uint8_t);

    // INT: type(1) + value(4)
    EXPECT_EQ(U8At(valOff), static_cast<uint8_t>(FieldType::INT));
    EXPECT_EQ(U32At(valOff + 1U), TEST_INT_FIELD_VALUE);
    valOff += sizeof(uint8_t) + sizeof(uint32_t);

    // OBJECT: type(1) + value(4)
    EXPECT_EQ(U8At(valOff), static_cast<uint8_t>(FieldType::OBJECT));
    EXPECT_EQ(U32At(valOff + 1U), TEST_CLASS_OBJ_ADDR);
}

// An instance record with fieldCount=0 must emit an empty instance body.
// An instance with no fields (e.g., a marker class) should emit only the fixed prefix.
TEST_F(StaticDumpWriterTest, WriteInstanceDumpItem_FieldCountZero_EmptyBody)
{
    Writer().BeginRecord(TAG_STATIC_INSTANCE_DUMP);
    Writer().WriteInstanceDumpItem(TEST_INSTANCE_OBJECT_ID, TEST_INSTANCE_CLASS_ID, TEST_INSTANCE_SIZE, nullptr,
                                   0);  // fieldCount=0, fields=nullptr
    Writer().EndRecord();

    auto bytes = Bytes();
    // Body = fixed prefix only (18 bytes)
    size_t expectedBody = INSTANCE_DUMP_FIXED_BODY_SIZE;
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE + expectedBody);
    EXPECT_EQ(U32At(SIZE_OFFSET), expectedBody);

    size_t bodyOff = RECORD_HEADER_SIZE;
    EXPECT_EQ(U32At(bodyOff + ID_OBJADDR_OFF), TEST_INSTANCE_OBJECT_ID);
    EXPECT_EQ(U32At(bodyOff + ID_CLASSOBJ_OFF), TEST_INSTANCE_CLASS_ID);
    EXPECT_EQ(U32At(bodyOff + ID_INSTSIZE_OFF), TEST_INSTANCE_SIZE);
    EXPECT_EQ(U16At(bodyOff + ID_FIELD_COUNT_OFF), 0U);  // fieldCount = 0
}

TEST_F(StaticDumpWriterTest, WriteArrayDumpItem_RefElement)
{
    const std::array<uint32_t, TEST_ARRAY_LENGTH_OBJ> elements = {TEST_ARRAY_ELEMENT_1, TEST_ARRAY_ELEMENT_2,
                                                                  TEST_ARRAY_ELEMENT_3};

    Writer().BeginRecord(TAG_STATIC_ARRAY_DUMP);
    Writer().WriteArrayDumpItem(TEST_ARRAY_OBJECT_ID, TEST_ARRAY_CLASS_ID, TEST_ARRAY_INSTANCE_SIZE,
                                TEST_ARRAY_LENGTH_OBJ, static_cast<uint8_t>(FieldType::OBJECT),
                                reinterpret_cast<const uint8_t *>(elements.data()),
                                elements.size() * sizeof(elements.front()));
    Writer().EndRecord();

    auto bytes = Bytes();
    ASSERT_GE(bytes.size(),
              RECORD_HEADER_SIZE + ARRAY_INSTANCE_FIXED_BODY_SIZE + TEST_ARRAY_LENGTH_OBJ * STATIC_OBJECT_ID_SIZE);

    size_t bodyOff = RECORD_HEADER_SIZE;
    // Fixed prefix: objectId(4) + classObjectId(4) + stackTrace(4) +
    //               instanceSize(4) + arrayLen(4) + elementType(1) = 21
    EXPECT_EQ(U32At(bodyOff + AD_OBJADDR_OFF), TEST_ARRAY_OBJECT_ID);
    EXPECT_EQ(U32At(bodyOff + AD_CLASSOBJ_OFF), TEST_ARRAY_CLASS_ID);
    EXPECT_EQ(U32At(bodyOff + AD_STACKTRACE_OFF), 0U);
    EXPECT_EQ(U32At(bodyOff + AD_INSTSIZE_OFF), TEST_ARRAY_INSTANCE_SIZE);
    EXPECT_EQ(U32At(bodyOff + AD_ARRAY_LENGTH_OFF), TEST_ARRAY_LENGTH_OBJ);
    EXPECT_EQ(U8At(bodyOff + AD_ELEM_TYPE_OFF), static_cast<uint8_t>(FieldType::OBJECT));

    // Elements start at bodyOff + 21 (OBJECT elements are u32 nodeIds)
    size_t elemOff = bodyOff + ARRAY_INSTANCE_FIXED_BODY_SIZE;
    EXPECT_EQ(U32At(elemOff), TEST_ARRAY_ELEMENT_1);
    EXPECT_EQ(U32At(elemOff + STATIC_OBJECT_ID_SIZE), TEST_ARRAY_ELEMENT_2);
    EXPECT_EQ(U32At(elemOff + 2U * STATIC_OBJECT_ID_SIZE), TEST_ARRAY_ELEMENT_3);
}

TEST_F(StaticDumpWriterTest, WriteArrayDumpItem_PrimitiveElement)
{
    Writer().BeginRecord(TAG_STATIC_ARRAY_DUMP);
    Writer().WriteArrayDumpItem(TEST_PRIMITIVE_ARRAY_OBJECT_ID, TEST_PRIMITIVE_ARRAY_CLASS_ID, TEST_SMALL_INSTANCE_SIZE,
                                TEST_ARRAY_LENGTH_INT, static_cast<uint8_t>(FieldType::INT),
                                reinterpret_cast<const uint8_t *>(TEST_PRIMITIVE_ARRAY_VALUES.data()),
                                TEST_PRIMITIVE_ARRAY_VALUES.size() * sizeof(TEST_PRIMITIVE_ARRAY_VALUES.front()));
    Writer().EndRecord();

    auto bytes = Bytes();
    ASSERT_GE(bytes.size(),
              RECORD_HEADER_SIZE + ARRAY_INSTANCE_FIXED_BODY_SIZE + TEST_ARRAY_LENGTH_INT * sizeof(uint32_t));

    size_t bodyOff = RECORD_HEADER_SIZE;
    EXPECT_EQ(U8At(TAG_OFFSET), TAG_STATIC_ARRAY_DUMP);
    EXPECT_EQ(U32At(SIZE_OFFSET), ARRAY_INSTANCE_FIXED_BODY_SIZE + TEST_ARRAY_LENGTH_INT * sizeof(uint32_t));
    EXPECT_EQ(U32At(COUNT_OFFSET), 1U);

    EXPECT_EQ(U32At(bodyOff + AD_OBJADDR_OFF), TEST_PRIMITIVE_ARRAY_OBJECT_ID);
    EXPECT_EQ(U32At(bodyOff + AD_ARRAY_LENGTH_OFF), TEST_ARRAY_LENGTH_INT);
    EXPECT_EQ(U8At(bodyOff + AD_ELEM_TYPE_OFF), static_cast<uint8_t>(FieldType::INT));

    // Raw data starts at bodyOff + 21
    size_t dataOff = bodyOff + ARRAY_INSTANCE_FIXED_BODY_SIZE;
    for (size_t i = 0; i < TEST_PRIMITIVE_ARRAY_VALUES.size(); i++) {
        EXPECT_EQ(U32At(dataOff + i * sizeof(uint32_t)), static_cast<uint32_t>(TEST_PRIMITIVE_ARRAY_VALUES.at(i)));
    }
}

TEST_F(StaticDumpWriterTest, WriteTaggedArrayDumpItem_RuntimeTypedElements)
{
    static constexpr uint64_t DOUBLE_TWO_POINT_FIVE_BITS = 0x4004000000000000ULL;
    static constexpr uint64_t TAGGED_UNDEFINED_RAW = 0x0AULL;
    const std::array<FieldValueData, TEST_TAGGED_VALUE_COUNT> elements = {{
        {static_cast<uint8_t>(FieldType::INT), 0xFFFFFFF9U},
        {static_cast<uint8_t>(FieldType::DOUBLE), DOUBLE_TWO_POINT_FIVE_BITS},
        {static_cast<uint8_t>(FieldType::BOOLEAN), 1U},
        {static_cast<uint8_t>(FieldType::OBJECT), TEST_ARRAY_ELEMENT_1},
        {static_cast<uint8_t>(FieldType::WEAK_OBJECT), TEST_ARRAY_ELEMENT_2},
        {static_cast<uint8_t>(FieldType::TAGGED), TAGGED_UNDEFINED_RAW},
    }};

    Writer().BeginRecord(TAG_STATIC_ARRAY_DUMP);
    Writer().WriteTaggedArrayDumpItem(TEST_ARRAY_OBJECT_ID, TEST_ARRAY_CLASS_ID, TEST_ARRAY_INSTANCE_SIZE,
                                      elements.data(), static_cast<uint32_t>(elements.size()));
    Writer().EndRecord();

    auto bytes = Bytes();
    size_t taggedValuesSize = (sizeof(uint8_t) + sizeof(uint32_t)) + (sizeof(uint8_t) + sizeof(uint64_t)) +
                              (sizeof(uint8_t) + sizeof(uint8_t)) +
                              TEST_TAGGED_REFERENCE_VALUE_COUNT * (sizeof(uint8_t) + sizeof(uint32_t)) +
                              (sizeof(uint8_t) + sizeof(uint64_t));
    EXPECT_EQ(U32At(SIZE_OFFSET), ARRAY_INSTANCE_FIXED_BODY_SIZE + taggedValuesSize);
    EXPECT_EQ(U32At(COUNT_OFFSET), 1U);
    size_t offset = RECORD_HEADER_SIZE;
    EXPECT_EQ(U32At(offset + AD_ARRAY_LENGTH_OFF), elements.size());
    EXPECT_EQ(U8At(offset + AD_ELEM_TYPE_OFF), static_cast<uint8_t>(FieldType::TAGGED));

    offset += ARRAY_INSTANCE_FIXED_BODY_SIZE;
    EXPECT_EQ(U8At(offset), static_cast<uint8_t>(FieldType::INT));
    EXPECT_EQ(U32At(offset + sizeof(uint8_t)), 0xFFFFFFF9U);
    offset += sizeof(uint8_t) + sizeof(uint32_t);

    EXPECT_EQ(U8At(offset), static_cast<uint8_t>(FieldType::DOUBLE));
    EXPECT_EQ(U64At(offset + sizeof(uint8_t)), DOUBLE_TWO_POINT_FIVE_BITS);
    offset += sizeof(uint8_t) + sizeof(uint64_t);

    EXPECT_EQ(U8At(offset), static_cast<uint8_t>(FieldType::BOOLEAN));
    EXPECT_EQ(U8At(offset + sizeof(uint8_t)), 1U);
    offset += sizeof(uint8_t) + sizeof(uint8_t);

    EXPECT_EQ(U8At(offset), static_cast<uint8_t>(FieldType::OBJECT));
    EXPECT_EQ(U32At(offset + sizeof(uint8_t)), TEST_ARRAY_ELEMENT_1);
    offset += sizeof(uint8_t) + sizeof(uint32_t);

    EXPECT_EQ(U8At(offset), static_cast<uint8_t>(FieldType::WEAK_OBJECT));
    EXPECT_EQ(U32At(offset + sizeof(uint8_t)), TEST_ARRAY_ELEMENT_2);
    offset += sizeof(uint8_t) + sizeof(uint32_t);

    EXPECT_EQ(U8At(offset), static_cast<uint8_t>(FieldType::TAGGED));
    EXPECT_EQ(U64At(offset + sizeof(uint8_t)), TAGGED_UNDEFINED_RAW);
}

// An array with unrecognized element type should emit the type byte but skip element payload.
TEST_F(StaticDumpWriterTest, WriteArrayDumpItem_ElementTypeUnknown_NoElementData)
{
    Writer().BeginRecord(TAG_STATIC_ARRAY_DUMP);
    Writer().WriteArrayDumpItem(TEST_PRIMITIVE_ARRAY_OBJECT_ID, TEST_PRIMITIVE_ARRAY_CLASS_ID, TEST_SMALL_INSTANCE_SIZE,
                                0,  // arrayLength=0 for UNKNOWN type
                                static_cast<uint8_t>(FieldType::UNKNOWN), nullptr, 0);  // no element data for UNKNOWN
    Writer().EndRecord();

    auto bytes = Bytes();
    // Body = fixed prefix (21) with elementType=UNKNOWN, no element payload
    size_t expectedBody = ARRAY_INSTANCE_FIXED_BODY_SIZE;
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE + expectedBody);
    EXPECT_EQ(U32At(SIZE_OFFSET), expectedBody);

    size_t bodyOff = RECORD_HEADER_SIZE;
    EXPECT_EQ(U32At(bodyOff + AD_OBJADDR_OFF), TEST_PRIMITIVE_ARRAY_OBJECT_ID);
    EXPECT_EQ(U32At(bodyOff + AD_CLASSOBJ_OFF), TEST_PRIMITIVE_ARRAY_CLASS_ID);
    EXPECT_EQ(U32At(bodyOff + AD_ARRAY_LENGTH_OFF), 0U);  // array length = 0
    EXPECT_EQ(U8At(bodyOff + AD_ELEM_TYPE_OFF), static_cast<uint8_t>(FieldType::UNKNOWN));
}

TEST_F(StaticDumpWriterTest, WriteFileHeader_ByteLayout)
{
    TestOutputStream ts;
    CommonWriter cw(ts.Get());
    cw.WriteFileHeader(Language::STATIC, DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT);

    auto bytes = ts.CapturedBytes();
    ASSERT_GE(bytes.size(), HYBRID_DUMP_FILE_HEADER_SIZE);

    // version (8 bytes)
    EXPECT_EQ(std::memcmp(bytes.data(), HYBRID_DUMP_VERSION.data(), HYBRID_DUMP_VERSION_SIZE), 0);
    // identifierSize (offset 8) - static-side nodeIds are 4 bytes
    EXPECT_EQ(ReadU32LE(bytes, HDR_IDENTIFIER_SIZE_OFF), STATIC_OBJECT_ID_SIZE);
    // timestamp (offset 12) - nonzero, auto-generated
    EXPECT_NE(ReadU64LE(bytes, HDR_TIMESTAMP_OFF), 0ULL);
    // language (offset 20)
    EXPECT_EQ(bytes.at(HDR_LANGUAGE_OFF), static_cast<uint8_t>(Language::STATIC));
    // headerSize (offset 21)
    EXPECT_EQ(ReadU32LE(bytes, HDR_HEADER_SIZE_OFF), HYBRID_DUMP_HEADER_SIZE);
    // recordCount (offset 25)
    EXPECT_EQ(ReadU32LE(bytes, HDR_RECORD_COUNT_OFF), DUMP_RECORD_COUNT);
    // featureFlags (offset 29)
    EXPECT_EQ(ReadU32LE(bytes, HDR_FEATURE_FLAGS_OFF), HYBRID_DUMP_FEATURE_FLAGS);
}

TEST_F(StaticDumpWriterTest, WriteStringPool_ByteLayout)
{
    StringIdPool pool;
    pool.AddString(HELLO_STRING.data());
    pool.AddString(WORLD_STRING.data());
    pool.Freeze();

    TestOutputStream ts;
    CommonWriter cw(ts.Get());
    cw.WriteStringPool(&pool);

    auto bytes = ts.CapturedBytes();
    auto records = ParseRecords(bytes);
    ASSERT_GE(records.size(), 1U);

    auto &rec = records.at(0);
    EXPECT_EQ(rec.tag, TAG_STRING_IN_UTF8);
    EXPECT_EQ(rec.itemCount, 2U);

    // First string item: stringId(4) + strLen(4) + utf8Data(5)
    size_t pos = 0;
    EXPECT_EQ(ReadU32LE(rec.body, pos), 0U);  // first string gets id 0
    EXPECT_EQ(ReadU32LE(rec.body, pos + sizeof(uint32_t)), HELLO_STRING.size());
    auto helloStart = std::next(rec.body.cbegin(), static_cast<std::ptrdiff_t>(pos + STRING_HEADER_SIZE));
    EXPECT_TRUE(std::equal(HELLO_STRING.cbegin(), HELLO_STRING.cend(), helloStart));
    pos += STRING_HEADER_SIZE + HELLO_STRING.size();

    // Second string item
    EXPECT_EQ(ReadU32LE(rec.body, pos), 1U);
    EXPECT_EQ(ReadU32LE(rec.body, pos + sizeof(uint32_t)), WORLD_STRING.size());
    auto worldStart = std::next(rec.body.cbegin(), static_cast<std::ptrdiff_t>(pos + STRING_HEADER_SIZE));
    EXPECT_TRUE(std::equal(WORLD_STRING.cbegin(), WORLD_STRING.cend(), worldStart));
}

TEST_F(StaticDumpWriterTest, WriteHeapSummary_ByteLayout)
{
    TestOutputStream ts;
    CommonWriter cw(ts.Get());
    cw.WriteHeapSummary(DUMP_OBJECT_COUNT, DUMP_CLASS_COUNT, TEST_STATIC_OBJ_COUNT, TEST_DYNAMIC_OBJ_COUNT);

    auto bytes = ts.CapturedBytes();
    auto records = ParseRecords(bytes);
    ASSERT_GE(records.size(), 1U);

    auto &rec = records.at(0);
    EXPECT_EQ(rec.tag, TAG_HEAP_SUMMARY);
    EXPECT_EQ(rec.itemCount, 1U);
    EXPECT_EQ(rec.bodyLength, HEAP_SUMMARY_BODY_SIZE);

    // 7 u64 fields: totalLiveBytes, totalLiveInstances, totalAllocated,
    // totalInstancesAllocated, staticObjCount, dynamicObjCount, classCount
    size_t pos = 0;
    EXPECT_EQ(ReadU64LE(rec.body, pos), 0ULL);  // totalLiveBytes (reserved)
    pos += HYBRID_DUMP_IDENTIFIER_SIZE;
    EXPECT_EQ(ReadU64LE(rec.body, pos), DUMP_OBJECT_COUNT);  // totalLiveInstances
    pos += HYBRID_DUMP_IDENTIFIER_SIZE;
    EXPECT_EQ(ReadU64LE(rec.body, pos), 0ULL);  // totalAllocated (reserved)
    pos += HYBRID_DUMP_IDENTIFIER_SIZE;
    EXPECT_EQ(ReadU64LE(rec.body, pos), 0ULL);  // totalInstancesAllocated (reserved)
    pos += HYBRID_DUMP_IDENTIFIER_SIZE;
    EXPECT_EQ(ReadU64LE(rec.body, pos), TEST_STATIC_OBJ_COUNT);  // staticObjCount
    pos += HYBRID_DUMP_IDENTIFIER_SIZE;
    EXPECT_EQ(ReadU64LE(rec.body, pos), TEST_DYNAMIC_OBJ_COUNT);  // dynamicObjCount
    pos += HYBRID_DUMP_IDENTIFIER_SIZE;
    EXPECT_EQ(ReadU64LE(rec.body, pos), DUMP_CLASS_COUNT);  // classCount
}

TEST_F(StaticDumpWriterTest, WriteXRefEdge_ByteLayout)
{
    TestOutputStream ts;
    CommonWriter cw(ts.Get());

    cw.BeginRecord(TAG_XREF_EDGE);
    cw.WriteXRefEdge(TEST_INSTANCE_OBJECT_ID, TEST_INSTANCE_CLASS_ID, XREF_DIR_STA_TO_DYN);
    cw.EndRecord();

    auto bytes = ts.CapturedBytes();
    ASSERT_GE(bytes.size(), RECORD_HEADER_SIZE + XREF_EDGE_BODY_SIZE);

    size_t off = 0;
    EXPECT_EQ(bytes.at(off + TAG_OFFSET), TAG_XREF_EDGE);
    EXPECT_EQ(ReadU32LE(bytes, off + SIZE_OFFSET), XREF_EDGE_BODY_SIZE);
    EXPECT_EQ(ReadU32LE(bytes, off + COUNT_OFFSET), 1U);

    size_t bodyOff = RECORD_HEADER_SIZE;
    EXPECT_EQ(ReadU32LE(bytes, bodyOff + XREF_FROM_OFF), TEST_INSTANCE_OBJECT_ID);
    EXPECT_EQ(ReadU32LE(bytes, bodyOff + XREF_TO_OFF), TEST_INSTANCE_CLASS_ID);
    EXPECT_EQ(bytes.at(bodyOff + XREF_DIR_OFF), XREF_DIR_STA_TO_DYN);
}

// ============================================================================
// StaticDumpUtilTest - MapFieldType / ExtractFieldName / ComputeClassFlags
// ============================================================================

TEST(StaticDumpUtilTest, MapFieldType_PrimitivesAndFallback)
{
    using Tid = panda_file::Type::TypeId;
    EXPECT_EQ(StaticDump::MapFieldType(Tid::I8), FieldType::BYTE);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::U8), FieldType::BYTE);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::I16), FieldType::SHORT);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::U16), FieldType::CHAR);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::I32), FieldType::INT);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::U32), FieldType::INT);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::I64), FieldType::LONG);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::U64), FieldType::LONG);

    // Unique mappings
    EXPECT_EQ(StaticDump::MapFieldType(Tid::U1), FieldType::BOOLEAN);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::F32), FieldType::FLOAT);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::F64), FieldType::DOUBLE);

    // Declared Any values remain TAGGED; references use OBJECT.
    EXPECT_EQ(StaticDump::MapFieldType(Tid::TAGGED), FieldType::TAGGED);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::REFERENCE), FieldType::OBJECT);

    // Fallback to UNKNOWN
    EXPECT_EQ(StaticDump::MapFieldType(Tid::INVALID), FieldType::UNKNOWN);
    EXPECT_EQ(StaticDump::MapFieldType(Tid::VOID), FieldType::UNKNOWN);
}

TEST(StaticDumpUtilTest, EncodeTaggedValueUsesRuntimeTypeAndLiveNodeId)
{
    StringIdPool stringPool;
    ObjectIdMap objectIdMap;
    DumpRequest request;
    StaticDump dump(nullptr, &stringPool, &objectIdMap, request);

    auto *object = reinterpret_cast<ObjectHeader *>(TEST_LIVE_OBJECT_ADDRESS);
    ASSERT_TRUE(objectIdMap.MarkLive<Language::STATIC>(reinterpret_cast<uintptr_t>(object)));
    uint32_t nodeId = objectIdMap.Find(reinterpret_cast<uintptr_t>(object));

    auto strong = StaticDumpTest::EncodeTaggedValue(&dump, coretypes::TaggedValue(object));
    EXPECT_EQ(strong.type, static_cast<uint8_t>(FieldType::OBJECT));
    EXPECT_EQ(strong.value, nodeId);

    auto weakValue = coretypes::TaggedValue(object).CreateAndGetWeakRef();
    auto weak = StaticDumpTest::EncodeTaggedValue(&dump, weakValue);
    EXPECT_EQ(weak.type, static_cast<uint8_t>(FieldType::WEAK_OBJECT));
    EXPECT_EQ(weak.value, nodeId);

    auto *unreachableObject = reinterpret_cast<ObjectHeader *>(TEST_UNREACHABLE_OBJECT_ADDRESS);
    auto unreachableWeakValue = coretypes::TaggedValue(unreachableObject).CreateAndGetWeakRef();
    auto unreachableWeak = StaticDumpTest::EncodeTaggedValue(&dump, unreachableWeakValue);
    EXPECT_EQ(unreachableWeak.type, static_cast<uint8_t>(FieldType::WEAK_OBJECT));
    EXPECT_EQ(unreachableWeak.value, 0U);

    auto integer = StaticDumpTest::EncodeTaggedValue(&dump, coretypes::TaggedValue(-7));
    EXPECT_EQ(integer.type, static_cast<uint8_t>(FieldType::INT));
    EXPECT_EQ(integer.value, 0xFFFFFFF9U);

    auto number = StaticDumpTest::EncodeTaggedValue(&dump, coretypes::TaggedValue(TEST_DOUBLE_VALUE));
    EXPECT_EQ(number.type, static_cast<uint8_t>(FieldType::DOUBLE));
    EXPECT_EQ(number.value, coretypes::ReinterpretDoubleToTaggedType(TEST_DOUBLE_VALUE));

    auto boolean = StaticDumpTest::EncodeTaggedValue(&dump, coretypes::TaggedValue(true));
    EXPECT_EQ(boolean.type, static_cast<uint8_t>(FieldType::BOOLEAN));
    EXPECT_EQ(boolean.value, 1U);

    auto special =
        StaticDumpTest::EncodeTaggedValue(&dump, coretypes::TaggedValue(coretypes::TaggedValue::VALUE_UNDEFINED));
    EXPECT_EQ(special.type, static_cast<uint8_t>(FieldType::TAGGED));
    EXPECT_EQ(special.value, coretypes::TaggedValue::VALUE_UNDEFINED);
}

TEST(StaticDumpUtilTest, AcquireOutputUsesRequestPath)
{
    constexpr const char *OUTPUT_PATH = "StaticDumpUtilTest_request_path.rawheap";
    StringIdPool stringPool;
    ObjectIdMap objectIdMap;
    DumpRequest request;
    request.output.staticPath = OUTPUT_PATH;

    {
        StaticDump dump(nullptr, &stringPool, &objectIdMap, request);
        EXPECT_TRUE(dump.AcquireOutput());
        EXPECT_TRUE(dump.AcquireOutput());
    }

    EXPECT_EQ(unlink(OUTPUT_PATH), 0);
}

// DeriveFromDescriptorChars parses an array descriptor string to infer the element type.
// Directly tests the extracted function (no longer needs Class* mock).
TEST(StaticDumpUtilTest, DeriveFromDescriptorChars_DescriptorParsing)
{
    // Primitive array descriptors
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[Z")),
              FieldType::BOOLEAN);  // boolean[]
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[B")),
              FieldType::BYTE);  // byte[]
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[H")),
              FieldType::BYTE);  // unsigned byte[] -> BYTE
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[C")),
              FieldType::CHAR);  // char[]
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[S")),
              FieldType::SHORT);  // short[]
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[I")),
              FieldType::INT);  // int[]
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[U")),
              FieldType::INT);  // unsigned int[] -> INT
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[J")),
              FieldType::LONG);  // long[]
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[Q")),
              FieldType::LONG);  // unsigned long[] -> LONG
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[F")),
              FieldType::FLOAT);  // float[]
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[D")),
              FieldType::DOUBLE);  // double[]

    // Reference array descriptors
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[L")),
              FieldType::OBJECT);  // reference[] (e.g., [LMyClass;)
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[A")),
              FieldType::TAGGED);  // tagged/dynamic[]

    // Nested array descriptor
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[[")),
              FieldType::ARRAY);  // nested array (e.g., int[][])

    // Invalid descriptors
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(nullptr), FieldType::UNKNOWN);
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("")), FieldType::UNKNOWN);
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("I")),
              FieldType::UNKNOWN);  // not an array descriptor
    EXPECT_EQ(StaticDump::DeriveFromDescriptorChars(reinterpret_cast<const uint8_t *>("[X")),
              FieldType::UNKNOWN);  // unknown type char
}

TEST(StaticDumpUtilTest, ExtractFieldName_AsciiAndModifiedUtf8Conversion)
{
    panda_file::File::StringData asciiData {};
    asciiData.isAscii = true;
    const char *text = "myField";
    asciiData.data = reinterpret_cast<const uint8_t *>(text);
    asciiData.utf16Length = 0;
    EXPECT_EQ(StaticDump::ExtractFieldName(asciiData), "myField");

    // Empty
    asciiData.data = reinterpret_cast<const uint8_t *>("");
    EXPECT_EQ(StaticDump::ExtractFieldName(asciiData), "");

    // File::StringData always stores MUTF-8 bytes, including when isAscii is false.
    panda_file::File::StringData nonAscii {};
    nonAscii.isAscii = false;
    const std::array<uint8_t, 3U> eAcute = {0xC3U, 0xA9U, 0U};
    nonAscii.data = eAcute.data();
    nonAscii.utf16Length = 1;
    EXPECT_EQ(StaticDump::ExtractFieldName(nonAscii), "\xC3\xA9");

    panda_file::File::StringData mixed {};
    mixed.isAscii = false;
    const std::array<uint8_t, 5U> mixedText = {'A', 0xC3U, 0xA9U, 'B', 0U};
    mixed.data = mixedText.data();
    mixed.utf16Length = 3;
    EXPECT_EQ(StaticDump::ExtractFieldName(mixed), std::string("A") + "\xC3\xA9" + "B");

    // Supplementary characters are encoded as a surrogate pair in MUTF-8 and
    // converted to standard four-byte UTF-8 in the dump.
    panda_file::File::StringData supplementary {};
    supplementary.isAscii = false;
    const std::array<uint8_t, 7U> grinningFace = {0xEDU, 0xA0U, 0xBDU, 0xEDU, 0xB8U, 0x80U, 0U};
    supplementary.data = grinningFace.data();
    supplementary.utf16Length = 2;
    EXPECT_EQ(StaticDump::ExtractFieldName(supplementary), "\xF0\x9F\x98\x80");
}

// ============================================================================
// StaticDumpIntegrationTest - End-to-end: Runtime -> DumpBinary -> parse -> verify
// ============================================================================

#ifdef STATIC_DUMP_TEST_ABC_DIR

#ifdef STATIC_DUMP_TEST_ABC_DIR
static const char kTestAbcFile[] = STATIC_DUMP_TEST_ABC_DIR "/StaticDumpTest.abc";
static const char kDumpPath[] = STATIC_DUMP_TEST_ABC_DIR "/static_dump_test_output.rawheap";
#else
static const char kTestAbcFile[] = "StaticDumpTest.abc";
static const char kDumpPath[] = "static_dump_test_output.rawheap";
#endif

// Expected class definitions from StaticDumpTest.ets.
// Runtime stores only the class's OWN instance fields in CLASS_DUMP.
struct ExpectedClass {
    std::string simpleName;
    bool optional = false;
};

static const std::vector<ExpectedClass> kExpectedClasses = {
    {"AllPrimitives"},   {"Node"},         {"Animal", true}, {"Dog"},          {"Cat"},  {"Person"},
    {"DataSet"},         {"FriendGroup"},  {"Base"},         {"Mid"},          {"Leaf"}, {"CircularFriendA"},
    {"CircularFriendB"}, {"TaggedHolder"}, {"WeakTarget"},   {"SizedObjects"},
};

class HeapsnapshotGraph {
public:
    explicit HeapsnapshotGraph(const std::string &path)
    {
        std::ifstream ifs(path);
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        if (content.empty()) {
            return;
        }
        root_ = cJSON_Parse(content.c_str());
        if (root_ == nullptr) {
            return;
        }
        cJSON *meta = cJSON_GetObjectItem(root_, "snapshot");
        if (meta != nullptr) {
            cJSON *nc = cJSON_GetObjectItem(meta, "node_count");
            cJSON *ec = cJSON_GetObjectItem(meta, "edge_count");
            if (nc != nullptr) {
                nodeCount_ = nc->valueint;
            }
            if (ec != nullptr) {
                edgeCount_ = ec->valueint;
            }
        }
        nodes_ = cJSON_GetObjectItem(root_, "nodes");
        edges_ = cJSON_GetObjectItem(root_, "edges");
        strings_ = cJSON_GetObjectItem(root_, "strings");
        // node_types / edge_types live under snapshot.meta (not at the root).
        cJSON *metaMeta = (meta != nullptr) ? cJSON_GetObjectItem(meta, "meta") : nullptr;
        nodeTypes_ = (metaMeta != nullptr) ? cJSON_GetObjectItem(metaMeta, "node_types") : nullptr;
        edgeTypes_ = (metaMeta != nullptr) ? cJSON_GetObjectItem(metaMeta, "edge_types") : nullptr;
        ok_ = (nodes_ != nullptr && edges_ != nullptr && strings_ != nullptr && nodeTypes_ != nullptr &&
               edgeTypes_ != nullptr && nodeCount_ > 0);
    }
    ~HeapsnapshotGraph()
    {
        if (root_ != nullptr) {
            cJSON_Delete(root_);
        }
    }
    bool Ok() const
    {
        return ok_;
    }
    int NodeCount() const
    {
        return nodeCount_;
    }
    int EdgeCount() const
    {
        return edgeCount_;
    }
    int NodesArraySize() const
    {
        return cJSON_GetArraySize(nodes_);
    }
    int EdgesArraySize() const
    {
        return cJSON_GetArraySize(edges_);
    }

    double NodeField(int nodeIdx, int field) const
    {
        cJSON *item = cJSON_GetArrayItem(nodes_, nodeIdx * NODE_FIELD_COUNT + field);
        return (item != nullptr) ? item->valuedouble : 0.0;
    }
    std::string NodeName(int nodeIdx) const
    {
        int strIdx = static_cast<int>(NodeField(nodeIdx, NAME_FIELD));
        cJSON *s = cJSON_GetArrayItem(strings_, strIdx);
        return (s != nullptr && s->valuestring != nullptr) ? std::string(s->valuestring) : std::string();
    }
    std::string NodeType(int nodeIdx) const
    {
        int typeIdx = static_cast<int>(NodeField(nodeIdx, TYPE_FIELD));
        cJSON *types0 = cJSON_GetArrayItem(nodeTypes_, 0);  // node_types[0] = type-name array
        cJSON *t = cJSON_GetArrayItem(types0, typeIdx);
        return (t != nullptr && t->valuestring != nullptr) ? std::string(t->valuestring) : std::string();
    }
    int NodeEdgeCount(int nodeIdx) const
    {
        return static_cast<int>(NodeField(nodeIdx, EDGECOUNT_FIELD));
    }
    uint64_t NodeSelfSize(int nodeIdx) const
    {
        return static_cast<uint64_t>(NodeField(nodeIdx, SELF_SIZE_FIELD));
    }

    // Index of the first edge belonging to nodeIdx. Uses a lazily-built
    // prefix-sum cache - without it, a single EdgeStart(nodeIdx) is O(nodeIdx^2)
    // (cJSON arrays are linked lists, so each NodeEdgeCount(i) is O(i)), and a
    // full-graph BFS calling EdgeStart per node degrades to O(N^3) (~minutes).
    // The cache is built once (O(N^2)) on first use, then O(1) per lookup.
    int EdgeStart(int nodeIdx) const
    {
        EnsureEdgeStartCache();
        return edgeStartCache_[nodeIdx];
    }
    // Edge field accessor for absolute edge index j (0..edgeCount-1).
    double EdgeField(int j, int field) const
    {
        cJSON *item = cJSON_GetArrayItem(edges_, j * EDGE_FIELD_COUNT + field);
        return (item != nullptr) ? item->valuedouble : 0.0;
    }
    int EdgeTypeIndex(int j) const
    {
        return static_cast<int>(EdgeField(j, EDGE_TYPE_FIELD));
    }
    int EdgeNameOrIndex(int j) const
    {
        return static_cast<int>(EdgeField(j, EDGE_NAME_FIELD));
    }
    int EdgeToNodeIndex(int j) const
    {
        return static_cast<int>(EdgeField(j, EDGE_TO_NODE_FIELD) / NODE_FIELD_COUNT);  // to_node is a byte offset
    }
    // Edge type name (e.g. "property","internal") for absolute edge index j.
    std::string EdgeTypeName(int j) const
    {
        int typeIdx = EdgeTypeIndex(j);
        cJSON *types0 = cJSON_GetArrayItem(edgeTypes_, 0);
        cJSON *t = cJSON_GetArrayItem(types0, typeIdx);
        return (t != nullptr && t->valuestring != nullptr) ? std::string(t->valuestring) : std::string();
    }
    // For string-typed edges, resolve name_or_index to a string.
    std::string EdgeName(int j) const
    {
        int strIdx = EdgeNameOrIndex(j);
        cJSON *s = cJSON_GetArrayItem(strings_, strIdx);
        return (s != nullptr && s->valuestring != nullptr) ? std::string(s->valuestring) : std::string();
    }

    // True if `name` appears in the snapshot.meta edge_types[0] declaration.
    // Guards the "xref" edge-type meta fix (EdgeType::XREF must be declared,
    // otherwise any xref edge would carry an out-of-range type field).
    bool EdgeTypeIsDeclared(const std::string &name) const
    {
        cJSON *types0 = cJSON_GetArrayItem(edgeTypes_, 0);
        cJSON *t = nullptr;
        cJSON_ArrayForEach(t, types0)
        {
            if (t->valuestring != nullptr && std::string(t->valuestring) == name) {
                return true;
            }
        }
        return false;
    }

    // Find the first node whose type==typeStr and name contains nameSubstr.
    int FindNode(const std::string &typeStr, const std::string &nameSubstr) const
    {
        for (int i = 0; i < nodeCount_; ++i) {
            if (NodeType(i) == typeStr && NodeName(i).find(nameSubstr) != std::string::npos) {
                return i;
            }
        }
        return -1;
    }

    // Find a class node by its simple (last path segment) name. Substring-based
    // FindNode("class","Base") would match "StaticDumpTest.Base[]" (an array
    // class) before "StaticDumpTest.Base"; this matches the last '.'-separated
    // segment exactly so array classes ("Base[]") don't shadow the real class.
    int FindClassBySimpleName(const std::string &simpleName) const
    {
        for (int i = 0; i < nodeCount_; ++i) {
            if (NodeType(i) != "class") {
                continue;
            }
            std::string nm = NodeName(i);
            auto pos = nm.find_last_of('.');
            std::string last = (pos == std::string::npos) ? nm : nm.substr(pos + 1);
            if (last == simpleName) {
                return i;
            }
        }
        return -1;
    }

    // Find a node by its `id` field. O(N) - node_fields[2] is the id.
    int FindNodeById(uint64_t id) const
    {
        for (int i = 0; i < nodeCount_; ++i) {
            if (static_cast<uint64_t>(NodeField(i, ID_FIELD)) == id) {
                return i;
            }
        }
        return -1;
    }

    // Reachable-set from `startIdx` (DFS over to_node edges; reachability is
    // order-independent). Returns a bool vector indexed by node index.
    std::vector<bool> ReachableFrom(int startIdx) const
    {
        std::vector<bool> seen(nodeCount_, false);
        if (startIdx < 0 || startIdx >= nodeCount_) {
            return seen;
        }
        std::vector<int> stack;
        stack.push_back(startIdx);
        seen[startIdx] = true;
        while (!stack.empty()) {
            int u = stack.back();
            stack.pop_back();
            int s = EdgeStart(u);
            int e = s + NodeEdgeCount(u);
            for (int j = s; j < e; ++j) {
                int v = EdgeToNodeIndex(j);
                if (v >= 0 && v < nodeCount_ && !seen[v]) {
                    seen[v] = true;
                    stack.push_back(v);
                }
            }
        }
        return seen;
    }

    static constexpr int NODE_FIELD_COUNT = 8;
    static constexpr int EDGE_FIELD_COUNT = 3;
    static constexpr int TYPE_FIELD = 0;
    static constexpr int NAME_FIELD = 1;
    static constexpr int ID_FIELD = 2;
    static constexpr int SELF_SIZE_FIELD = 3;
    static constexpr int EDGECOUNT_FIELD = 4;
    // Edge record layout: [edgeType, name_or_index, to_node].
    static constexpr int EDGE_TYPE_FIELD = 0;
    static constexpr int EDGE_NAME_FIELD = 1;
    static constexpr int EDGE_TO_NODE_FIELD = 2;

private:
    void EnsureEdgeStartCache() const
    {
        if (edgeStartReady_) {
            return;
        }
        edgeStartCache_.assign(nodeCount_ + 1, 0);
        for (int i = 0; i < nodeCount_; ++i) {
            edgeStartCache_[i + 1] = edgeStartCache_[i] + NodeEdgeCount(i);
        }
        edgeStartReady_ = true;
    }

    cJSON *root_ {nullptr};
    cJSON *nodes_ {nullptr};
    cJSON *edges_ {nullptr};
    cJSON *strings_ {nullptr};
    cJSON *nodeTypes_ {nullptr};
    cJSON *edgeTypes_ {nullptr};
    int nodeCount_ {0};
    int edgeCount_ {0};
    bool ok_ {false};
    mutable std::vector<int> edgeStartCache_;
    mutable bool edgeStartReady_ {false};
};

// ============================================================================
// Structure-driven helpers for StaticDumpTest.ets assertions.
// Instance nodes are type "object" named "<pkg>.<ClassSimple>" (e.g.
// "StaticDumpTest.Person"). Each instance carries one "hclass" property edge
// (-> its class node) plus one PROPERTY edge per instance field (edge name
// = field name; target = a number value node for primitives, the referenced
// object/node for references, or a string-typed node named by its content for
// string fields). The static dumper walks the base chain, so OWN + INHERITED
// instance fields are emitted (e.g. a Dog instance has breed + species + age,
// where species/age are inherited from Animal). Per-class expected field sets
// below include inherited fields.
// ============================================================================

// One pass over all nodes; returns class simple-name -> instance node indices.
// NodeType/NodeName are O(index) (cJSON linked-list traversal), so this pass is
// O(N^2) - called once per test that needs instance lookups.
static std::map<std::string, std::vector<int>> CollectTestInstances(const HeapsnapshotGraph &g)
{
    static const std::string PREFIX = "StaticDumpTest.";
    std::map<std::string, std::vector<int>> out;
    for (int i = 0; i < g.NodeCount(); ++i) {
        if (g.NodeType(i) != "object") {
            continue;
        }
        std::string nm = g.NodeName(i);
        if (nm.compare(0, PREFIX.size(), PREFIX) != 0) {
            continue;
        }
        out[nm.substr(PREFIX.size())].push_back(i);
    }
    return out;
}

// All PROPERTY edges of a node as (name -> target node index). Calls EdgeStart
// once (O(nodeIdx^2)) so per-instance cost is bounded regardless of field count.
static std::map<std::string, int> GetPropertyEdges(const HeapsnapshotGraph &g, int nodeIdx)
{
    std::map<std::string, int> out;
    int s = g.EdgeStart(nodeIdx);
    int e = s + g.NodeEdgeCount(nodeIdx);
    for (int j = s; j < e; ++j) {
        if (g.EdgeTypeName(j) == "property") {
            out[g.EdgeName(j)] = g.EdgeToNodeIndex(j);
        }
    }
    return out;
}

// Target node index of a node's property edge named `field`, or -1.
static int FindPropertyEdge(const HeapsnapshotGraph &g, int nodeIdx, const std::string &field)
{
    auto edges = GetPropertyEdges(g, nodeIdx);
    auto it = edges.find(field);
    return (it != edges.end()) ? it->second : -1;
}

// Upper bound on "buffer" property hops when descending to an array's element owner.
static constexpr int MAX_BUFFER_HOPS = 16;

// Whether `cur` owns any "element" edge.
static bool HasElementEdge(const HeapsnapshotGraph &g, int cur)
{
    int s = g.EdgeStart(cur);
    int e = s + g.NodeEdgeCount(cur);
    for (int j = s; j < e; ++j) {
        if (g.EdgeTypeName(j) == "element") {
            return true;
        }
    }
    return false;
}

// If `cur` owns element edges return `cur`; otherwise advance to its "buffer"
// sub-node, or return `cur` again when there is no further buffer hop.
static int NextBufferOrSelf(const HeapsnapshotGraph &g, int cur)
{
    if (HasElementEdge(g, cur)) {
        return cur;
    }
    int next = FindPropertyEdge(g, cur, "buffer");
    return (next < 0) ? cur : next;
}

// An ETS array's ELEMENT edges live on a synthetic "buffer" sub-node reached
// through one or more "buffer" property hops (wrapper -> inner array -> buffer).
// Descend to the first node that owns element edges.
static int GetArrayBufferNode(const HeapsnapshotGraph &g, int arrayObjIdx)
{
    int cur = FindPropertyEdge(g, arrayObjIdx, "buffer");
    for (int guard = 0; cur >= 0 && guard < MAX_BUFFER_HOPS; ++guard) {
        int next = NextBufferOrSelf(g, cur);
        if (next == cur) {
            return cur;
        }
        cur = next;
    }
    return cur;
}

// Element target node indices of an array object (via its buffer sub-node).
static std::vector<int> GetArrayElements(const HeapsnapshotGraph &g, int arrayObjIdx)
{
    std::vector<int> out;
    int buf = GetArrayBufferNode(g, arrayObjIdx);
    if (buf < 0) {
        return out;
    }
    int s = g.EdgeStart(buf);
    int e = s + g.NodeEdgeCount(buf);
    for (int j = s; j < e; ++j) {
        if (g.EdgeTypeName(j) == "element") {
            out.push_back(g.EdgeToNodeIndex(j));
        }
    }
    return out;
}

// -- Integration-test query helpers (operate on a translated heapsnapshot) --

// First instance index of `cls`, or -1 if none.
static int FirstInstance(const std::map<std::string, std::vector<int>> &instances, const std::string &cls)
{
    auto it = instances.find(cls);
    return (it != instances.end() && !it->second.empty()) ? it->second.front() : -1;
}

// Instance of `cls` whose number field `field` carries value `val`, or -1.
static int FindInstanceByNumField(const HeapsnapshotGraph &g, const std::map<std::string, std::vector<int>> &instances,
                                  const std::string &cls, const std::string &field, const std::string &val)
{
    auto it = instances.find(cls);
    if (it == instances.end()) {
        return -1;
    }
    for (int idx : it->second) {
        auto edges = GetPropertyEdges(g, idx);
        auto fi = edges.find(field);
        if (fi != edges.end() && g.NodeType(fi->second) == "number" && g.NodeName(fi->second) == val) {
            return idx;
        }
    }
    return -1;
}

// Instance of `cls` whose string-typed field `field` targets a string node
// named `val`, or -1. INSTANCE_DUMP record order follows the dumper's
// unordered_map iteration over object addresses and is not stable across
// builds (debug vs release, ASLR, etc.), so locating an instance by a
// distinguishing field value is preferable to relying on dump order.
static int FindInstanceByStringField(const HeapsnapshotGraph &g,
                                     const std::map<std::string, std::vector<int>> &instances, const std::string &cls,
                                     const std::string &field, const std::string &val)
{
    auto it = instances.find(cls);
    if (it == instances.end()) {
        return -1;
    }
    for (int idx : it->second) {
        auto edges = GetPropertyEdges(g, idx);
        auto fi = edges.find(field);
        if (fi != edges.end() && g.NodeType(fi->second) == "string" && g.NodeName(fi->second) == val) {
            return idx;
        }
    }
    return -1;
}

// Node name of the target of idx's property edge `field` (empty if missing).
static std::string TargetNodeName(const HeapsnapshotGraph &g, int idx, const std::string &field)
{
    auto edges = GetPropertyEdges(g, idx);
    auto it = edges.find(field);
    return (it != edges.end()) ? g.NodeName(it->second) : std::string();
}

// Node type of the target of idx's property edge `field` (empty if missing).
static std::string TargetNodeType(const HeapsnapshotGraph &g, int idx, const std::string &field)
{
    auto edges = GetPropertyEdges(g, idx);
    auto it = edges.find(field);
    return (it != edges.end()) ? g.NodeType(it->second) : std::string();
}

// Assert a number field's value node name on an instance.
static void CheckNumField(const HeapsnapshotGraph &g, int idx, const std::string &field, const std::string &val)
{
    auto edges = GetPropertyEdges(g, idx);
    auto fi = edges.find(field);
    ASSERT_NE(fi, edges.end()) << "field " << field << " missing";
    EXPECT_EQ(g.NodeType(fi->second), "number") << field << " target not a number";
    EXPECT_EQ(g.NodeName(fi->second), val) << "field " << field;
}

static void CheckBoxedValue(const HeapsnapshotGraph &g, int idx, const std::string &className, const std::string &value)
{
    EXPECT_EQ(g.NodeName(idx), className);
    EXPECT_EQ(TargetNodeName(g, idx, "value"), value);
}

// Count superClass edges of a class node that target the same node (duplicates).
static int CountSuperClassDuplicates(const HeapsnapshotGraph &g, int nodeIdx)
{
    std::map<int, int> targets;  // toNodeIdx -> count
    int s = g.EdgeStart(nodeIdx);
    int e = s + g.NodeEdgeCount(nodeIdx);
    for (int j = s; j < e; ++j) {
        if (g.EdgeName(j) == "superClass") {
            targets[g.EdgeToNodeIndex(j)]++;
        }
    }
    int dups = 0;
    for (const auto &kv : targets) {
        if (kv.second > 1) {
            ++dups;
        }
    }
    return dups;
}

// ============================================================================
// End-to-end validation via the real translator (rawheap_translate_static):
// Runtime -> DumpBinary -> .rawheap -> RawHeap::TranslateRawheap -> .heapsnapshot.
// All assertions are expressed at the translated heapsnapshot-graph level; there is no
// hand-written binary parser. Binary-only metadata (file header, record tags,
// classFlags, method names) is intentionally not validated here - the
// .heapsnapshot does not carry it.
// ============================================================================

class StaticDumpIntegrationTest : public ::testing::Test {
protected:
    static void SetUpTestSuite()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);

        auto *stdlib = std::getenv("PANDA_STD_LIB");
        ASSERT_NE(stdlib, nullptr) << "PANDA_STD_LIB must point to etsstdlib.abc";
        options.SetBootPandaFiles({stdlib, kTestAbcFile});
        ASSERT_TRUE(Runtime::Create(options));
        Runtime::GetCurrent()->ExecutePandaFile(kTestAbcFile, "StaticDumpTest.ETSGLOBAL::main", {});
    }

    static void TearDownTestSuite()
    {
        if (Runtime::GetCurrent() != nullptr) {
            Runtime::Destroy();
        }
    }

    // Run the real dumper to produce .rawheap, then drive the real translator
    // (rawheap_translate_static) to emit a .heapsnapshot and wrap it in the
    // cJSON graph reader. No hand-written binary parser.
    static std::unique_ptr<HeapsnapshotGraph> DumpAndTranslate()
    {
        auto &profiler = HeapDumpCoordinator::GetInstance();
        DumpRequest request;
        request.policy.executionMode = DumpExecutionMode::IN_PROCESS;
        auto dump = StaticDump::Create(Runtime::GetCurrent()->GetPandaVM(), &profiler.GetStringIdPool(),
                                       &profiler.GetObjectIdMap(), request);
        auto *staticDump = static_cast<StaticDump *>(dump.get());
        if (staticDump == nullptr) {
            LOG(ERROR, RUNTIME) << "[StaDumpTest] failed to create static dumper";
            return nullptr;
        }

        const int fd = open(kDumpPath, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, TEST_DUMP_FILE_PERMS);
        if (fd < 0) {
            LOG(ERROR, RUNTIME) << "[StaDumpTest] failed to open test dump file";
            return nullptr;
        }
        StaticDumpTest::SetOutput(staticDump, fd);

        profiler.GetStringIdPool().Unfreeze();
        profiler.GetObjectIdMap().Unfreeze();
        staticDump->TriggerGC();
        staticDump->PrepareSession();
        if (!staticDump->Dump().success) {
            LOG(ERROR, RUNTIME) << "[StaDumpTest] static dump lifecycle failed";
            return nullptr;
        }
        // StaticDump owns and closes the injected descriptor on destruction.
        dump.reset();

        std::string jsonPath = std::string(kDumpPath) + ".heapsnapshot";
        if (!rawheap_translate::RawHeap::TranslateRawheap(kDumpPath, jsonPath)) {
            LOG(ERROR, RUNTIME) << "[StaDumpTest] RawHeap::TranslateRawheap failed";
            return nullptr;
        }

        auto g = std::make_unique<HeapsnapshotGraph>(jsonPath);
        return g->Ok() ? std::move(g) : nullptr;
    }
};

TEST_F(StaticDumpIntegrationTest, HeapsnapshotProducedAndStructurallyValid)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    // Flat array lengths match snapshot.meta node_count/edge_count.
    EXPECT_EQ(g->NodesArraySize(), g->NodeCount() * HeapsnapshotGraph::NODE_FIELD_COUNT);
    EXPECT_EQ(g->EdgesArraySize(), g->EdgeCount() * HeapsnapshotGraph::EDGE_FIELD_COUNT);
    EXPECT_GT(g->NodeCount(), 0);
    EXPECT_GT(g->EdgeCount(), 0);
}

TEST_F(StaticDumpIntegrationTest, HeapsnapshotContainsExpectedClassNodes)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    for (const auto &ec : kExpectedClasses) {
        if (ec.optional) {
            continue;
        }
        EXPECT_GE(g->FindClassBySimpleName(ec.simpleName), 0)
            << "expected class node '" << ec.simpleName << "' not in heapsnapshot";
    }
}

TEST_F(StaticDumpIntegrationTest, HeapsnapshotInheritanceChainValid)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    // superClass chain: Leaf --internal--> Mid --internal--> Base.
    auto assertInternalTo = [&](int fromIdx, const std::string &targetName) -> bool {
        int s = g->EdgeStart(fromIdx);
        int e = s + g->NodeEdgeCount(fromIdx);
        for (int j = s; j < e; ++j) {
            if (g->EdgeTypeName(j) != "internal") {
                continue;
            }
            int toIdx = g->EdgeToNodeIndex(j);
            if (g->NodeType(toIdx) == "class" && g->NodeName(toIdx).find(targetName) != std::string::npos) {
                return true;
            }
        }
        return false;
    };

    int leafIdx = g->FindClassBySimpleName("Leaf");
    ASSERT_GE(leafIdx, 0) << "Leaf class node not found";
    EXPECT_TRUE(assertInternalTo(leafIdx, "Mid")) << "Leaf has no internal superClass edge to Mid";

    int midIdx = g->FindClassBySimpleName("Mid");
    ASSERT_GE(midIdx, 0) << "Mid class node not found";
    EXPECT_TRUE(assertInternalTo(midIdx, "Base")) << "Mid has no internal superClass edge to Base";
}

TEST_F(StaticDumpIntegrationTest, HeapsnapshotStaticFieldsAndValuesValid)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    int baseIdx = g->FindClassBySimpleName("Base");
    ASSERT_GE(baseIdx, 0) << "Base class node not found";

    bool counterFound = false;
    bool instancesFound = false;
    int eStart = g->EdgeStart(baseIdx);
    int eEnd = eStart + g->NodeEdgeCount(baseIdx);
    for (int j = eStart; j < eEnd; ++j) {
        if (g->EdgeTypeName(j) != "property") {
            continue;
        }
        std::string name = g->EdgeName(j);
        int toIdx = g->EdgeToNodeIndex(j);
        if (name == "counter") {
            counterFound = true;
            EXPECT_EQ(g->NodeType(toIdx), "number") << "counter value node should be type 'number'";
            // counter == 4 after 3x init() (counter=3) + 1x increment() (counter=4).
            EXPECT_EQ(g->NodeName(toIdx), "4");
        } else if (name == "instances") {
            instancesFound = true;
            // Base.instances is a non-null array reference.
            EXPECT_TRUE(g->NodeType(toIdx) == "array" || g->NodeType(toIdx) == "object")
                << "instances edge target should be an array/object node";
        }
    }
    EXPECT_TRUE(counterFound) << "Base has no 'counter' PROPERTY edge to a number value node";
    EXPECT_TRUE(instancesFound) << "Base has no 'instances' PROPERTY edge to an array/object node";
}

TEST_F(StaticDumpIntegrationTest, HeapsnapshotInstanceToClassEdgesValid)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    // Every "hclass" PROPERTY edge must point to a "class" node. Scan nodes
    // until the first hclass edge is found (instance nodes appear early in the
    // graph); verifying one is enough - the edge is emitted by the same code
    // path for every instance. (Full enumeration is avoided because
    // cJSON_GetArrayItem is O(n) and the root/staticRoot nodes own many edges.)
    bool anyHclass = false;
    for (int i = 0; i < g->NodeCount() && !anyHclass; ++i) {
        int s = g->EdgeStart(i);
        int e = s + g->NodeEdgeCount(i);
        for (int j = s; j < e; ++j) {
            if (g->EdgeTypeName(j) != "property" || g->EdgeName(j) != "hclass") {
                continue;
            }
            anyHclass = true;
            int toIdx = g->EdgeToNodeIndex(j);
            EXPECT_EQ(g->NodeType(toIdx), "class") << "node " << i << " has hclass edge to non-class node " << toIdx;
            break;
        }
    }
    EXPECT_TRUE(anyHclass) << "no instance->class 'hclass' edge found";
}

// ============================================================================
// Structure-driven tests: assertions tied to StaticDumpTest.ets declarations.
// Each test maps a .ets construct (method/field/value) to a graph assertion
// (edge name == method/field name; value node name == the assigned value).
// ============================================================================

// Each test class's class node carries a PROPERTY edge (-> closure node) for
// every method declared in the .ets, with the edge name == method name. The
// ETS compiler also emits implicit <ctor>/<cctor>; we only assert the
// user-declared methods are present (not that the generated ones are absent).
TEST_F(StaticDumpIntegrationTest, HeapsnapshotClassMethodEdgesMatchEts)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    struct Cls {
        std::string simple;
        std::vector<std::string> methods;
    };
    const Cls classes[] = {
        {"Base", {"init", "increment", "describe"}},
        {"Mid", {"run", "greet"}},
        {"Leaf", {"serial", "compare", "setScore"}},
        {"SnapshotFixtures", {"mirrorOnlyMethod"}},
    };
    for (const auto &c : classes) {
        int idx = g->FindClassBySimpleName(c.simple);
        ASSERT_GE(idx, 0) << "class " << c.simple << " node not found";
        // Collect this class node's method-name edges (property -> closure).
        std::set<std::string> methodEdges;
        int s = g->EdgeStart(idx);
        int e = s + g->NodeEdgeCount(idx);
        for (int j = s; j < e; ++j) {
            if (g->EdgeTypeName(j) != "property") {
                continue;
            }
            int to = g->EdgeToNodeIndex(j);
            if (g->NodeType(to) == "closure") {
                methodEdges.insert(g->EdgeName(j));
            }
        }
        for (const auto &m : c.methods) {
            EXPECT_GT(methodEdges.count(m), 0U) << "class " << c.simple << " missing method edge '" << m << "'";
        }
    }
}

// Each instance's PROPERTY edges (excluding the synthetic "hclass" edge) must
// match the class's populated own and inherited fields. Undefined tagged values
// do not produce heap edges, so TaggedHolder.specialValue is intentionally absent.
TEST_F(StaticDumpIntegrationTest, HeapsnapshotPopulatedInstanceFieldNamesMatchEts)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto instances = CollectTestInstances(*g);
    struct Cls {
        std::string simple;
        std::vector<std::string> fields;
    };
    const Cls classes[] = {
        {"AllPrimitives", {"z", "b", "s", "i", "j", "f", "d"}},
        {"Node", {"name", "next"}},
        {"Dog", {"breed", "species", "age"}},
        {"Cat", {"color", "species", "age"}},
        {"Person", {"age", "weight", "height", "flags", "id", "bigId", "name", "pet", "active"}},
        {"DataSet", {"label", "values"}},
        {"FriendGroup", {"groupName", "members"}},
        {"Base", {"id", "name"}},
        {"Mid", {"tag", "id", "name"}},
        {"Leaf", {"score", "tag", "id", "name"}},
        {"TaggedHolder", {"intValue", "doubleValue", "boolValue", "objectValue", "values"}},
        {"WeakTarget", {"marker"}},
        {"SizedObjects", {"shortArray", "longArray", "shortText", "longText"}},
    };
    for (const auto &c : classes) {
        auto it = instances.find(c.simple);
        ASSERT_TRUE(it != instances.end() && !it->second.empty())
            << "no instance of " << c.simple << " in heapsnapshot";
        int idx = it->second.front();
        auto edges = GetPropertyEdges(*g, idx);
        std::set<std::string> fieldEdges;
        for (const auto &kv : edges) {
            if (kv.first != "hclass") {
                fieldEdges.insert(kv.first);
            }
        }
        std::set<std::string> expected(c.fields.begin(), c.fields.end());
        EXPECT_EQ(fieldEdges, expected) << "instance field edge names of " << c.simple << " do not match .ets";
    }
}

TEST_F(StaticDumpIntegrationTest, HeapsnapshotAnyValuesPreserveBoxedRuntimeTypes)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto instances = CollectTestInstances(*g);
    int holder = FirstInstance(instances, "TaggedHolder");
    ASSERT_GE(holder, 0);
    auto fields = GetPropertyEdges(*g, holder);

    ASSERT_NE(fields.find("intValue"), fields.end());
    CheckBoxedValue(*g, fields["intValue"], "std.core.Int", "-7");
    ASSERT_NE(fields.find("doubleValue"), fields.end());
    CheckBoxedValue(*g, fields["doubleValue"], "std.core.Double", "2.500000");
    ASSERT_NE(fields.find("boolValue"), fields.end());
    CheckBoxedValue(*g, fields["boolValue"], "std.core.Boolean", "true");
    ASSERT_NE(fields.find("objectValue"), fields.end());
    EXPECT_EQ(g->NodeName(fields["objectValue"]), "StaticDumpTest.Node");
    EXPECT_EQ(TargetNodeName(*g, fields["objectValue"], "name"), "tagged-object-only");
    EXPECT_EQ(fields.find("specialValue"), fields.end());

    ASSERT_NE(fields.find("values"), fields.end());
    auto elements = GetArrayElements(*g, fields["values"]);
    constexpr size_t FALSE_ELEMENT_INDEX = 2U;
    constexpr size_t OBJECT_ELEMENT_INDEX = 3U;
    constexpr size_t NULL_ELEMENT_INDEX = 4U;
    ASSERT_EQ(elements.size(), 5U);
    CheckBoxedValue(*g, elements[0], "std.core.Int", "-9");
    CheckBoxedValue(*g, elements[1], "std.core.Double", "6.250000");
    CheckBoxedValue(*g, elements[FALSE_ELEMENT_INDEX], "std.core.Boolean", "false");
    EXPECT_EQ(g->NodeName(elements[OBJECT_ELEMENT_INDEX]), "StaticDumpTest.Node");
    EXPECT_EQ(TargetNodeName(*g, elements[OBJECT_ELEMENT_INDEX], "name"), "tagged-array-only");
    EXPECT_EQ(g->NodeName(elements[NULL_ELEMENT_INDEX]), "std.core.Null");

    int holderClass = g->FindClassBySimpleName("TaggedHolder");
    ASSERT_GE(holderClass, 0);
    auto staticFields = GetPropertyEdges(*g, holderClass);
    ASSERT_NE(staticFields.find("staticObjectValue"), staticFields.end());
    EXPECT_EQ(g->NodeName(staticFields["staticObjectValue"]), "StaticDumpTest.Node");
    EXPECT_EQ(TargetNodeName(*g, staticFields["staticObjectValue"], "name"), "tagged-static-only");
    ASSERT_NE(staticFields.find("staticSpecialValue"), staticFields.end());
    EXPECT_EQ(g->NodeName(staticFields["staticSpecialValue"]), "std.core.Null");
    ASSERT_NE(staticFields.find("staticIntValue"), staticFields.end());
    CheckBoxedValue(*g, staticFields["staticIntValue"], "std.core.Int", "77");
}

TEST_F(StaticDumpIntegrationTest, HeapsnapshotWeakReferenceDoesNotKeepReferentAlive)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto instances = CollectTestInstances(*g);
    auto targetIt = instances.find("WeakTarget");
    ASSERT_NE(targetIt, instances.end());
    ASSERT_EQ(targetIt->second.size(), 1U) << "weak-only referent must not be included by reachability traversal";

    int retainedTarget = FindInstanceByStringField(*g, instances, "WeakTarget", "marker", "weak-retained");
    ASSERT_GE(retainedTarget, 0);

    bool foundWeakEdge = false;
    for (int node = 0; node < g->NodeCount() && !foundWeakEdge; ++node) {
        int edgeStart = g->EdgeStart(node);
        int edgeEnd = edgeStart + g->NodeEdgeCount(node);
        for (int edge = edgeStart; edge < edgeEnd; ++edge) {
            if (g->EdgeTypeName(edge) == "weak" && g->EdgeToNodeIndex(edge) == retainedTarget) {
                foundWeakEdge = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundWeakEdge) << "retained WeakRef referent must be serialized as a weak edge";
}

TEST_F(StaticDumpIntegrationTest, HeapsnapshotUsesActualVariableObjectSizes)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto instances = CollectTestInstances(*g);
    int sized = FirstInstance(instances, "SizedObjects");
    ASSERT_GE(sized, 0);
    auto fields = GetPropertyEdges(*g, sized);

    ASSERT_NE(fields.find("shortArray"), fields.end());
    ASSERT_NE(fields.find("longArray"), fields.end());
    int shortBuffer = FindPropertyEdge(*g, fields["shortArray"], "buffer");
    int longBuffer = FindPropertyEdge(*g, fields["longArray"], "buffer");
    ASSERT_GE(shortBuffer, 0);
    ASSERT_GE(longBuffer, 0);
    EXPECT_LT(g->NodeSelfSize(shortBuffer), g->NodeSelfSize(longBuffer));

    ASSERT_NE(fields.find("shortText"), fields.end());
    ASSERT_NE(fields.find("longText"), fields.end());
    EXPECT_LT(g->NodeSelfSize(fields["shortText"]), g->NodeSelfSize(fields["longText"]));
}

// Primitive field values on specific instances must match the values assigned
// in main(). Number value nodes carry the value as their name string (floats
// and doubles are formatted to 6 fractional digits by the translator).
TEST_F(StaticDumpIntegrationTest, HeapsnapshotPrimitiveFieldValuesMatchEts)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);
    auto instances = CollectTestInstances(*g);

    // ap = new AllPrimitives(); ap.{z,b,s,i,j,f,d} = {true,127,32767,42,...,3.14,2.718281828}
    int ap = FindInstanceByNumField(*g, instances, "AllPrimitives", "i", "42");
    ASSERT_GE(ap, 0) << "AllPrimitives ap (i=42) not found";
    CheckNumField(*g, ap, "z", "true");
    CheckNumField(*g, ap, "b", "127");
    CheckNumField(*g, ap, "s", "32767");
    CheckNumField(*g, ap, "i", "42");
    CheckNumField(*g, ap, "j", "9007199254740991");
    CheckNumField(*g, ap, "f", "3.140000");
    CheckNumField(*g, ap, "d", "2.718282");

    // ap2 = new AllPrimitives(); ap2.i = 999 (rest default 0/false).
    int ap2 = FindInstanceByNumField(*g, instances, "AllPrimitives", "i", "999");
    ASSERT_GE(ap2, 0) << "AllPrimitives ap2 (i=999) not found";
    CheckNumField(*g, ap2, "z", "false");
    CheckNumField(*g, ap2, "b", "0");
    CheckNumField(*g, ap2, "i", "999");
    CheckNumField(*g, ap2, "d", "0.000000");

    // p1 = Person(25, 70.5, 175.0, 1, 100, 9999999999, "Alice", dog, true)
    int p1 = FindInstanceByNumField(*g, instances, "Person", "id", "100");
    ASSERT_GE(p1, 0) << "Person p1 (id=100) not found";
    CheckNumField(*g, p1, "age", "25");
    CheckNumField(*g, p1, "weight", "70.500000");
    CheckNumField(*g, p1, "height", "175.000000");
    CheckNumField(*g, p1, "flags", "1");
    CheckNumField(*g, p1, "id", "100");
    CheckNumField(*g, p1, "bigId", "9999999999");
    CheckNumField(*g, p1, "active", "true");

    // p2 = Person(30, 80.0, 180.0, 2, 200, 8888888888, "Bob", null, false)
    int p2 = FindInstanceByNumField(*g, instances, "Person", "id", "200");
    ASSERT_GE(p2, 0) << "Person p2 (id=200) not found";
    CheckNumField(*g, p2, "age", "30");
    CheckNumField(*g, p2, "weight", "80.000000");
    CheckNumField(*g, p2, "height", "180.000000");
    CheckNumField(*g, p2, "flags", "2");
    CheckNumField(*g, p2, "id", "200");
    CheckNumField(*g, p2, "bigId", "8888888888");
    CheckNumField(*g, p2, "active", "false");

    // base.id = 1 (set via init); leaf.score = 98.5 (set via setScore).
    int base = FindInstanceByNumField(*g, instances, "Base", "id", "1");
    ASSERT_GE(base, 0) << "Base instance (id=1) not found";
    CheckNumField(*g, base, "id", "1");
    int leaf = FirstInstance(instances, "Leaf");
    ASSERT_GE(leaf, 0) << "Leaf instance not found";
    CheckNumField(*g, leaf, "score", "98.500000");
}

// Reference-typed fields point to the right target: string fields -> a
// string-typed node NAMED BY THE STRING'S CONTENT (e.g. n1.name -> "alpha"),
// object fields -> the referenced instance (or std.core.Null for null), array
// fields -> std.core.Array. String content is carried in the .heapsnapshot via
// TAG_STATIC_STRING_DUMP, so the literal string value is asserted.
TEST_F(StaticDumpIntegrationTest, HeapsnapshotReferenceFieldsMatchEts)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);
    auto instances = CollectTestInstances(*g);

    // Node linked list: n1.name -> "alpha" (string node), n1.next -> n2 -> n3 -> Null.
    // n1 is the linked-list head: the Node whose `name` field is "alpha".
    // Locate it by that distinguishing field - INSTANCE_DUMP order is not
    // stable across builds (see FindInstanceByStringField).
    int n1 = FindInstanceByStringField(*g, instances, "Node", "name", "alpha");
    ASSERT_GE(n1, 0) << "Node n1 (name=alpha) not found";
    EXPECT_EQ(TargetNodeType(*g, n1, "name"), "string") << "n1.name target should be a string node";
    EXPECT_EQ(TargetNodeName(*g, n1, "name"), "alpha") << "n1.name should be the string 'alpha'";
    int n2 = FindPropertyEdge(*g, n1, "next");
    ASSERT_GE(n2, 0) << "n1.next missing";
    EXPECT_EQ(g->NodeName(n2), "StaticDumpTest.Node") << "n1.next should point to a Node";
    int n3 = FindPropertyEdge(*g, n2, "next");
    ASSERT_GE(n3, 0) << "n2.next missing";
    EXPECT_EQ(g->NodeName(n3), "StaticDumpTest.Node") << "n2.next should point to a Node";
    int n3next = FindPropertyEdge(*g, n3, "next");
    ASSERT_GE(n3next, 0) << "n3.next missing";
    EXPECT_EQ(g->NodeName(n3next), "std.core.Null") << "n3.next should be null";

    // p1.pet -> Dog instance; p2.pet -> Null.
    int p1 = FindInstanceByNumField(*g, instances, "Person", "id", "100");
    ASSERT_GE(p1, 0) << "Person p1 not found";
    EXPECT_EQ(TargetNodeName(*g, p1, "pet"), "StaticDumpTest.Dog") << "p1.pet should point to a Dog";
    // p1.name -> "Alice": the string field must resolve to a string-typed node
    // whose name is the real literal dumped from the ABC (Issue 3 fix surface).
    EXPECT_EQ(TargetNodeType(*g, p1, "name"), "string") << "p1.name target should be a string node";
    EXPECT_EQ(TargetNodeName(*g, p1, "name"), "Alice") << "p1.name should be the string 'Alice'";
    int p2 = FindInstanceByNumField(*g, instances, "Person", "id", "200");
    ASSERT_GE(p2, 0) << "Person p2 not found";
    EXPECT_EQ(TargetNodeName(*g, p2, "pet"), "std.core.Null") << "p2.pet should be null";
    EXPECT_EQ(TargetNodeType(*g, p2, "name"), "string") << "p2.name target should be a string node";
    EXPECT_EQ(TargetNodeName(*g, p2, "name"), "Bob") << "p2.name should be the string 'Bob'";

    // DataSet.values -> std.core.Array; FriendGroup.members -> std.core.Array.
    int ds = FirstInstance(instances, "DataSet");
    ASSERT_GE(ds, 0) << "DataSet instance not found";
    EXPECT_EQ(TargetNodeName(*g, ds, "values"), "std.core.Array");
    int fg = FirstInstance(instances, "FriendGroup");
    ASSERT_GE(fg, 0) << "FriendGroup instance not found";
    EXPECT_EQ(TargetNodeName(*g, fg, "members"), "std.core.Array");

    // base.name -> "base" (init(1,"base")); mid.tag -> "ran" (mid.run()).
    int base = FindInstanceByNumField(*g, instances, "Base", "id", "1");
    ASSERT_GE(base, 0) << "Base instance not found";
    EXPECT_EQ(TargetNodeType(*g, base, "name"), "string") << "base.name should be a string node";
    EXPECT_EQ(TargetNodeName(*g, base, "name"), "base") << "base.name should be 'base'";
    int mid = FirstInstance(instances, "Mid");
    ASSERT_GE(mid, 0) << "Mid instance not found";
    EXPECT_EQ(TargetNodeType(*g, mid, "tag"), "string") << "mid.tag should be a string node";
    EXPECT_EQ(TargetNodeName(*g, mid, "tag"), "ran") << "mid.tag should be 'ran'";
}

// Array field elements match the values assigned in main():
//   DataSet.values      = int[5] {23, 25, 21, 28, 30} - each wrapped in a
//                         std.core.Int node carrying a "value" -> number edge.
//   FriendGroup.members = Person[2] {p1, p2} - element edges point directly at
//                         the Person instance nodes.
TEST_F(StaticDumpIntegrationTest, HeapsnapshotArrayElementsMatchEts)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto instances = CollectTestInstances(*g);
    auto firstOf = [&](const std::string &cls) -> int {
        auto it = instances.find(cls);
        return (it != instances.end() && !it->second.empty()) ? it->second.front() : -1;
    };

    // DataSet.values: int array, elements wrapped in std.core.Int.
    int ds = firstOf("DataSet");
    ASSERT_GE(ds, 0) << "DataSet instance not found";
    int valuesArr = FindPropertyEdge(*g, ds, "values");
    ASSERT_GE(valuesArr, 0) << "DataSet.values missing";
    auto intElems = GetArrayElements(*g, valuesArr);
    ASSERT_EQ(intElems.size(), 5U) << "DataSet.values should have 5 elements";
    const std::vector<std::string> expectedInts = {"23", "25", "21", "28", "30"};
    for (size_t k = 0; k < intElems.size(); ++k) {
        EXPECT_EQ(g->NodeName(intElems[k]), "std.core.Int") << "values[" << k << "] is not a std.core.Int wrapper";
        int valNode = FindPropertyEdge(*g, intElems[k], "value");
        ASSERT_GE(valNode, 0) << "values[" << k << "] Int wrapper has no 'value' edge";
        EXPECT_EQ(g->NodeType(valNode), "number");
        EXPECT_EQ(g->NodeName(valNode), expectedInts[k]) << "values[" << k << "] value";
    }

    // FriendGroup.members: object array, elements are Person instances.
    int fg = firstOf("FriendGroup");
    ASSERT_GE(fg, 0) << "FriendGroup instance not found";
    int membersArr = FindPropertyEdge(*g, fg, "members");
    ASSERT_GE(membersArr, 0) << "FriendGroup.members missing";
    auto objElems = GetArrayElements(*g, membersArr);
    ASSERT_EQ(objElems.size(), 2U) << "FriendGroup.members should have 2 elements";
    for (int idx : objElems) {
        EXPECT_EQ(g->NodeName(idx), "StaticDumpTest.Person") << "members element is not a Person instance";
    }
}

// ============================================================================
// Regression guards for the 5 static-snapshot issues. Each maps a defect to a
// graph-level assertion so a regression is caught at the graph, not the bytes.
// ============================================================================

// Issue 1: user-visible objects must be reachable from the SyntheticRoot. The
// dumper's BFS only emits root-reachable objects; the historical bug was that
// the writer emitted only own instance fields, so objects reachable via an
// inherited reference field (e.g. Dog.species "dog") were dumped as instances
// with no incoming edge. This guard asserts every StaticDumpTest.* instance
// and every string literal assigned in the test ABC is reachable.
// (Stdlib-internal objects - Maps/Sets/Arrays/Mutexes/wrapper instances that
// form disconnected components - are a separate, pre-existing dumper
// root-coverage issue and are NOT asserted here. The total orphan count is
// printed for visibility and bounded as a regression guard.)
TEST_F(StaticDumpIntegrationTest, HeapsnapshotNoOrphanNodes)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto reachable = g->ReachableFrom(0);  // node 0 == SyntheticRoot

    // Every user (StaticDumpTest.*) instance must be reachable.
    int userOrphans = 0;
    for (int i = 0; i < g->NodeCount(); ++i) {
        if (g->NodeType(i) != "object") {
            continue;
        }
        if (g->NodeName(i).find("StaticDumpTest.") == 0) {
            if (!reachable[i]) {
                ++userOrphans;
            }
        }
    }
    EXPECT_EQ(userOrphans, 0) << userOrphans << " StaticDumpTest.* instance(s) are unreachable";

    // Every string literal assigned in the test ABC must be reachable.
    const std::vector<std::string> literals = {"alpha", "beta",   "gamma",        "Alice",   "Bob",  "dog", "beagle",
                                               "cat",   "orange", "temperatures", "friends", "base", "mid", "leaf"};
    int unreachableLiterals = 0;
    for (const auto &lit : literals) {
        for (int i = 0; i < g->NodeCount(); ++i) {
            if (g->NodeType(i) == "string" && g->NodeName(i) == lit && !reachable[i]) {
                ++unreachableLiterals;
                break;
            }
        }
    }
    EXPECT_EQ(unreachableLiterals, 0) << unreachableLiterals << " test-ABC string literal(s) unreachable";

    // Regression guard: total orphans bounded (stdlib-internal residual).
    int orphans = 0;
    for (int i = 0; i < g->NodeCount(); ++i) {
        if (!reachable[i]) {
            ++orphans;
        }
    }
    EXPECT_LE(orphans, ORPHAN_REGRESSION_CEILING)
        << "orphan count regressed past " << ORPHAN_REGRESSION_CEILING << ": " << orphans;
}

// Issue 1 targeted guard: inherited instance fields are emitted with correct
// values. Dog (extends Animal) has species="dog"/age=3 inherited from Animal;
// Leaf (extends Mid extends Base) has id/name from Base and tag from Mid.
TEST_F(StaticDumpIntegrationTest, HeapsnapshotInheritedFieldsPresent)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto instances = CollectTestInstances(*g);
    auto firstOf = [&](const std::string &cls) -> int {
        auto it = instances.find(cls);
        return (it != instances.end() && !it->second.empty()) ? it->second.front() : -1;
    };
    auto numField = [&](int idx, const std::string &f) -> std::string {
        auto e = GetPropertyEdges(*g, idx);
        auto it = e.find(f);
        return (it != e.end()) ? g->NodeName(it->second) : std::string("<missing>");
    };

    // dog = new Dog("dog", 3, "beagle"): species="dog", age=3 (inherited from Animal).
    int dog = firstOf("Dog");
    ASSERT_GE(dog, 0) << "Dog instance not found";
    EXPECT_EQ(g->NodeType(FindPropertyEdge(*g, dog, "species")), "string");
    EXPECT_EQ(numField(dog, "species"), "dog") << "Dog.species (inherited) value";
    EXPECT_EQ(numField(dog, "age"), "3") << "Dog.age (inherited) value";

    // leaf = new Leaf(); leaf.init(3,"leaf"): id=3 (Base), name="leaf" (Base),
    // tag="" default then... Mid.run() sets tag="ran", but leaf.run() is not
    // called, so leaf.tag stays "". Assert the inherited fields EXIST.
    int leaf = firstOf("Leaf");
    ASSERT_GE(leaf, 0) << "Leaf instance not found";
    EXPECT_GE(FindPropertyEdge(*g, leaf, "id"), 0) << "Leaf.id (inherited from Base) missing";
    EXPECT_GE(FindPropertyEdge(*g, leaf, "name"), 0) << "Leaf.name (inherited from Base) missing";
    EXPECT_GE(FindPropertyEdge(*g, leaf, "tag"), 0) << "Leaf.tag (inherited from Mid) missing";
    EXPECT_EQ(numField(leaf, "id"), "3");
    EXPECT_EQ(numField(leaf, "name"), "leaf");
}

// Issue 2: no class node may have two superClass edges to the same target.
// (The historical bug: CreateInstanceEdges processed the class's mirror
// instance record, emitting a [property] superClass edge that duplicated
// EmitSuperClassEdge's [internal] superClass edge.)
TEST_F(StaticDumpIntegrationTest, HeapsnapshotNoDuplicateSuperClassEdges)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    int duplicates = 0;
    int firstDupNode = -1;
    for (int i = 0; i < g->NodeCount(); ++i) {
        if (g->NodeType(i) != "class") {
            continue;
        }
        int nodeDups = CountSuperClassDuplicates(*g, i);
        if (nodeDups > 0) {
            if (duplicates == 0) {
                firstDupNode = i;
            }
            duplicates += nodeDups;
        }
    }
    EXPECT_EQ(duplicates, 0) << "class node " << firstDupNode << " ('" << g->NodeName(firstDupNode)
                             << "') has duplicate superClass edges to the same target";
}

// Issue 3: string content is visible. At least one string-typed node exists
// whose name is a real literal from the test ABC. (The historical bug: string
// objects showed as type=object named "std.core.String" with no visible text.)
TEST_F(StaticDumpIntegrationTest, HeapsnapshotStringContentVisible)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    // One pass over nodes collecting string-typed node names (exact).
    std::set<std::string> stringNames;
    for (int i = 0; i < g->NodeCount(); ++i) {
        if (g->NodeType(i) == "string") {
            stringNames.insert(g->NodeName(i));
        }
    }
    EXPECT_FALSE(stringNames.empty()) << "no string-typed nodes in heapsnapshot";
    EXPECT_EQ(stringNames.count("中文😀"), 1U) << "UTF-16 string content must be preserved as UTF-8";

    // The test ABC assigns these literals to string fields; each must appear as
    // a string-typed node named exactly by its content.
    const std::vector<std::string> literals = {"alpha", "beta",   "gamma",        "Alice",   "Bob",  "dog", "beagle",
                                               "cat",   "orange", "temperatures", "friends", "base", "mid", "ran"};
    int found = 0;
    for (const auto &lit : literals) {
        if (stringNames.count(lit) > 0) {
            ++found;
        }
    }
    EXPECT_GE(found, MIN_STRING_CONTENT_NODES)
        << "expected at least " << MIN_STRING_CONTENT_NODES << " string-typed nodes with real content; found " << found;
}

// Issue 4: closure (method) nodes are reachable leaves with 0 outgoing edges.
// Documents the by-design behavior - the static dump carries no method-internal
// data (no code/context), so each declared method is a synthetic closure node
// reachable via its class's PROPERTY edge, with no outgoing edges of its own.
TEST_F(StaticDumpIntegrationTest, HeapsnapshotClosureNodesAreReachableLeaves)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto reachable = g->ReachableFrom(0);
    int total = 0;
    int unreachable = 0;
    int nonLeaf = 0;
    for (int i = 0; i < g->NodeCount(); ++i) {
        if (g->NodeType(i) != "closure") {
            continue;
        }
        ++total;
        if (!reachable[i]) {
            ++unreachable;
        }
        if (g->NodeEdgeCount(i) != 0) {
            ++nonLeaf;
        }
    }
    EXPECT_GT(total, 0) << "no closure nodes found";
    EXPECT_EQ(unreachable, 0) << unreachable << "/" << total << " closure nodes are unreachable";
    EXPECT_EQ(nonLeaf, 0) << nonLeaf << "/" << total << " closure nodes have outgoing edges (expected 0)";
}

// Issue 5: the snapshot.meta edge_types declaration must include "xref" so that
// any cross-VM xref edge emitted by the hybrid merger carries a valid type.
// (Single-file static output has 0 xref edges by design - no dynamic side -
// but the meta must still declare the type.)
TEST_F(StaticDumpIntegrationTest, HeapsnapshotEdgeTypesMetaIncludesXref)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    EXPECT_TRUE(g->EdgeTypeIsDeclared("xref")) << "snapshot.meta.edge_types must declare 'xref' (EdgeType::XREF=7)";

    // Single-file static output has no dynamic side -> 0 xref edges.
    int xrefEdges = 0;
    for (int j = 0; j < g->EdgeCount(); ++j) {
        if (g->EdgeTypeName(j) == "xref") {
            ++xrefEdges;
        }
    }
    EXPECT_EQ(xrefEdges, 0) << "single-file static output should have 0 xref edges";
}

// Circular references (A.friend=B, B.friend=A) must not make object-graph BFS infinite-loop.
// The static dump's reachability BFS uses a visited-set to handle circular references.
// This test verifies both CircularFriendA and CircularFriendB instances are reachable
// and their friend edges point to each other (no crash, no infinite loop).
TEST_F(StaticDumpIntegrationTest, HeapsnapshotCircularReference_NoInfiniteLoop)
{
    auto g = DumpAndTranslate();
    ASSERT_NE(g, nullptr);

    auto instances = CollectTestInstances(*g);

    // Find CircularFriendA and CircularFriendB instances
    auto aIt = instances.find("CircularFriendA");
    auto bIt = instances.find("CircularFriendB");
    ASSERT_TRUE(aIt != instances.end() && !aIt->second.empty()) << "CircularFriendA instance not found";
    ASSERT_TRUE(bIt != instances.end() && !bIt->second.empty()) << "CircularFriendB instance not found";

    int friendA = aIt->second.front();
    int friendB = bIt->second.front();

    // Verify A.name == "Alice" (string field)
    auto edgesA = GetPropertyEdges(*g, friendA);
    auto nameAIt = edgesA.find("name");
    ASSERT_NE(nameAIt, edgesA.end()) << "CircularFriendA.name missing";
    EXPECT_EQ(g->NodeType(nameAIt->second), "string");
    EXPECT_EQ(g->NodeName(nameAIt->second), "Alice");

    // Verify A.friend -> CircularFriendB
    auto friendAIt = edgesA.find("friend");
    ASSERT_NE(friendAIt, edgesA.end()) << "CircularFriendA.friend missing";
    EXPECT_EQ(g->NodeName(friendAIt->second), "StaticDumpTest.CircularFriendB");

    // Verify B.name == "Bob"
    auto edgesB = GetPropertyEdges(*g, friendB);
    auto nameBIt = edgesB.find("name");
    ASSERT_NE(nameBIt, edgesB.end()) << "CircularFriendB.name missing";
    EXPECT_EQ(g->NodeType(nameBIt->second), "string");
    EXPECT_EQ(g->NodeName(nameBIt->second), "Bob");

    // Verify B.friend -> CircularFriendA (circular back-link)
    auto friendBIt = edgesB.find("friend");
    ASSERT_NE(friendBIt, edgesB.end()) << "CircularFriendB.friend missing";
    EXPECT_EQ(g->NodeName(friendBIt->second), "StaticDumpTest.CircularFriendA");

    // Both must be reachable from SyntheticRoot (BFS visited-set prevents infinite loop)
    auto reachable = g->ReachableFrom(0);
    EXPECT_TRUE(reachable[friendA]) << "CircularFriendA is unreachable";
    EXPECT_TRUE(reachable[friendB]) << "CircularFriendB is unreachable";
}

#endif  // STATIC_DUMP_TEST_ABC_DIR

}  // namespace ark::tooling::hprof
