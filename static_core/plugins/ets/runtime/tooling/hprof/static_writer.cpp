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

#include "plugins/ets/runtime/tooling/hprof/static_writer.h"
#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"
#include "libarkbase/utils/span.h"

namespace ark::tooling::hprof {

StaticWriter::StaticWriter(OutputStream *stream, size_t bufferSize) : AbstractWriter(stream, bufferSize) {}

StaticWriter::~StaticWriter() = default;

// -- Item-level methods (caller wraps with BeginRecord/EndRecord) --

void StaticWriter::WriteRootItem(uint32_t objectId)
{
    WriteU8(static_cast<uint8_t>(RootType::STATIC_OBJECT));
    WriteU32(objectId);
    FinishItem();
}

void StaticWriter::WriteLoadClassItem(uint32_t classSerial, uint32_t classObjectId, uint32_t classNameId,
                                      uint32_t classFlags)
{
    WriteU32(classSerial);
    WriteU32(classObjectId);
    WriteU32(0);  // stackTraceSerial = 0
    WriteU32(classNameId);
    WriteU8(static_cast<uint8_t>(Language::STATIC));
    WriteU32(classFlags);
    FinishItem();
}

void StaticWriter::WriteClassDumpItem(uint32_t classObjectId, uint32_t superClassId, uint32_t instanceSize,
                                      const ClassFieldData *staticFields, uint16_t staticCount,
                                      const ClassFieldData *instanceFields, uint16_t instanceCount,
                                      const FieldValueData *staticValues, uint16_t staticValueCount,
                                      const uint32_t *methodNameIds, uint16_t methodCount)
{
    // Fixed prefix (22 bytes)
    WriteU32(classObjectId);
    WriteU32(0);  // stackTraceSerial = 0
    WriteU32(superClassId);
    WriteU32(0);  // classLoaderId = 0
    WriteU32(instanceSize);

    // Static field descriptors
    WriteU16(staticCount);
    for (const auto &field : Span<const ClassFieldData>(staticFields, staticCount)) {
        WriteFieldDescriptor(field.nameId, field.type, field.offset, field.flags);
    }

    // Instance field descriptors
    WriteU16(instanceCount);
    for (const auto &field : Span<const ClassFieldData>(instanceFields, instanceCount)) {
        WriteFieldDescriptor(field.nameId, field.type, field.offset, field.flags);
    }

    // Static field values (parallel to static field descriptors, same order).
    // Each entry is [type:u1][value:variable] - identical to INSTANCE_DUMP's
    // field-value encoding, reusing WriteFieldValue.
    WriteU16(staticValueCount);
    for (const auto &field : Span<const FieldValueData>(staticValues, staticValueCount)) {
        WriteFieldValue(field.type, field.value);
    }

    // Declared method name ids (dump string-pool indices).
    WriteU16(methodCount);
    for (auto methodNameId : Span<const uint32_t>(methodNameIds, methodCount)) {
        WriteU32(methodNameId);
    }

    FinishItem();  // Complete this CLASS_DUMP item
}

void StaticWriter::WriteInstanceDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                                         const FieldValueData *fields, uint16_t fieldCount)
{
    // Fixed prefix (18 bytes)
    WriteU32(objectId);
    WriteU32(classObjectId);
    WriteU32(0);  // stackTraceSerial = 0
    WriteU32(instanceSize);
    WriteU16(fieldCount);

    // Field values
    for (const auto &field : Span<const FieldValueData>(fields, fieldCount)) {
        WriteFieldValue(field.type, field.value);
    }

    FinishItem();  // Complete this INSTANCE_DUMP item
}

void StaticWriter::WriteArrayDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                                      uint32_t arrayLength, uint8_t elementType, const uint8_t *data, size_t dataSize)
{
    // Fixed prefix (21 bytes) + optional raw data
    WriteArrayPrefix(objectId, classObjectId, instanceSize, arrayLength, elementType);
    if (data != nullptr && dataSize > 0) {
        WriteArrayData(data, dataSize);
    }
    FinishItem();
}

void StaticWriter::WriteTaggedArrayDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                                            const FieldValueData *elements, uint32_t arrayLength)
{
    WriteArrayPrefix(objectId, classObjectId, instanceSize, arrayLength, static_cast<uint8_t>(FieldType::TAGGED));
    for (const auto &element : Span<const FieldValueData>(elements, arrayLength)) {
        WriteFieldValue(element.type, element.value);
    }
    FinishItem();
}

// -- Internal helpers --

void StaticWriter::WriteFieldDescriptor(uint32_t nameId, uint8_t type, uint32_t offset, uint16_t flags)
{
    WriteU32(nameId);
    WriteU8(type);
    WriteU32(offset);
    WriteU16(flags);
}

void StaticWriter::WriteFieldValue(uint8_t type, uint64_t value)
{
    WriteU8(type);
    switch (static_cast<FieldType>(type)) {
        case FieldType::BOOLEAN:
        case FieldType::BYTE:
            WriteU8(static_cast<uint8_t>(value));
            break;
        case FieldType::CHAR:
        case FieldType::SHORT:
            WriteU16(static_cast<uint16_t>(value));
            break;
        case FieldType::INT:
        case FieldType::FLOAT:
            WriteU32(static_cast<uint32_t>(value));
            break;
        case FieldType::LONG:
        case FieldType::DOUBLE:
        case FieldType::TAGGED:
            WriteU64(value);
            break;
        case FieldType::OBJECT:
        case FieldType::ARRAY:
        case FieldType::WEAK_OBJECT:
            WriteU32(static_cast<uint32_t>(value));  // nodeId (4 bytes)
            break;
        case FieldType::UNKNOWN:
            // Only wrote type byte, no value bytes.
            break;
    }
}

void StaticWriter::WriteArrayPrefix(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                                    uint32_t arrayLength, uint8_t elementType)
{
    WriteU32(objectId);
    WriteU32(classObjectId);
    WriteU32(0);  // stackTraceSerial = 0
    WriteU32(instanceSize);
    WriteU32(arrayLength);
    WriteU8(elementType);
}

void StaticWriter::WriteArrayData(const uint8_t *data, size_t size)
{
    WriteBytes(data, size);
}

void StaticWriter::WriteStringDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                                       const uint8_t *valueBytes, uint32_t valueLen)
{
    // Fixed prefix (16 bytes) + valueLen bytes of UTF-8 content.
    WriteU32(objectId);
    WriteU32(classObjectId);
    WriteU32(instanceSize);
    WriteU32(valueLen);
    if (valueBytes != nullptr && valueLen > 0) {
        WriteArrayData(valueBytes, valueLen);
    }
    FinishItem();  // Complete this STATIC_STRING_DUMP item
}

}  // namespace ark::tooling::hprof
