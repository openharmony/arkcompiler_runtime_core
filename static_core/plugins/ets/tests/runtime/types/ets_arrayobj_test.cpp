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

#include <gtest/gtest.h>

#include "ets_coroutine.h"

#include "types/ets_class.h"
#include "types/ets_arraybuffer.h"
#include "tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

template <class ElementType>
class EtsArrayObjectMembers : public testing::Test {
public:
    EtsArrayObjectMembers()
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

    ~EtsArrayObjectMembers() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsArrayObjectMembers);
    NO_MOVE_SEMANTIC(EtsArrayObjectMembers);

    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsArrayObject<ElementType>, actualLength_, "actualLength"),
            MIRROR_FIELD_INFO(EtsArrayObject<ElementType>, buffer_, "buffer")};
    }

protected:
    PandaEtsVM *vm_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

using EtsArrayBoxedBooleanTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsBoolean>>;
using EtsArrayBoxedByteTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsByte>>;
using EtsArrayBoxedCharTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsChar>>;
using EtsArrayBoxedShortTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsShort>>;
using EtsArrayBoxedIntTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsInt>>;
using EtsArrayBoxedLongTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsLong>>;
using EtsArrayBoxedFloatTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsFloat>>;
using EtsArrayBoxedDoubleTest = EtsArrayObjectMembers<EtsBoxPrimitive<EtsDouble>>;

TEST_F(EtsArrayBoxedBooleanTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsArrayBoxedByteTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsArrayBoxedCharTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsArrayBoxedShortTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsArrayBoxedIntTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsArrayBoxedLongTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsArrayBoxedFloatTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

TEST_F(EtsArrayBoxedDoubleTest, MemoryLayout)
{
    EtsClass *klass = vm_->GetClassLinker()->GetArrayClass();
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}

}  // namespace ark::ets::test
