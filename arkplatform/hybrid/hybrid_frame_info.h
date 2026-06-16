/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PROFILER_HYBRID_FRAME_INFO_H
#define PROFILER_HYBRID_FRAME_INFO_H

#include <array>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace arkplatform {

constexpr size_t HYBRID_FRAME_FUNCTION_NAME_SIZE = 100;
constexpr size_t HYBRID_FRAME_URL_SIZE = 512;
constexpr size_t HYBRID_FRAME_MODULE_NAME_SIZE = 512;

struct HybridFrameInfo {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::array<char, HYBRID_FRAME_FUNCTION_NAME_SIZE> functionName {};
    std::array<char, HYBRID_FRAME_URL_SIZE> url {};
    std::array<char, HYBRID_FRAME_MODULE_NAME_SIZE> moduleName {};
    int32_t scriptId = 0;
    int32_t lineNumber = -1;
    int32_t columnNumber = -1;
    uint32_t bcOffset = 0;
    uintptr_t fileId = 0;
    bool isStaticFrame = true;  // true for ETS (static), false for JS (dynamic)

    void *nativePtr = nullptr;  // methodPtr (for static) or dynamicFramePtr (for dynamic)
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    void SetFunctionName(std::string_view name)
    {
        CopyToBuffer(functionName, name);
    }

    void SetUrl(std::string_view value)
    {
        CopyToBuffer(url, value);
    }

    void SetModuleName(std::string_view name)
    {
        CopyToBuffer(moduleName, name);
    }

    std::string_view GetFunctionName() const
    {
        return BufferToStringView(functionName);
    }

    std::string_view GetUrl() const
    {
        return BufferToStringView(url);
    }

    std::string_view GetModuleName() const
    {
        return BufferToStringView(moduleName);
    }

private:
    template <size_t N>
    static void CopyToBuffer(std::array<char, N> &dst, std::string_view src)
    {
        static_assert(N > 0);
        const size_t len = (src.size() < (N - 1)) ? src.size() : (N - 1);
        for (size_t i = 0; i < len; ++i) {
            dst[i] = src[i];
        }
        dst[len] = '\0';
    }

    template <size_t N>
    static std::string_view BufferToStringView(const std::array<char, N> &buffer)
    {
        size_t len = 0;
        while (len < N && buffer[len] != '\0') {
            ++len;
        }
        return std::string_view(buffer.data(), len);
    }
};

}  // namespace arkplatform

#endif  // PROFILER_HYBRID_FRAME_INFO_H
