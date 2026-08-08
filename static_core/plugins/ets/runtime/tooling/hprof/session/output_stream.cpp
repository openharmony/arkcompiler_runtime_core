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

#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"
#include "libarkbase/utils/logger.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iterator>
#include <unistd.h>
#include "securec.h"

namespace ark::tooling::hprof {

OutputStream::OutputStream(const std::string &path, size_t bufferSize) : fd_(-1), ownsFd_(true), buffer_(bufferSize)
{
    constexpr auto OPEN_FLAGS = static_cast<int>(static_cast<uint32_t>(O_WRONLY) | static_cast<uint32_t>(O_CREAT) |
                                                 static_cast<uint32_t>(O_TRUNC) | static_cast<uint32_t>(O_CLOEXEC));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) -- POSIX open accepts a mode only when O_CREAT is set.
    fd_ = ::open(path.c_str(), OPEN_FLAGS, DEFAULT_FILE_MODE);
    if (fd_ < 0) {
        LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output file open failed: path=" << path << ", errno=" << errno;
    }
}

OutputStream::OutputStream(int fd, size_t bufferSize, bool ownsFd) : fd_(fd), ownsFd_(ownsFd), buffer_(bufferSize) {}

OutputStream::~OutputStream()
{
    os::memory::LockHolder lock(mutex_);
    if (fd_ >= 0) {
        (void)FlushLocked();
        if (ownsFd_) {
            ::close(fd_);
        }
    }
}

bool OutputStream::Write(const uint8_t *data, size_t size, const uint8_t *header, size_t headerSize)
{
    if (size == 0 && headerSize == 0) {
        return true;
    }
    if (headerSize > 0 && header == nullptr) {
        return false;
    }
    if (size > 0 && data == nullptr) {
        return false;
    }

    os::memory::LockHolder lock(mutex_);
    if (fd_ < 0 || writeFailed_) {
        return false;
    }
    return WriteToCacheLocked(header, headerSize) && WriteToCacheLocked(data, size);
}

void OutputStream::Close()
{
    os::memory::LockHolder lock(mutex_);
    (void)FlushLocked();
    if (ownsFd_ && fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void OutputStream::HandleWriteErrorLocked(size_t written, int error)
{
    LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output flush failed: fd=" << fd_ << ", errno=" << error
                        << ", written=" << written << ", expected=" << pos_;
    if (written > 0) {
        if (memmove_s(buffer_.data(), buffer_.size(), std::next(buffer_.data(), written), pos_ - written) != EOK) {
            LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output buffer recovery failed";
        }
    }
    pos_ -= written;
    writeFailed_ = true;
}

bool OutputStream::Flush()
{
    os::memory::LockHolder lock(mutex_);
    return FlushLocked();
}

bool OutputStream::FlushLocked()
{
    if (writeFailed_) {
        return false;
    }
    if (pos_ == 0) {
        return true;
    }

    size_t written = 0;
    while (written < pos_) {
        ssize_t n = ::write(fd_, std::next(buffer_.data(), written), pos_ - written);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            int error = n < 0 ? errno : EIO;
            HandleWriteErrorLocked(written, error);
            return false;
        }
        written += static_cast<size_t>(n);
    }
    pos_ = 0;
    return true;
}

bool OutputStream::WriteToCacheLocked(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return true;
    }

    size_t offset = 0;
    while (offset < size) {
        size_t remaining = buffer_.size() - pos_;
        size_t chunk = std::min(size - offset, remaining);
        if (memcpy_s(std::next(buffer_.data(), pos_), buffer_.size() - pos_, std::next(data, offset), chunk) != EOK) {
            LOG(ERROR, RUNTIME) << "[HybDump][Sta] Output buffer write failed";
            writeFailed_ = true;
            return false;
        }
        pos_ += chunk;
        offset += chunk;
        if (pos_ == buffer_.size() && !FlushLocked()) {
            return false;
        }
    }
    return true;
}

}  // namespace ark::tooling::hprof
