/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class FindNamespaceTest : public AniTest {};

TEST_F(FindNamespaceTest, has_namespace)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lgeometry;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);
}

TEST_F(FindNamespaceTest, namespace_not_found)
{
    ani_namespace ns;
    ASSERT_EQ(env_->FindNamespace("bla-bla-bla", &ns), ANI_NOT_FOUND);
}

TEST_F(FindNamespaceTest, invalid_argument_1)
{
    ani_namespace ns;
    ASSERT_EQ(env_->FindNamespace(nullptr, &ns), ANI_INVALID_ARGS);
}

TEST_F(FindNamespaceTest, invalid_argument_2)
{
    ASSERT_EQ(env_->FindNamespace("Lgeometry;", nullptr), ANI_INVALID_ARGS);
}

// Enable when #22400 is resolved.
TEST_F(FindNamespaceTest, DISABLED_namespace_is_not_class)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lgeometry;", &cls), ANI_NOT_FOUND);
}

}  // namespace ark::ets::ani::testing
