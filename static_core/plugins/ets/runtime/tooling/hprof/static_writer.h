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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_STATIC_WRITER_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_STATIC_WRITER_H

#include "plugins/ets/runtime/tooling/hprof/session/abstract_writer.h"
#include "libarkbase/macros.h"

#include <cstdint>

namespace ark::tooling::hprof {

/**
 * @brief Field descriptor data for CLASS_DUMP items.
 *
 * Each entry carries the four fields of a wire-format field descriptor:
 * nameId, type, offset, and flags. The StaticDump caller pre-collects
 * field data into arrays of ClassFieldData, then passes them to
 * WriteClassDumpItem which writes the complete item body in one call.
 */
struct ClassFieldData {
    uint32_t nameId;
    uint8_t type;
    uint32_t offset;
    uint16_t flags;
};

/**
 * @brief Field value data for INSTANCE_DUMP items.
 *
 * Each entry carries the runtime value type and its payload. Primitive values
 * use their natural width, OBJECT/ARRAY/WEAK_OBJECT use a u32 nodeId, and
 * TAGGED special values preserve the original u64 tagged payload.
 */
struct FieldValueData {
    uint8_t type;
    uint64_t value;
};

/**
 * @brief StaticWriter - extends AbstractWriter to write the static-side record
 * item bodies: ROOT, LOAD_CLASS, STATIC_CLASS_DUMP, STATIC_INSTANCE_DUMP,
 * STATIC_ARRAY_DUMP, STATIC_STRING_DUMP.
 *
 * Sibling leaf: CommonWriter writes the serial sections (file header, string
 * pool, XRef, heap summary); both derive from AbstractWriter and share its
 * batching mechanism. StaticDump uses StaticWriter during Execute
 * (instance / array / string records) and Finalize (CLASS_DUMP records).
 *
 * Each WriteXxxItem call writes ONE complete item body and FinishItem(); the
 * caller wraps sequences with BeginRecord(tag) / EndRecord(). The per-item
 * byte layout of each record type is documented on the corresponding method
 * below; the file-level format (header, record envelope, tag catalog,
 * emission order) is in dump_format.h.
 */
class StaticWriter : public AbstractWriter {
public:
    explicit StaticWriter(OutputStream *stream, size_t bufferSize = DEFAULT_BUFFER_SIZE);
    ~StaticWriter() override;

    NO_COPY_SEMANTIC(StaticWriter);
    NO_MOVE_SEMANTIC(StaticWriter);

    // -- Item-level methods (caller wraps with BeginRecord/EndRecord) --

    /**
     * Write one ROOT item body (tag TAG_ROOT_RECORD, 5 bytes) + FinishItem.
     *
     *   +---------+---------+
     *   |rootType | objectId|
     *   |  u8 =0  |   u4    |
     *   +---------+---------+
     *
     * @param objectId  nodeId of the static root object.
     */
    void WriteRootItem(uint32_t objectId);

    /**
     * Write one LOAD_CLASS item body (tag TAG_LOAD_CLASS, 21 bytes) + FinishItem.
     *
     *   +-------+----------+----------+--------+-------+-------+
     *   |serial |classObjId|stackTrace| nameId |lang   |flags  |
     *   |  u4   |    u4    |   u4 =0  |   u4   |u1 =1  |  u4   |
     *   +-------+----------+----------+--------+-------+-------+
     *
     * @param classSerial    Class serial number (1-based position in this record).
     * @param classObjectId  nodeId of the class's representative object.
     * @param classNameId    String-pool id of the class name.
     * @param classFlags     Class flags (ClassFlags bitmask).
     */
    void WriteLoadClassItem(uint32_t classSerial, uint32_t classObjectId, uint32_t classNameId, uint32_t classFlags);
    /**
     * Write one complete STATIC_CLASS_DUMP item body (tag
     * TAG_STATIC_CLASS_DUMP) + FinishItem. Self-contained: accepts all field
     * data as pre-collected arrays, no external EndItem call needed.
     *
     *   22-byte prefix
     *     +----------+----------+---------+---------+---------+------------+
     *     |classObjId|stackTrace|superId  |loaderId |instSize |staticCount |
     *     |    u4    |   u4 =0  |   u4    |  u4 =0  |   u4    |    u2      |
     *     +----------+----------+---------+---------+---------+------------+
     *
     *   Variable tail:
     *     [staticCount      x FieldDescriptor(11)]            see WriteFieldDescriptor
     *     [instanceCount   : u2] -> instanceCount   x FieldDescriptor(11)
     *     [staticValueCount: u2] -> staticValueCount x FieldValue (parallel to
     *                               the static-field descriptors above; a class
     *                               with no mirror object passes = 0)
     *     [methodCount     : u2] -> methodCount     x methodNameId(u4)
     *
     *   FieldDescriptor (11 bytes)
     *     +--------+------+--------+-------+
     *     |nameId  |type  |offset  |flags  |
     *     |  u4    |  u1  |   u4   |  u2   |
     *     +--------+------+--------+-------+
     *
     * The static-value section carries one FieldValueData per static field, in
     * the SAME order as the static-field descriptors above (so the reader can
     * label each value with its descriptor's nameId/type). The method section
     * carries one methodNameId (dump string-pool index) per declared method.
     *
     * @param classObjectId       nodeId of the class's representative object.
     * @param superClassId        nodeId of the superclass representative (0 if none).
     * @param instanceSize        Instance size in bytes.
     * @param staticFields        Array of static field descriptors.
     * @param staticCount         Number of static field descriptors.
     * @param instanceFields      Array of instance field descriptors.
     * @param instanceCount       Number of instance field descriptors.
     * @param staticValues        Array of static field values (parallel to staticFields).
     * @param staticValueCount    Number of static field values (<= staticCount).
     * @param methodNameIds       Array of method-name string-pool ids.
     * @param methodCount         Number of methods.
     */
    void WriteClassDumpItem(uint32_t classObjectId, uint32_t superClassId, uint32_t instanceSize,
                            const ClassFieldData *staticFields, uint16_t staticCount,
                            const ClassFieldData *instanceFields, uint16_t instanceCount,
                            const FieldValueData *staticValues, uint16_t staticValueCount,
                            const uint32_t *methodNameIds, uint16_t methodCount);

