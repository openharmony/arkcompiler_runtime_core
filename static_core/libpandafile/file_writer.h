/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "os/file.h"
#include "utils/span.h"
#include "utils/type_helpers.h"
#include "utils/leb128.h"
#include "securec.h"

#include <cstdint>
#include <cerrno>

#include <limits>
#include <vector>

namespace ark::panda_file {

class Writer {
public:
    virtual bool WriteByte(uint8_t byte) = 0;

    virtual bool WriteBytes(const std::vector<uint8_t> &bytes) = 0;

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
        while (n-- > 0) {
            if (!WriteByte(0)) {
                return false;
            }
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
        size_t n = leb128::UnsignedEncodingSize(v);
        std::vector<uint8_t> out(n);
        leb128::EncodeUnsigned(v, out.data());
        return WriteBytes(out);
    }

    template <class T>
    bool WriteSleb128(T v)
    {
        size_t n = leb128::SignedEncodingSize(v);
        std::vector<uint8_t> out(n);
        leb128::EncodeSigned(v, out.data());
        return WriteBytes(out);
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

    bool WriteByte(uint8_t byte) override
    {
        return WriteBytes({byte});
    }

    bool WriteBytes(const std::vector<uint8_t> &bytes) override;

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

    NO_COPY_SEMANTIC(MemoryBufferWriter);
    NO_MOVE_SEMANTIC(MemoryBufferWriter);

    void CountChecksum(bool counting) override
    {
        countChecksum_ = counting;
    }

    bool WriteChecksum(size_t offset) override
    {
        auto sub = sp_.SubSpan(offset);
        return (memcpy_s(sub.data(), sizeof(checksum_), &checksum_, sizeof(checksum_)) != 0);
    }

    bool WriteByte(uint8_t byte) override
    {
        return WriteBytes({byte});
    }

    bool WriteBytes(const std::vector<uint8_t> &bytes) override;

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

    NO_COPY_SEMANTIC(FileWriter);
    NO_MOVE_SEMANTIC(FileWriter);

    void CountChecksum(bool counting) override
    {
        countChecksum_ = counting;
    }

    bool WriteChecksum(size_t offset) override
    {
        if (fseek(file_, static_cast<int64_t>(offset), SEEK_SET) != 0) {
            LOG(FATAL, RUNTIME) << "Unable to write checksum by offset: " << static_cast<int64_t>(offset);
            UNREACHABLE();
        }
        auto res = Write<uint32_t>(checksum_);
        if (fseek(file_, static_cast<int64_t>(offset), SEEK_END) != 0) {
            LOG(FATAL, RUNTIME) << "Unable to write checksum by offset: " << static_cast<int64_t>(offset);
            UNREACHABLE();
        }
        return res;
    }

    bool WriteByte(uint8_t data) override;

    bool WriteBytes(const std::vector<uint8_t> &bytes) override;

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

private:
    FILE *file_;
    size_t offset_ {0};
    uint32_t checksum_;
    bool countChecksum_ {false};
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_FILE_WRITER_H_
