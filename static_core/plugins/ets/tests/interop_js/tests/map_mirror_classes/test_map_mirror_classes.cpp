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
#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/thread_scopes.h"

namespace panda::ets::interop::js::testing {

class EtsInteropJsClassLinkerTest : public EtsInteropTest {};

struct MemberInfo {
    uint32_t offset;
    const char *name;
};

static void CheckOffsetOfFields(const char *class_name, const std::vector<MemberInfo> &members_list)
{
    ScopedManagedCodeThread scoped(ManagedThread::GetCurrent());

    EtsClassLinker *ets_class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
    EtsClass *klass = ets_class_linker->GetClass(class_name);
    ASSERT_NE(klass, nullptr);
    ASSERT_EQ(klass->GetInstanceFieldsNumber(), members_list.size());

    for (const auto &member_info : members_list) {
        EtsField *field = klass->GetFieldIDByOffset(member_info.offset);
        ASSERT_NE(field, nullptr);
        EXPECT_STREQ(field->GetName(), member_info.name);
    }
}

class JSValueOffsets {
public:
    static std::vector<MemberInfo> GetMembersInfo()
    {
        return std::vector<MemberInfo> {
            MemberInfo {MEMBER_OFFSET(JSValue, type_), "__internal_field_0"},  // long
            MemberInfo {MEMBER_OFFSET(JSValue, data_), "__internal_field_1"},  // long
        };
    }
};

TEST_F(EtsInteropJsClassLinkerTest, Filed_std_interop_js_JSValue)
{
    CheckOffsetOfFields("Lstd/interop/js/JSValue;", JSValueOffsets::GetMembersInfo());
}

}  // namespace panda::ets::interop::js::testing