    /**
     * Write one complete STATIC_INSTANCE_DUMP item body (tag
     * TAG_STATIC_INSTANCE_DUMP) + FinishItem. Self-contained: accepts all
     * field value data as a pre-collected array, no external EndItem call
     * needed.
     *
     *   18-byte prefix
     *     +--------+----------+----------+---------+-----------+
     *     | objId  |classObjId|stackTrace|instSize |fieldCount |
     *     |   u4   |    u4    |   u4 =0  |   u4    |    u2     |
     *     +--------+----------+----------+---------+-----------+
     *
     *   Variable tail:
     *     [fieldCount x FieldValue]                  see WriteFieldValue
     *
     *   FieldValue
     *     +------+-------------------+
     *     |type  | value (variable)  |   OBJECT/ARRAY value = 4-byte nodeId
     *     |  u1  |                   |
     *     +------+-------------------+
     *
     * @param objectId       nodeId of the object.
     * @param classObjectId  nodeId of the object's class representative.
     * @param instanceSize   Instance size in bytes.
     * @param fields         Array of field value data (type + value per field).
     * @param fieldCount     Number of field values.
     */
    void WriteInstanceDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                               const FieldValueData *fields, uint16_t fieldCount);

    /**
     * Write one STATIC_ARRAY_DUMP item body (tag TAG_STATIC_ARRAY_DUMP)
     * + FinishItem. For zero-length arrays, only the prefix is written.
     *
     *   21-byte prefix
     *     +--------+----------+----------+---------+---------+---------+
     *     | objId  |classObjId|stackTrace|instSize |arrayLen |elemType |
     *     |   u4   |    u4    |   u4 =0  |   u4    |   u4    |   u1    |
     *     +--------+----------+----------+---------+---------+---------+
     *
     *   Variable tail:
     *     [raw fixed-width element data if arrayLen > 0]
     *
     * Any[] arrays use WriteTaggedArrayDumpItem because their elements have
     * variable-width, runtime-typed payloads.
     *
     * @param objectId       nodeId of the array object.
     * @param classObjectId  nodeId of the array's class representative.
     * @param instanceSize   Instance size in bytes.
     * @param arrayLength    Number of elements (may be 0).
     * @param elementType    Element type (FieldType).
     * @param data           Raw element data buffer (may be null if arrayLength=0).
     * @param dataSize       Size of `data` in bytes.
     */
    void WriteArrayDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize, uint32_t arrayLength,
                            uint8_t elementType, const uint8_t *data, size_t dataSize);

    /**
     * Write one TAGGED STATIC_ARRAY_DUMP item.
     *
     * The common 21-byte array prefix declares elementType=TAGGED. The tail
     * contains exactly arrayLength FieldValue entries, each encoded as
     * [runtimeType:u1][payload:variable]. This keeps the declared Any[] type
     * separate from the runtime type of each element.
     */
    void WriteTaggedArrayDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                                  const FieldValueData *elements, uint32_t arrayLength);

    /**
     * Write one STATIC_STRING_DUMP item body (tag TAG_STATIC_STRING_DUMP)
     * + FinishItem.
     *
     * String objects carry their UTF-8 content so the translator can emit a
     * STRING-typed node named by the content (an INSTANCE_DUMP has no
     * per-instance name field, so plain string instances would otherwise show
     * as objects named after their class with no visible value).
     *
     *   16-byte prefix
     *     +--------+----------+---------+---------+
     *     | objId  |classObjId|instSize | valueLen|
     *     |   u4   |    u4    |   u4    |   u4    |
     *     +--------+----------+---------+---------+
     *
     *   Variable tail:
     *     [valueLen bytes of UTF-8 content]
     *
     * @param objectId       nodeId of the string object.
     * @param classObjectId  nodeId of the string's class representative.
     * @param instanceSize   Instance size in bytes.
     * @param valueBytes     UTF-8 content buffer (may be null if valueLen=0).
     * @param valueLen       Length of `valueBytes` in bytes.
     */
    void WriteStringDumpItem(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize,
                             const uint8_t *valueBytes, uint32_t valueLen);

private:
    // -- Internal helpers (only called by public WriteXXXItem methods) --

    /** Write one field descriptor (11 bytes) inside a CLASS_DUMP item. */
    void WriteFieldDescriptor(uint32_t nameId, uint8_t type, uint32_t offset, uint16_t flags);

    /** Write one field value inside an INSTANCE_DUMP, CLASS_DUMP, or tagged ARRAY_DUMP item. */
    void WriteFieldValue(uint8_t type, uint64_t value);

    /** Write the common 21-byte ARRAY_DUMP prefix. */
    void WriteArrayPrefix(uint32_t objectId, uint32_t classObjectId, uint32_t instanceSize, uint32_t arrayLength,
                          uint8_t elementType);

    /** Write raw array data inside an ARRAY_DUMP item. */
    void WriteArrayData(const uint8_t *data, size_t size);
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_STATIC_WRITER_H
