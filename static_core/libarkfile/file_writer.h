/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifndef LIBPANDAFILE_FILE_WRITER_H_
#define LIBPANDAFILE_FILE_WRITER_H_

#include "libarkbase/os/file.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/type_helpers.h"
#include "libarkbase/utils/leb128.h"
#include "securec.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>

#include <limits>
#include <type_traits>
#include <vector>

namespace ark::panda_file {

class Writer {
public:
    virtual bool WriteByte(uint8_t byte) = 0;

    virtual bool WriteBytes(const uint8_t *bytes, size_t size)
    {
        for (size_t i = 0; i < size; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (!WriteByte(bytes[i])) {
                return false;
            }
        }
        return true;
    }

    virtual bool WriteBytes(const std::vector<uint8_t> &bytes)
    {
        return WriteBytes(bytes.data(), bytes.size());
    }

    virtual size_t GetOffset() const = 0;

    virtual void CountChecksum(bool /* counting */) {}

    virtual bool WriteChecksum(size_t /* offset */)
    {
        return false;
    }

    bool Align(size_t alignment)
    {
        size_t offset = GetOffset();
        size_t n = RoundUp(offset, alignment) - offset;
        static constexpr std::array<uint8_t, 64U> ZERO_PADDING {};
        while (n > 0) {
            auto chunkSize = std::min(n, ZERO_PADDING.size());
            if (!WriteBytes(ZERO_PADDING.data(), chunkSize)) {
                return false;
            }
            n -= chunkSize;
        }
        return true;
    }

    template <class T>
    bool Write(T data)
    {
        static constexpr size_t BYTE_MASK = 0xff;
        [[maybe_unused]] static constexpr size_t BYTE_WIDTH = std::numeric_limits<uint8_t>::digits;

        for (size_t i = 0; i < sizeof(T); i++) {
            if (!WriteByte(data & BYTE_MASK)) {
                return false;
            }

            if constexpr (sizeof(T) > sizeof(uint8_t)) {
                data >>= BYTE_WIDTH;
            }
        }
        return true;
    }

    template <class T>
    bool WriteUleb128(T v)
    {
        static_assert(std::is_integral_v<T>, "T must be integral");
        if constexpr (std::is_signed_v<T>) {
            ASSERT(v >= 0);
        }
        using UnsignedT = std::make_unsigned_t<T>;
        constexpr size_t MAX_BYTES =
            (std::numeric_limits<UnsignedT>::digits + leb128::PAYLOAD_WIDTH - 1) / leb128::PAYLOAD_WIDTH;
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        uint8_t out[MAX_BYTES];
        size_t n = leb128::EncodeUnsigned(static_cast<UnsignedT>(v), out);
        return WriteBytes(out, n);
    }

    template <class T>
    bool WriteSleb128(T v)
    {
        static_assert(std::is_signed_v<T>, "T must be signed");
        using UnsignedT = std::make_unsigned_t<T>;
        constexpr size_t MAX_BYTES =
            (std::numeric_limits<UnsignedT>::digits + leb128::PAYLOAD_WIDTH - 1) / leb128::PAYLOAD_WIDTH + 1;
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        uint8_t out[MAX_BYTES];
        size_t n = leb128::EncodeSigned(v, out);
        return WriteBytes(out, n);
    }

    virtual void ReserveBufferCapacity([[maybe_unused]] size_t size) {}

    virtual bool FinishWrite()
    {
        return true;
    }

    // default methods
    Writer() = default;
    virtual ~Writer() = default;

    NO_COPY_SEMANTIC(Writer);
    NO_MOVE_SEMANTIC(Writer);
};

class MemoryWriter : public Writer {
public:
    PANDA_PUBLIC_API MemoryWriter();

    using Writer::WriteBytes;

    void CountChecksum(bool counting) override
    {
        countChecksum_ = counting;
    }

    bool WriteChecksum(size_t offset) override
    {
        auto span = Span(data_.data(), data_.size());
        auto sub = span.SubSpan(offset);
        return (memcpy_s(sub.data(), sizeof(checksum_), &checksum_, sizeof(checksum_)) == 0);
    }

    bool WriteByte(uint8_t byte) override;

    bool WriteBytes(const std::vector<uint8_t> &bytes) override;

    bool WriteBytes(const uint8_t *bytes, size_t size) override;

    const std::vector<uint8_t> &GetData()
    {
        return data_;
    }

    size_t GetOffset() const override
    {
        return data_.size();
    }

private:
    std::vector<uint8_t> data_;
    uint32_t checksum_;
    bool countChecksum_ {false};
};

class MemoryBufferWriter : public Writer {
public:
    PANDA_PUBLIC_API explicit MemoryBufferWriter(uint8_t *buffer, size_t size);

    ~MemoryBufferWriter() override = default;

    using Writer::WriteBytes;

    NO_COPY_SEMANTIC(MemoryBufferWriter);
    NO_MOVE_SEMANTIC(MemoryBufferWriter);

    void CountChecksum(bool counting) override
    {
        countChecksum_ = counting;
    }

    bool WriteChecksum(size_t offset) override
    {
        auto sub = sp_.SubSpan(offset);
        return (memcpy_s(sub.data(), sizeof(checksum_), &checksum_, sizeof(checksum_)) == 0);
    }

    bool WriteByte(uint8_t byte) override;

    bool WriteBytes(const std::vector<uint8_t> &bytes) override;

    bool WriteBytes(const uint8_t *bytes, size_t size) override;

    size_t GetOffset() const override
    {
        return offset_;
    }

private:
    Span<uint8_t> sp_;
    size_t offset_ {0};
    uint32_t checksum_;
    bool countChecksum_ {false};
};

class FileWriter : public Writer {
public:
    PANDA_PUBLIC_API explicit FileWriter(const std::string &fileName);

    PANDA_PUBLIC_API ~FileWriter() override;

    using Writer::WriteBytes;

    NO_COPY_SEMANTIC(FileWriter);
    NO_MOVE_SEMANTIC(FileWriter);

    void CountChecksum(bool counting) override;

    bool WriteChecksum(size_t offset) override;

    bool WriteByte(uint8_t data) override;

    bool WriteBytes(const std::vector<uint8_t> &bytes) override;

    bool WriteBytes(const uint8_t *bytes, size_t size) override;

    size_t GetOffset() const override
    {
        return offset_;
    }

    uint32_t GetChecksum() const
    {
        return checksum_;
    }

    explicit operator bool() const
    {
        return file_ != nullptr;
    }

    void ReserveBufferCapacity(size_t size) override
    {
        buffer_.reserve(seekable_ ? std::min(size, BUFFER_CAPACITY) : size);
    }

    bool FinishWrite() override;

private:
    bool FlushBuffer();

    static constexpr size_t BUFFER_CAPACITY = 1U << 20U;
    FILE *file_;
    uint32_t checksum_;
    bool countChecksum_ {false};
    bool seekable_ {false};
    size_t offset_ {0};
    std::vector<uint8_t> buffer_;
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_FILE_WRITER_H_
