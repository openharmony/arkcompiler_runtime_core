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

#include "ani_gtest.h"
#include "plugins/ets/runtime/libani_helpers/gc_guard.h"
#include "runtime/assert_gc_scope.h"

namespace ark::ets::ani_helpers::testing {

class GcGuardTest : public ark::ets::ani::testing::AniTest {};

TEST_F(GcGuardTest, ScopeDisallowsGarbageCollection)
{
    ASSERT_TRUE(::ark::AssertGCScopeT::IsAllowed());
    {
        arkts::GCGuard gcGuard;
        EXPECT_FALSE(::ark::AssertGCScopeT::IsAllowed());
    }
    EXPECT_TRUE(::ark::AssertGCScopeT::IsAllowed());
}

TEST_F(GcGuardTest, NestedScopesRemainGuarded)
{
    ASSERT_TRUE(::ark::AssertGCScopeT::IsAllowed());
    {
        arkts::GCGuard outer;
        EXPECT_FALSE(::ark::AssertGCScopeT::IsAllowed());
        {
            arkts::GCGuard inner;
            EXPECT_FALSE(::ark::AssertGCScopeT::IsAllowed());
        }
        EXPECT_FALSE(::ark::AssertGCScopeT::IsAllowed());
    }
    EXPECT_TRUE(::ark::AssertGCScopeT::IsAllowed());
}

}  // namespace ark::ets::ani_helpers::testing
