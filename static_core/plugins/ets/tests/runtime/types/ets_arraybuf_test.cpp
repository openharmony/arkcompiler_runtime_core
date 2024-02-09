/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "types/ets_arraybuffer.h"

namespace ark::ets::test {

class EtsArrayBufferTest : public testing::Test {
public:
    EtsArrayBufferTest()
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

    ~EtsArrayBufferTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsArrayBufferTest);
    NO_MOVE_SEMANTIC(EtsArrayBufferTest);

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

struct Member {
    const char *name;
    size_t offset;
};

class EtsArrayBufferMembers {
public:
    static std::vector<Member> GetMembers()
    {
        return std::vector<Member> {{"byteLength", MEMBER_OFFSET(EtsArrayBuffer, byteLength_)},
                                    {"data", MEMBER_OFFSET(EtsArrayBuffer, data_)}};
    }
};

TEST_F(EtsArrayBufferTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayBufferClass();
    ASSERT_NE(nullptr, klass);
    std::vector<Member> members = EtsArrayBufferMembers::GetMembers();
    ASSERT_EQ(members.size(), klass->GetInstanceFieldsNumber());

    // Check both EtsPromise and ark::Class<Promise> has the same number of fields
    // and at the same offsets
    for (const Member &memb : members) {
        EtsField *field = klass->GetFieldIDByName(memb.name);
        ASSERT_NE(nullptr, field);
        ASSERT_EQ(memb.offset, field->GetOffset()) << "Offsets of the field '" << memb.name << "' are different";
    }
}
}  // namespace ark::ets::test
