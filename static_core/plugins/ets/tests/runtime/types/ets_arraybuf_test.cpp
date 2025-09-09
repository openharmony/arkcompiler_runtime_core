/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "types/ets_arraybuffer.h"
#include "tests/runtime/types/ets_test_mirror_classes.h"

namespace ark::ets::test {

class EtsArrayBufferTest : public EtsMirrorClassTestBase {
public:
    static std::vector<MirrorFieldInfo> GetMembers()
    {
        return std::vector<MirrorFieldInfo> {MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, managedData_, "data"),
                                             MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, byteLength_, "_byteLength"),
                                             MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, nativeData_, "dataAddress"),
                                             MIRROR_FIELD_INFO(EtsEscompatArrayBuffer, isResizable_, "isResizable")};
    }
};

TEST_F(EtsArrayBufferTest, MemoryLayout)
{
    EtsClass *klass = GetPlatformTypes()->escompatArrayBuffer;
    MirrorFieldInfo::CompareMemberOffsets(klass, GetMembers());
}
}  // namespace ark::ets::test
