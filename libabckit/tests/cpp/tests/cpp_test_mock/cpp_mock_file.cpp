/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#define ABCKIT_TEST_ENABLE_MOCK

#include "libabckit/include/cpp/abckit_cpp.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "libabckit/include/c/metadata_core.h"

#include <gtest/gtest.h>
#include <string_view>

namespace libabckit::test {

class LibAbcKitCppMockTest : public ::testing::Test {};

// Test: test-kind=internal, abc-kind=ArkTS1, category=internal
TEST_F(LibAbcKitCppMockTest, CppTestMockFile)
{
    {
        abckit::File file("abckit.abc");
        ASSERT(CheckMockedApi("openAbc"));
        file.WriteAbc("abckit.abc");
        ASSERT(CheckMockedApi("writeAbc"));
    }
    ASSERT(CheckMockedApi("closeFile"));
}

}  // namespace libabckit::test
