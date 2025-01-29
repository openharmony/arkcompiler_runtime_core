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

class CreateGlobalReferenceTest : public AniTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(CreateGlobalReferenceTest, create_global_ref)
{
    ani_string string = nullptr;

    ani_size length = 4;
    // create local reference
    ASSERT_EQ(env_->String_NewUTF8("test", length, &string), ANI_OK);

    ani_gref result;
    ani_ref ref = string;

    // create global reference
    ASSERT_EQ(env_->GlobalReference_Create(ref, &result), ANI_OK);

    // create global successful
    ASSERT_NE(result, nullptr);
    // issue 22089
    // check invalid args
    ASSERT_EQ(env_->GlobalReference_Create(nullptr, &result), ANI_INVALID_ARGS);

    // check invalid args
    ASSERT_EQ(env_->GlobalReference_Create(ref, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
