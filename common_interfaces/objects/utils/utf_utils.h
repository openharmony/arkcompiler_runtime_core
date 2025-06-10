/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef COMMON_INTERFACES_OBJECTS_UTILS_UTF_H
#define COMMON_INTERFACES_OBJECTS_UTILS_UTF_H

#include <cstddef>
#include <cstdint>

namespace common {
class BaseString;

class UtfUtils {
private:
    static constexpr uint8_t UTF8_1B_MAX = 0x7f;
    static size_t DebuggerConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len,
                                                   size_t utf8Len, size_t start, bool modify = true,
                                                   bool isWriteBuffer = false);

    static size_t Utf8ToUtf16Size(const uint8_t *utf8, size_t utf8Len);

    static size_t Utf16ToUtf8Size(const uint16_t *utf16, uint32_t length, bool modify = true,
                                  bool isGetBufferSize = false, bool cesu8 = false);

    static size_t ConvertRegionUtf8ToUtf16(const uint8_t *utf8In, uint16_t *utf16Out, size_t utf8Len, size_t utf16Len);

    static size_t ConvertRegionUtf16ToLatin1(const uint16_t *utf16In, uint8_t *latin1Out, size_t utf16Len,
                                             size_t latin1Len);

    static size_t ConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len,
                                           size_t utf8Len, size_t start, bool modify = true, bool isWriteBuffer = false,
                                           bool cesu = false);

    friend BaseString;
};
} // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_UTILS_UTF_H