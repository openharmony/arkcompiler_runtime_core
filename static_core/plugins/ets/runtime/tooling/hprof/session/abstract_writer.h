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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_ABSTRACT_WRITER_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_ABSTRACT_WRITER_H

#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"

#include <cstdint>
#include <vector>

namespace ark::tooling::hprof {

class OutputStream;  // forward declaration - full definition in output_stream.h

/**
 * @brief AbstractWriter - base class for all hybrid-dump binary record writers.
 *
 * Owns the record-batching buffer and the BeginRecord / FinishItem / EndRecord
 * mechanism every writer shares. A record = 17-byte header (tag + time +
 * length + count, see dump_format.h) + a body of `count` items of the same
 * tag; FinishItem() advances the count and, when the buffer crosses the
 * threshold, flushes a complete independently-parseable record then starts a
 * fresh one for the same tag. The buffer auto-expands for large single items
 * (no truncation); memory is bounded at bufferSize_ + the largest item in the
 * current batch.
 *
 * Usage: BeginRecord(tag) -> WriteXxxItem() x N (each calls FinishItem) ->
 * EndRecord(); callers never invoke FinishItem() or WriteU* directly.
 *
 * Two concrete leaves, both referring to dump_format.h for the file-level
 * format spec: CommonWriter (file header, string pool, XRef, heap summary -
 * the serial sections) and StaticWriter (the static-side record items).
 * Multiple writers may share one OutputStream (its Write() is mutex-protected).
 */
class AbstractWriter {
public:
    /** Default buffer flush threshold (64 KB). When buffer_.size() >= this after FinishItem, auto-flush. */
    static constexpr size_t DEFAULT_BUFFER_SIZE = 64 * 1024;

    /**
     * Construct a writer with the given OutputStream and buffer size.
     * bufferSize is the flush threshold: when buffer_.size() >= bufferSize
     * after FinishItem(), the current record is flushed to the stream.
     */
    explicit AbstractWriter(OutputStream *stream, size_t bufferSize = DEFAULT_BUFFER_SIZE);
    virtual ~AbstractWriter();

    // Non-copyable, non-movable: owns the record buffer and references a
    // borrowed OutputStream. Declared to satisfy Rule-of-5 (user-declared dtor).
    AbstractWriter(const AbstractWriter &) = delete;
    AbstractWriter &operator=(const AbstractWriter &) = delete;
    AbstractWriter(AbstractWriter &&) = delete;
    AbstractWriter &operator=(AbstractWriter &&) = delete;

    /**
     * Begin a new record group for the given tag. If a previous record
     * group is pending (itemCount_ > 0), it is auto-flushed via
     * FlushRecord(). Captures current timestamp, resets itemCount_ and
     * clears buffer_.
     */
    void BeginRecord(uint8_t tag);

    /**
     * End the current record group. Flushes any remaining items as a
     * complete record. If itemCount_ == 0 (no items were written), does
     * nothing - no empty record is produced.
     */
    void EndRecord();

    /** Get the underlying OutputStream. */
    OutputStream *GetStream() const
    {
        return stream_;
    }

protected:
    /**
     * Mark one item as complete within the current record group.
     * Increments itemCount_, then checks if buffer_ exceeds bufferSize_.
     * If exceeded, the current record is flushed as a complete unit and
     * a new record group is started for the same tag (with a fresh timestamp).
     *
     * Only called by derived writers' WriteXXXItem methods - never by
     * external callers directly.
     */
    void FinishItem();

    /** Write a single byte into buffer_. */
    void WriteU8(uint8_t value);

    /** Write a 16-bit value into buffer_ in little-endian byte order. */
    void WriteU16(uint16_t value);

    /** Write a 32-bit value into buffer_ in little-endian byte order. */
    void WriteU32(uint32_t value);

    /** Write a 64-bit value into buffer_ in little-endian byte order. */
    void WriteU64(uint64_t value);

    /** Write a raw byte array into buffer_. */
    void WriteBytes(const uint8_t *data, size_t size);

private:
    OutputStream *stream_;

    /** Body data buffer. Auto-expands via vector for large items. */
    std::vector<uint8_t> buffer_;

    /** Flush threshold. When buffer_.size() >= bufferSize_ at FinishItem, flush. */
    size_t bufferSize_;

    /** Tag of the current record group. */
    uint8_t tag_ = 0;

    /** Timestamp of the current record group (steady_clock ms). */
    uint64_t recordTime_ = 0;

    /** Number of items accumulated in the current record group. */
    uint32_t itemCount_ = 0;

    /**
     * Flush the current record group as a complete, independently
     * parseable unit. Constructs a 17-byte header (tag, time, length,
     * count) and passes header + body to OutputStream::Write.
     * After flushing, resets itemCount_ and clears buffer_.
     */
    void FlushRecord();
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_ABSTRACT_WRITER_H
