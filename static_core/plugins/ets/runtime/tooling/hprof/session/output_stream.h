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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_OUTPUT_STREAM_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_OUTPUT_STREAM_H

#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/tooling/hprof/session/dump_format.h"
#include "profiler/heap_dump.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ark::tooling::hprof {

/** @brief Default file creation mode: owner rw only; heap dumps carry sensitive data. */
static constexpr int DEFAULT_FILE_MODE = 0600;

/**
 * @brief OutputStream is a concrete, buffered, thread-safe output stream backed by a
 * POSIX file descriptor.
 *
 * It owns a cache buffer (default 64KB). Data written via Write() first enters
 * the cache; when the cache is full, it is automatically flushed to the fd.
 * Write() is mutex-protected so multiple writers can safely write to the same
 * stream concurrently.
 *
 * Write() accepts an optional record header that is written before the body
 * data. The header+body form one logical unit - the header bytes always
 * appear immediately before the body bytes in the output, even if the cache
 * flushes between header and body.
 *
 * Two construction modes:
 *   1. OutputStream(path) - opens the file by path, owns the fd.
 *   2. OutputStream(fd, bufferSize) - uses a pre-opened fd, does NOT own it.
 */
class OutputStream final : public common::dump::DumpOutput {
public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 64 * 1024;  // 64KB

    explicit OutputStream(const std::string &path, size_t bufferSize = DEFAULT_BUFFER_SIZE);
    explicit OutputStream(int fd, size_t bufferSize = DEFAULT_BUFFER_SIZE, bool ownsFd = false);
    ~OutputStream() override;
    OutputStream(const OutputStream &) = delete;
    OutputStream &operator=(const OutputStream &) = delete;
    OutputStream(OutputStream &&) = delete;
    OutputStream &operator=(OutputStream &&) = delete;

    /**
     * Thread-safe write. If a record header is provided, it is written
     * before the body data, within the same mutex lock.
     *
     * @param data       Pointer to the body data to write.
     * @param size       Number of body bytes to write.
     * @param header     Optional record header to prepend (e.g. 17-byte
     *                   [tag|time|length|count] record header). If nullptr, only body
     *                   data is written (backward compatible).
     * @param headerSize Number of header bytes. 0 if no header.
     * @return true if all input was buffered or flushed successfully.
     *         false for invalid arguments, an invalid fd, or an I/O failure.
     */
    bool Write(const uint8_t *data, size_t size, const uint8_t *header = nullptr, size_t headerSize = 0);

    void Close();

    bool Good() const
    {
        return fd_ >= 0;
    }

    /**
     * Flush remaining buffer data to the file descriptor.
     *
     * @return true if all buffered data was written successfully; false if
     *         this or an earlier write operation failed.
     */
    bool Flush() override;

private:
    /**
     * Internal write into cache buffer. Caller must hold mutex_.
     * Loops: fill remaining cache space, flush when full, continue.
     */
    bool WriteToCacheLocked(const uint8_t *data, size_t size);

    /** @brief Flush while mutex_ is held. */
    bool FlushLocked();

    /** Record a permanent write error and preserve unwritten bytes. Caller must hold mutex_. */
    void HandleWriteErrorLocked(size_t written, int error);

    int fd_;
    bool ownsFd_;
    os::memory::Mutex mutex_;
    std::vector<uint8_t> buffer_;
    size_t pos_ = 0;
    bool writeFailed_ = false;
};

}  // namespace ark::tooling::hprof

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HPROF_SESSION_OUTPUT_STREAM_H
