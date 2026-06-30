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

#ifndef PANDA_RUNTIME_TOOLING_BUFFER_SERIALIZER_H
#define PANDA_RUNTIME_TOOLING_BUFFER_SERIALIZER_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

namespace ark::tooling {

constexpr uint32_t BYTE_MASK = 0xFFU;
constexpr uint32_t BYTE_SHIFT = 8U;

/**
 * Utility class for serializing/deserializing frame metadata into/from a fixed-size buffer.
 * Used by GetDynamicFrameInfo implementations to safely transfer data from the SIGPROF
 * signal handler without heap allocation.
 *
 * Wire format:
 *   - strings: [length:uint32_le][data:bytes]  (length prefix enables memcmp ordering)
 *   - integers: [value:uint32/int32 in little-endian]
 */
class BufferSerializer {
public:
    struct PluginFrameData {
        std::string_view functionName;
        std::string_view moduleName;
        std::string_view url;
        int32_t lineNumber = 0;
        int32_t columnNumber = 0;
    };

    static size_t SerializePluginFrameData(const PluginFrameData &frameData, uint8_t *buffer, size_t bufferSize)
    {
        // Bound-safe pointer advancement helper
        // clang-format off
        auto at = [buffer](size_t off) -> uint8_t* {
            return reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(buffer) + off);
        };
        // clang-format on
        size_t offset = 0;
        size_t written = 0;
        written = WriteStringToBuffer(frameData.functionName, at(offset), bufferSize - offset);
        if (written == 0) {
            return 0;
        }

        offset += written;
        written = WriteStringToBuffer(frameData.moduleName, at(offset), bufferSize - offset);
        if (written == 0) {
            return 0;
        }

        offset += written;
        written = WriteStringToBuffer(frameData.url, at(offset), bufferSize - offset);
        if (written == 0) {
            return 0;
        }

        offset += written;
        written = WritePODToBuffer(frameData.lineNumber, at(offset), bufferSize - offset);
        if (written == 0) {
            return 0;
        }

        offset += written;
        written = WritePODToBuffer(frameData.columnNumber, at(offset), bufferSize - offset);
        if (written == 0) {
            return 0;
        }

        offset += written;
        return offset;
    }

    static PluginFrameData ReadPluginFrameData(const uint8_t *buffer, size_t bufferSize)
    {
        size_t offset = 0;
        PluginFrameData data;
        data.functionName = ReadStringFromBuffer(buffer, bufferSize, offset);
        data.moduleName = ReadStringFromBuffer(buffer, bufferSize, offset);
        data.url = ReadStringFromBuffer(buffer, bufferSize, offset);
        data.lineNumber = ReadPODFromBuffer<int32_t>(buffer, bufferSize, offset);
        data.columnNumber = ReadPODFromBuffer<int32_t>(buffer, bufferSize, offset);
        return data;
    }

    // Write a string with a little-endian uint32 length prefix. Returns bytes written (0 if buffer too small).
    static inline size_t WriteStringToBuffer(std::string_view str, uint8_t *buffer, size_t bufferSize)
    {
        constexpr size_t LEN_SIZE = sizeof(uint32_t);
        if (bufferSize < LEN_SIZE) {
            return 0;
        }
        auto strLen = std::min(static_cast<uint32_t>(str.size()), static_cast<uint32_t>(bufferSize - LEN_SIZE));
        if (WriteLEToBuffer(strLen, buffer, bufferSize) != 0 ||
            WriteStringData(str.data(), strLen, buffer, bufferSize) != 0) {
            return 0;
        }
        return LEN_SIZE + strLen;
    }

    // Write an integral value in little-endian format. Returns sizeof(T) (0 if buffer too small).
    template <typename T>
    static inline size_t WritePODToBuffer(T value, uint8_t *buffer, size_t bufferSize)
    {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        constexpr size_t POD_SIZE = sizeof(T);
        if (bufferSize < POD_SIZE) {
            return 0;
        }
        if (WriteLEToBuffer(value, buffer, bufferSize) != 0) {
            return 0;
        }
        return POD_SIZE;
    }

