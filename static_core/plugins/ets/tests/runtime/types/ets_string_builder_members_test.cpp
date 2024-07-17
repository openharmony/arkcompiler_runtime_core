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

#include "intrinsics.h"
#include <gtest/gtest.h>
#include "ets_coroutine.h"
#include "types/ets_string.h"
#include "types/ets_array.h"
#include "tests/runtime/types/ets_test_mirror_classes.h"
#include "types/ets_string_builder.h"
#include "libpandabase/utils/utf.h"

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

namespace ark::ets::test {

class EtsStringBuilderMembersTest : public testing::Test {
public:
    EtsStringBuilderMembersTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
        vm_ = EtsCoroutine::GetCurrent()->GetPandaVM();
    }

    ~EtsStringBuilderMembersTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsStringBuilderMembersTest);
    NO_MOVE_SEMANTIC(EtsStringBuilderMembersTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsStringBuilder, buf_, "buf"),
                                             MIRROR_FIELD_INFO(EtsStringBuilder, index_, "index"),
                                             MIRROR_FIELD_INFO(EtsStringBuilder, length_, "length"),
                                             MIRROR_FIELD_INFO(EtsStringBuilder, compress_, "compress")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsStringBuilderMembersTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetStringBuilderClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTEND(readability-magic-numbers)
