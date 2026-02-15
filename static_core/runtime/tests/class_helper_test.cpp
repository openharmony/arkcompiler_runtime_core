/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <array>
#include <cerrno>
#include <cstdarg>
#include <gtest/gtest.h>
#include <sstream>
#include <string_view>
#include "libarkbase/utils/utf.h"

#include "runtime/include/class_helper.h"

namespace ark::test {

TEST(ClassHelpers, IsPrimitive)
{
    const std::array<std::pair<const char *, std::string_view>, 15> PRIMITIVE_RUNTIME_NAMES = {
        std::pair {"V", "void"}, std::pair {"Z", "u1"},  std::pair {"B", "i8"},  std::pair {"H", "u8"},
        std::pair {"S", "i16"},  std::pair {"C", "u16"}, std::pair {"I", "i32"}, std::pair {"U", "u32"},
        std::pair {"J", "i64"},  std::pair {"Q", "u64"}, std::pair {"F", "f32"}, std::pair {"D", "f64"},
        std::pair {"A", "any"},  std::pair {"Y", "Y"},   std::pair {"N", "N"}};

    for (const auto &p : PRIMITIVE_RUNTIME_NAMES) {
        ASSERT_TRUE(ClassHelper::IsPrimitive(utf::CStringAsMutf8(p.first)));
    }
}

}  // namespace ark::test
