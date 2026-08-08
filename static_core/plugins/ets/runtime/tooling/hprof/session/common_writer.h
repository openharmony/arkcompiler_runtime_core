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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_COMMON_WRITER_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_COMMON_WRITER_H

#include "plugins/ets/runtime/tooling/hprof/session/abstract_writer.h"
#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"

#include <cstdint>

namespace ark::tooling::hprof {

class StringIdPool;

/**
 * @brief CommonWriter - extends AbstractWriter to write the single-threaded
 * serial sections of a hybrid dump: the file header (HybridDumpHeader),
 * string pool (STRING_IN_UTF8), XRef edges, and trailing heap summary.
 *
 * Sibling leaf: StaticWriter writes the static-side record item bodies
 * (ROOT / LOAD_CLASS / STATIC_CLASS_DUMP / STATIC_INSTANCE_DUMP /
 * STATIC_ARRAY_DUMP / STATIC_STRING_DUMP); both derive from AbstractWriter
 * and share its batching mechanism.
 *
 * HeapDumpCoordinator uses CommonWriter for the steps that run serially - in
 * particular the XRef + heap summary appended to the static stream AFTER both
 * VM lifecycles complete (XRef resolves dynNodeId from the dynamic
 * entryIdMap, populated during the dynamic Execute).
 *
 * WriteStringPool and WriteHeapSummary are self-contained (they wrap
 * BeginRecord / EndRecord internally); WriteXRefEdge is wrapped by the caller.
 * Per-item layouts are on each method; the file-level format (header, record
 * envelope, tag catalog, emission order) is in dump_format.h.
 */
class CommonWriter : public AbstractWriter {
public:
    explicit CommonWriter(OutputStream *stream, size_t bufferSize = DEFAULT_BUFFER_SIZE);
    ~CommonWriter() override;

    // Non-copyable, non-movable (Rule-of-5: user-declared dtor).
    CommonWriter(const CommonWriter &) = delete;
    CommonWriter &operator=(const CommonWriter &) = delete;
    CommonWriter(CommonWriter &&) = delete;
    CommonWriter &operator=(CommonWriter &&) = delete;

    // -- V3 serial-section convenience methods --

    /**
     * Write the hybrid dump file header (33 bytes, HybridDumpHeader):
     *
     *   +---------+---------------+----------+--------+--------+--------+--------+
     *   |version  |identifierSize |timestamp |language|headerSi|recordCt|featureF|
     *   |  8 B    |     4 B       |   8 B    |  1 B   |  4 B   |  4 B   |  4 B   |
     *   +---------+---------------+----------+--------+--------+--------+--------+
     *    "3.0.0\0\0\0"              wall ms    STATIC=1  =33     summary  =0
     *                                          /HYBRID=2        (totObj+
     *                                                           totClass)
     *
     * @param language    Language identifier: Language::DYNAMIC/STATIC/HYBRID.
     * @param totalObj    Total live object count across all virtual machines.
     * @param totalClass  Total unique class count across all virtual machines.
     */
    void WriteFileHeader(Language language, uint64_t totalObj, uint64_t totalClass);

    /**
     * Write all STRING_IN_UTF8 items as a batched record.
     * Self-contained: calls BeginRecord(TAG_STRING_IN_UTF8), iterates the
     * pool writing each string item via WriteStringItem (which calls
     * FinishItem), and finally EndRecord(). If the buffer exceeds the
     * flush threshold mid-stream, FinishItem auto-flushes and starts a
     * new record for the same tag.
     *
     * Each item body
     *
     *   +----------+----------+-----------------+
     *   | stringId | length   | utf8Data(length)|
     *   |   u4     |   u4     |     bytes       |
     *   +----------+----------+-----------------+
     *
     * @param pool  The frozen StringIdPool to serialize.
     */
    void WriteStringPool(StringIdPool *pool);

    /**
     * Write a complete HEAP_SUMMARY record (tag TAG_HEAP_SUMMARY, count=1).
     * Self-contained: BeginRecord + 7 u64 fields + FinishItem + EndRecord.
     *
     * Body (one item, 7 x u64 = 56 bytes):
     *
     *   +------------+--------------+----------+------------+------------+-----------+----------+
     *   |liveBytes=0 |liveInstances | alloc=0  |instAlloc=0 |staticObjCt |dynObjCt   |classCount|
     *   |    u8      |     u8       |   u8     |    u8      |    u8      |    u8     |    u8    |
     *   +------------+--------------+----------+------------+------------+-----------+----------+
     *
     * Note: totalLiveBytes, totalAllocated, and totalInstancesAllocated are
     * currently not computed and written as 0 (reserved for future use).
     *
     * @param totalObj        Total live instances across all virtual machines.
     * @param totalClass      Total unique classes across all virtual machines.
     * @param dynamicObjCount Live instances from the dynamic VM.
     * @param staticObjCount  Live instances from the static VM.
     */
    void WriteHeapSummary(uint64_t totalObj, uint64_t totalClass, size_t dynamicObjCount, size_t staticObjCount);

    /**
     * Write one XRef edge item body + FinishItem. The caller must wrap
     * this with BeginRecord(TAG_XREF_EDGE) and EndRecord(), calling
     * WriteXRefEdge for each edge in a loop. This enables batching
     * multiple edges into a single record.
     *
     * Item body (9 bytes)
     *
     *   +----------+----------+-----------+
     *   | fromAddr | toAddr   | direction |
     *   |   u4     |   u4     |     u1    |
     *   +----------+----------+-----------+
     *    dynNodeId  staNodeId
     *
     * @param fromAddr    Dynamic-side nodeId (4 bytes; the JS side of an XRef,
     *                    resolved from the JS heap address via
     *                    the dynamic participant's GetNodeId at dump time).
     * @param toAddr      Static-side nodeId (4 bytes, the ETS side of an XRef).
     * @param direction   XREF_DIR_DYN_TO_STA / XREF_DIR_STA_TO_DYN / XREF_DIR_BIDIR.
     */
    void WriteXRefEdge(uint32_t fromAddr, uint32_t toAddr, uint8_t direction);

private:
    /**
     * Write one STRING_IN_UTF8 item body + FinishItem.
     * Called by WriteStringPool for each string in the pool.
     */
    void WriteStringItem(uint32_t id, uint32_t length, const uint8_t *data, size_t dataSize);
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_COMMON_WRITER_H
