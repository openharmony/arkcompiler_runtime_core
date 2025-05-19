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

#include "plugins/ets/tests/mock/mock_test_helper.h"

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "plugins/ets/runtime/types/ets_method.h"

// NOLINTBEGIN(cppcoreguidelines-pro-runtime-exception)

namespace ark::ets::test {

class EtsNativeInterfaceException : public MockEtsNapiTestBaseClass {};

TEST_F(EtsNativeInterfaceException, TestThrowRuntimeException)
{
    ets_class aCls = env_->FindClass("std/core/RuntimeException");
    ASSERT_NE(aCls, nullptr);
    env_->ThrowErrorNew(aCls, "Test runtime exception");
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-runtime-exception)