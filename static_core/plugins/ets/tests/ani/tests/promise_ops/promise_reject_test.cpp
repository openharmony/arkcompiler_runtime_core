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

#include "ani/ani.h"
#include "ani_gtest.h"

namespace ark::ets::ani::testing {

class PromiseRejectTest : public AniTest {};

TEST_F(PromiseRejectTest, ResolvePromise)
{
    ani_object promise {};
    ani_resolver resolver {};

    ASSERT_EQ(env_->Promise_New(&resolver, &promise), ANI_OK);

    std::string rejected = "rejected";
    ani_string rejection = nullptr;
    ASSERT_EQ(env_->String_NewUTF8(rejected.c_str(), rejected.size(), &rejection), ANI_OK);

    ASSERT_EQ(env_->PromiseResolver_Reject(resolver, reinterpret_cast<ani_error>(rejection)), ANI_OK);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("promise_reject_test", "checkReject", promise, rejection), ANI_TRUE);
}

TEST_F(PromiseRejectTest, InvalidArgument1)
{
    ani_resolver resolver {};
    ani_error error {};
    ASSERT_EQ(env_->c_api->PromiseResolver_Reject(nullptr, resolver, error), ANI_INVALID_ARGS);
}

TEST_F(PromiseRejectTest, InvalidArgument2)
{
    ani_error error {};
    ASSERT_EQ(env_->PromiseResolver_Reject(nullptr, error), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing
