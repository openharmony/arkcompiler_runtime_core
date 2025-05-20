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

#include "types/ets_runtime_linker.h"
#include "tests/runtime/types/ets_test_mirror_classes.h"

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "runtime/include/class_linker.h"
#include "libpandabase/utils/utf.h"

#include "ets_coroutine.h"
#include "ets_vm.h"

namespace ark::ets::test {

class EtsRuntimeLinkerTest : public testing::Test {
public:
    EtsRuntimeLinkerTest()
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
    }

    ~EtsRuntimeLinkerTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsRuntimeLinkerTest);
    NO_MOVE_SEMANTIC(EtsRuntimeLinkerTest);

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

    static std::vector<MirrorFieldInfo> GetClassMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsRuntimeLinker, classLinkerCtxPtr_, "classLinkerCtxPtr")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsRuntimeLinkerTest, RuntimeLinkerMemoryLayout)
{
    EtsClass *runtimeLinkerClass = PlatformTypes(vm_)->coreRuntimeLinker;
    MirrorFieldInfo::CompareMemberOffsets(runtimeLinkerClass, GetClassMembers());
}

class SuppressErrorHandler : public ark::ClassLinkerErrorHandler {
    void OnError([[maybe_unused]] ark::ClassLinker::Error error,
                 [[maybe_unused]] const ark::PandaString &message) override
    {
    }
};

TEST_F(EtsRuntimeLinkerTest, GetClassReturnsNullWhenErrorSuppressed)
{
    auto *ext = static_cast<ark::ets::EtsClassLinkerExtension *>(
        ark::Runtime::GetCurrent()->GetClassLinker()->GetExtension(ark::panda_file::SourceLang::ETS));

    const auto *fakeDescriptor = reinterpret_cast<const uint8_t *>("Lark/test/Fake;");
    SuppressErrorHandler handler;

    Class *klass = ext->GetClass(fakeDescriptor, true, nullptr, &handler);
    ASSERT_EQ(klass, nullptr);
}

}  // namespace ark::ets::test
