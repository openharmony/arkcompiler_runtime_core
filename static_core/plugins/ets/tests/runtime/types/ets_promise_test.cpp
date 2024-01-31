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

#include <gtest/gtest.h>

#include "ets_coroutine.h"

#include "types/ets_class.h"
#include "types/ets_promise.h"
#include "tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsPromiseTest : public testing::Test {
public:
    EtsPromiseTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(false);
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

    ~EtsPromiseTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsPromiseTest);
    NO_MOVE_SEMANTIC(EtsPromiseTest);

    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsPromise, value_, "value"),
                                             MIRROR_FIELD_INFO(EtsPromise, thenQueue_, "thenQueue"),
                                             MIRROR_FIELD_INFO(EtsPromise, catchQueue_, "catchQueue"),
                                             MIRROR_FIELD_INFO(EtsPromise, linkedPromise_, "linkedPromise"),
                                             MIRROR_FIELD_INFO(EtsPromise, event_, "eventPtr"),
                                             MIRROR_FIELD_INFO(EtsPromise, thenQueueSize_, "thenQueueSize"),
                                             MIRROR_FIELD_INFO(EtsPromise, catchQueueSize_, "catchQueueSize"),
                                             MIRROR_FIELD_INFO(EtsPromise, state_, "state")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(EtsPromiseTest, MemoryLayout)
{
    EtsClass *promiseClass = vm_->GetClassLinker()->GetPromiseClass();
    ASSERT_NE(nullptr, promiseClass);
    std::vector<MirrorFieldInfo> members = GetMembers();
    ASSERT_EQ(members.size(), promiseClass->GetInstanceFieldsNumber());

    // Check both EtsPromise and ark::Class<Promise> has the same number of fields
    // and at the same offsets
    for (const MirrorFieldInfo &memb : members) {
        EtsField *field = promiseClass->GetFieldIDByName(memb.Name());
        ASSERT_NE(nullptr, field);
        ASSERT_EQ(memb.Offset(), field->GetOffset()) << "Offsets of the field '" << memb.Name() << "' are different";
    }
}
}  // namespace ark::ets::test
