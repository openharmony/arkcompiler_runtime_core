/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "types/ets_class.h"
#include "types/ets_typed_arrays.h"
#include "plugins/ets/tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsCoreTypedArrayBaseTest : public EtsMirrorClassTestBase {
public:
    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {
            MIRROR_FIELD_INFO(EtsCoreTypedArrayBase, buffer_, "buffer"),
            MIRROR_FIELD_INFO(EtsCoreTypedArrayBase, name_, "name"),
            MIRROR_FIELD_INFO(EtsCoreTypedArrayBase, bytesPerElement_, "BYTES_PER_ELEMENT"),
            MIRROR_FIELD_INFO(EtsCoreTypedArrayBase, byteOffset_, "byteOffset"),
            MIRROR_FIELD_INFO(EtsCoreTypedArrayBase, byteLength_, "byteLength"),
            MIRROR_FIELD_INFO(EtsCoreTypedArrayBase, lengthInt_, "lengthInt")};
    }
};

TEST_F(EtsCoreTypedArrayBaseTest, MemoryLayout)
{
    static constexpr std::array TYPED_ARRAYS = {
        "Lstd/core/Int8Array;",     "Lstd/core/Int16Array;",   "Lstd/core/Int32Array;",
        "Lstd/core/BigInt64Array;", "Lstd/core/Float32Array;", "Lstd/core/Float64Array;",
    };
    auto *linker = GetClassLinker();
    for (const auto &desc : TYPED_ARRAYS) {
        auto *klass = linker->GetClass(desc);
        MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
    }
}
}  // namespace ark::ets::test