    // Read a length-prefixed string, advancing offset. Returns empty string on error and sets offset to bufferSize.
    static inline std::string_view ReadStringFromBuffer(const uint8_t *buffer, size_t bufferSize, size_t &offset)
    {
        uint32_t strLen = 0;
        if (DeserializeLE<uint32_t>(buffer, bufferSize, offset, strLen) != 0) {
            return {};
        }
        if (!CheckBounds(offset, bufferSize, strLen)) {
            return {};
        }
        return ReadStringData(buffer, strLen, offset);
    }

    // Read a little-endian integral value, advancing offset. Returns 0 on error and sets offset to bufferSize.
    template <typename T>
    static inline T ReadPODFromBuffer(const uint8_t *buffer, size_t bufferSize, size_t &offset)
    {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        using UT = std::make_unsigned_t<T>;
        UT value = 0;
        if (DeserializeLE<T>(buffer, bufferSize, offset, value) != 0) {
            return 0;
        }
        return static_cast<T>(value);
    }

private:
    // Serialize an integral value to little-endian bytes.
    template <typename T>
    static void SerializeLE(T value, std::array<uint8_t, sizeof(T)> &buf)
    {
        using UT = std::make_unsigned_t<T>;
        constexpr size_t N = sizeof(T);
        auto uv = static_cast<UT>(value);
        for (size_t i = 0; i < N; ++i) {
            buf[i] = static_cast<uint8_t>(uv & BYTE_MASK);
            if constexpr (N > 1) {
                uv >>= BYTE_SHIFT;
            }
        }
    }

    // Deserialize a little-endian integral value, advancing offset. Returns 0 on success, non-zero on failure.
    template <typename T>
    static int DeserializeLE(const uint8_t *buffer, size_t bufferSize, size_t &offset, std::make_unsigned_t<T> &value)
    {
        constexpr size_t N = sizeof(T);
        if (offset + N > bufferSize) {
            offset = bufferSize;
            return -1;
        }
        auto src = reinterpret_cast<const uint8_t *>(reinterpret_cast<uintptr_t>(buffer) + offset);
        std::array<uint8_t, N> bytes {};
        if (memcpy_s(bytes.data(), bytes.size(), src, N) != 0) {
            return -1;
        }
        using UT = std::make_unsigned_t<T>;
        for (size_t i = 0; i < N; ++i) {
            // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.Assign)
            value |= static_cast<UT>(bytes[i]) << (i * BYTE_SHIFT);
        }
        offset += N;
        return 0;
    }

    // Write string data after the length prefix. Returns 0 on success, non-zero on failure.
    static int WriteStringData(const char *data, uint32_t len, uint8_t *buffer, size_t bufferSize)
    {
        if (len == 0) {
            return 0;
        }
        constexpr size_t LEN_SIZE = sizeof(uint32_t);
        auto dest = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(buffer) + LEN_SIZE);
        return memcpy_s(dest, bufferSize - LEN_SIZE, data, len);
    }

    // Read string data after the length prefix.
    static std::string_view ReadStringData(const uint8_t *buffer, uint32_t strLen, size_t &offset)
    {
        auto data = reinterpret_cast<const char *>(reinterpret_cast<uintptr_t>(buffer) + offset);
        auto sv = std::string_view(data, strLen);
        offset += strLen;
        return sv;
    }

    // Check if there is enough space in the buffer; sets offset to bufferSize on overflow.
    static bool CheckBounds(size_t &offset, size_t bufferSize, size_t needed)
    {
        if (offset + needed > bufferSize) {
            offset = bufferSize;
            return false;
        }
        return true;
    }

    // Serialize an integral value in LE format and write to buffer. Returns 0 on success, non-zero on failure.
    template <typename T>
    static int WriteLEToBuffer(T value, uint8_t *buffer, size_t bufferSize)
    {
        constexpr size_t N = sizeof(T);
        std::array<uint8_t, N> tmp {};
        SerializeLE(value, tmp);
        return memcpy_s(buffer, bufferSize, tmp.data(), N);
    }
};

}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_BUFFER_SERIALIZER_H
