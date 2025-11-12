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

#include "ets_mirror_class_test_base.h"

#include "plugins/ets/runtime/types/atomics/ets_atomic_primitives.h"
#include "types/ets_class.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

// CC-OFFNXT(G.PRE.02-CPP): code generation
#define DEFINE_MIRROR_FIELD_INFO_TEST(type)                                          \
    TEST_F(EtsAtomicPrimitiveMembers, MemoryLayoutOf##type)                          \
    {                                                                                \
        auto *klass = GetClassLinker()->GetClass("Lstd/concurrency/" #type ";");     \
        ASSERT(klass != nullptr);                                                    \
        MirrorFieldInfo::CompareMemberOffsets(klass, GetAtomicMembers<Ets##type>()); \
    }

class EtsAtomicPrimitiveMembers : public EtsMirrorClassTestBase {
public:
    template <class T>
    static std::vector<MirrorFieldInfo> GetAtomicMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(T, value_, "val_")};
    }
};

DEFINE_MIRROR_FIELD_INFO_TEST(AtomicInt);
DEFINE_MIRROR_FIELD_INFO_TEST(AtomicShort);
DEFINE_MIRROR_FIELD_INFO_TEST(AtomicByte);
DEFINE_MIRROR_FIELD_INFO_TEST(AtomicLong);
DEFINE_MIRROR_FIELD_INFO_TEST(AtomicChar);
DEFINE_MIRROR_FIELD_INFO_TEST(AtomicBoolean);
DEFINE_MIRROR_FIELD_INFO_TEST(AtomicFloat);
DEFINE_MIRROR_FIELD_INFO_TEST(AtomicDouble);

#undef DEFINE_MIRROR_FIELD_INFO_TEST

}  // namespace ark::ets::test
