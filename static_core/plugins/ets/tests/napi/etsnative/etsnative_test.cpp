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

#include "plugins/ets/runtime/napi/ets_napi.h"

#include <cstdlib>
#include <gtest/gtest.h>
#include <vector>

namespace ark::ets::test {

class EtsNativeTest : public testing::Test {};

TEST_F(EtsNativeTest, EtsNativeTest)
{
    EtsEnv *env {nullptr};
    EtsVM *vm {nullptr};

    std::vector<EtsVMOption> optionsVector;
    optionsVector = {{EtsOptionType::ETS_BOOT_FILE, std::getenv("ARK_ETS_STDLIB_PATH")}};

    EtsVMInitArgs vmArgs;
    vmArgs.version = ETS_NAPI_VERSION_1_0;
    vmArgs.options = optionsVector.data();
    vmArgs.nOptions = static_cast<ets_int>(optionsVector.size());

    ASSERT_TRUE(ETS_CreateVM(&vm, &env, &vmArgs) == ETS_OK) << "Cannot create ETS VM";

    EtsVM *etsVms;
    ets_size nVms;
    ASSERT_TRUE(ETS_GetCreatedVMs(&etsVms, 1, &nVms) == ETS_OK) << "Cannot get created ETS VM";

    ASSERT_EQ(nVms, 1);
    ASSERT_EQ(etsVms, vm);
    ASSERT_TRUE(vm->DestroyEtsVM() == ETS_OK) << "Cannot destroy ETS VM";
}
}  // namespace ark::ets::test
