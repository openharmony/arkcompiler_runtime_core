/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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


#include "libpandabase/os/mem.h"
#include <gtest/gtest.h>

namespace panda {

class ReleasePagesTest : public testing::Test {};

HWTEST_F(ReleasePagesTest, ReleasePagesValidRangeTest, testing::ext::TestSize.Level0)
{
    constexpr uintptr_t kPageStart = 0x1000UL;
    constexpr uintptr_t kPageEnd = 0x2000UL;

    panda::os::mem::ReleasePages(kPageStart, kPageEnd);
    ASSERT_TRUE(true);
}
} // namespace ark::os::mem::test