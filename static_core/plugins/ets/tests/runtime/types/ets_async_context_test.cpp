/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ets_coroutine.h"

#include "ets_vm.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_async_context.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsAsyncContextTest : public testing::Test {
public:
    EtsAsyncContextTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        Runtime::Create(options);
        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        vm_ = coroutine->GetPandaVM();
    }

    ~EtsAsyncContextTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsAsyncContextTest);
    NO_MOVE_SEMANTIC(EtsAsyncContextTest);

    static std::vector<MirrorFieldInfo> GetAsyncContextMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsAsyncContext, awaitee_, "awaitee"),
            MIRROR_FIELD_INFO(EtsAsyncContext, returnValue_, "returnValue"),
            MIRROR_FIELD_INFO(EtsAsyncContext, refValues_, "refValues"),
            MIRROR_FIELD_INFO(EtsAsyncContext, primValues_, "primValues"),
            MIRROR_FIELD_INFO(EtsAsyncContext, frameOffsets_, "frameOffsets"),
            MIRROR_FIELD_INFO(EtsAsyncContext, pc_, "pc"),
            MIRROR_FIELD_INFO(EtsAsyncContext, refCount_, "refCount"),
            MIRROR_FIELD_INFO(EtsAsyncContext, primCount_, "primCount"),
            MIRROR_FIELD_INFO(EtsAsyncContext, awaitId_, "awaitId"),
            MIRROR_FIELD_INFO(EtsAsyncContext, compiledCode_, "compiledCode"),
        };
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

// Check both EtsAsyncContext and ark::Class<AsyncContext> has the same number of fields
// and at the same offsets
TEST_F(EtsAsyncContextTest, AsyncContextMemoryLayout)
{
    auto *asyncContextClass = PlatformTypes(vm_)->arkruntimeAsyncContext;
    MirrorFieldInfo::CompareMemberOffsets(asyncContextClass, GetAsyncContextMembers());
}

}  // namespace ark::ets::test
