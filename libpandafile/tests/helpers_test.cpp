/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "helpers.h"
#include "file_items.h"
#include "gtest/gtest.h"

namespace panda::panda_file::test {

HWTEST(HelpersTest, ReadULeb128Test, testing::ext::TestSize.Level0)
{
    // Test case 1: Normal single-byte value (value=1)
    {
        const uint8_t data[] = {0x01};
        Span<const uint8_t> sp(data);
        EXPECT_EQ(helpers::ReadULeb128(&sp), 1U);
        EXPECT_EQ(sp.Size(), 0U);
    }

    // Test case 2: Multi-byte value (value=624485)
    {
        const uint8_t data[] = {0xE5, 0x8E, 0x26};
        Span<const uint8_t> sp(data);
        EXPECT_EQ(helpers::ReadULeb128(&sp), 624485U);
        EXPECT_EQ(sp.Size(), 0U);
    }

    // Test case 3: Invalid LEB128 encoding (5th byte uses more than 4 bits)
    {
        const uint8_t data[] = {0x80, 0x80, 0x80, 0x80, 0x10};
        Span<const uint8_t> sp(data);
#ifdef HOST_UT
        EXPECT_DEATH(helpers::ReadULeb128(&sp), "F/pandafile: Invalid span offset");
#endif
    }

    // Test case 4: Insufficient span size
    {
        const uint8_t data[] = {0x80, 0x80};
        Span<const uint8_t> sp(data);
#ifdef HOST_UT
        EXPECT_DEATH(helpers::ReadULeb128(&sp), "F/pandafile: Invalid span offset");
#endif
    }

    // Test case 5: Maximum legal value (0xFFFFFFF)
    {
        const uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x0F};
        Span<const uint8_t> sp(data);
        EXPECT_EQ(helpers::ReadULeb128(&sp), 0xFFFFFFFF);
        EXPECT_EQ(sp.Size(), 0U);
    }
}
}  // namespace panda::panda_file::test
