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

#include <gtest/gtest.h>

#include "ark_guard/ark_guard.h"

using namespace testing::ext;

/**
 * @tc.name: ark_guard_test_001
 * @tc.desc: test ark_guard run
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ArkGuardTest, ark_guard_test_001, TestSize.Level0)
{
    int argc = 1;
    char *argv[1];
    argv[0] = const_cast<char *>("xxx");

    ark::guard::ArkGuard guard;
    guard.Run(argc, const_cast<const char **>(argv));
}
