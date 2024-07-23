/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/tests/native/native_test_helper.h"

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets::test {

class EtsNativePromiseTest : public EtsNapiTestBaseClass {};

TEST_F(EtsNativePromiseTest, PromiseResolve)
{
    ets_object promise;
    ets_deferred deferred;

    ASSERT_EQ(env_->PromiseCreate(&deferred, &promise), ETS_OK);

    ets_string value = env_->NewStringUTF("resolved");

    ASSERT_EQ(env_->DeferredResolve(deferred, value), ETS_OK);

    ets_boolean result;
    CallEtsFuntion(&result, "check_resolve", promise);
    ASSERT_EQ(result, true);
}

TEST_F(EtsNativePromiseTest, PromiseReject)
{
    ets_object promise;
    ets_deferred deferred;

    ASSERT_EQ(env_->PromiseCreate(&deferred, &promise), ETS_OK);

    ets_string exception = env_->NewStringUTF("rejected");

    ASSERT_EQ(env_->DeferredReject(deferred, exception), ETS_OK);

    ets_boolean result;
    CallEtsFuntion(&result, "check_reject", promise);
    ASSERT_EQ(result, true);
}

}  // namespace ark::ets::test
