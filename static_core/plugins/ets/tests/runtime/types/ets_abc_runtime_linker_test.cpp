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

#include "ets_coroutine.h"
#include "ets_vm.h"

#include "types/ets_abc_file.h"
#include "types/ets_abc_runtime_linker.h"
#include "tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsAbcRuntimeLinkerTest : public testing::Test {
public:
    EtsAbcRuntimeLinkerTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("g1-gc");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsAbcRuntimeLinkerTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsAbcRuntimeLinkerTest);
    NO_MOVE_SEMANTIC(EtsAbcRuntimeLinkerTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        vm_ = coroutine_->GetPandaVM();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    static std::vector<MirrorFieldInfo> GetEtsAbcClassMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsAbcFile, fileHandlePtr_, "fileHandlePtr")};
    }

    static std::vector<MirrorFieldInfo> GetEtsAbcRuntimeLinkerClassMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsAbcRuntimeLinker, parentLinker_, "parentLinker"),
                                             MIRROR_FIELD_INFO(EtsAbcRuntimeLinker, abcFiles_, "abcFiles")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsAbcRuntimeLinkerTest, AbcFileMemoryLayout)
{
    EtsClass *abcFileClass = PlatformTypes(vm_)->coreAbcFile;
    MirrorFieldInfo::CompareMemberOffsets(abcFileClass, GetEtsAbcClassMembers());
}

TEST_F(EtsAbcRuntimeLinkerTest, AbcRuntimeLinkerMemoryLayout)
{
    EtsClass *abcRuntimeLinkerClass = PlatformTypes(vm_)->coreAbcRuntimeLinker;
    MirrorFieldInfo::CompareMemberOffsets(abcRuntimeLinkerClass, GetEtsAbcRuntimeLinkerClassMembers());
}

}  // namespace ark::ets::test
