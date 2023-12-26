/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

namespace panda::ets::test {

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

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

struct Member {
    const char *name;
    size_t offset;
};

class EtsPromiseMembers {
public:
    static std::vector<Member> GetMembers()
    {
        return std::vector<Member> {{"value", MEMBER_OFFSET(EtsPromise, value_)},
                                    {"thenQueue", MEMBER_OFFSET(EtsPromise, thenQueue_)},
                                    {"catchQueue", MEMBER_OFFSET(EtsPromise, catchQueue_)},
                                    {"linkedPromise", MEMBER_OFFSET(EtsPromise, linkedPromise_)},
                                    {"eventPtr", MEMBER_OFFSET(EtsPromise, event_)},
                                    {"thenQueueSize", MEMBER_OFFSET(EtsPromise, thenQueueSize_)},
                                    {"catchQueueSize", MEMBER_OFFSET(EtsPromise, catchQueueSize_)},
                                    {"state", MEMBER_OFFSET(EtsPromise, state_)}};
    }
};

TEST_F(EtsPromiseTest, MemoryLayout)
{
    EtsClass *promiseClass = vm_->GetClassLinker()->GetPromiseClass();
    ASSERT_NE(nullptr, promiseClass);
    std::vector<Member> members = EtsPromiseMembers::GetMembers();
    ASSERT_EQ(members.size(), promiseClass->GetInstanceFieldsNumber());

    // Check both EtsPromise and panda::Class<Promise> has the same number of fields
    // and at the same offsets
    for (const Member &memb : members) {
        EtsField *field = promiseClass->GetFieldIDByName(memb.name);
        ASSERT_NE(nullptr, field);
        ASSERT_EQ(memb.offset, field->GetOffset()) << "Offsets of the field '" << memb.name << "' are different";
    }
}
}  // namespace panda::ets::test
