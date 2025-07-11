/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "tests/runtime/types/ets_test_mirror_classes.h"
#include "types/ets_escompat_array.h"

namespace ark::ets::test {

class EtsEscompatArrayMembers : public testing::Test {
public:
    EtsEscompatArrayMembers()
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

    ~EtsEscompatArrayMembers() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsEscompatArrayMembers);
    NO_MOVE_SEMANTIC(EtsEscompatArrayMembers);

    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsEscompatArray, actualLength_, "actualLength"),
                                             MIRROR_FIELD_INFO(EtsEscompatArray, buffer_, "buffer")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(EtsEscompatArrayMembers, MemoryLayout)
{
    EtsClass *klass = PlatformTypes(vm_)->escompatArray;
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

}  // namespace ark::ets::test
