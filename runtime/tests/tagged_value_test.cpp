/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "runtime/include/coretypes/tagged_value.h"

namespace panda::test {

HWTEST(TaggedValueTest, GetTaggedValueTest, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(panda::coretypes::TaggedValue::GetIntTaggedValue(0U), 18446462598732840960UL);
    ASSERT_EQ(panda::coretypes::TaggedValue::GetDoubleTaggedValue(0U), 281474976710656UL);
    ASSERT_EQ(panda::coretypes::TaggedValue::GetBoolTaggedValue(0U), 6UL);
    ASSERT_EQ(panda::coretypes::TaggedValue::GetBoolTaggedValue(1U), 7UL);

    panda::coretypes::TaggedValue false_tagged(false);
    ASSERT_EQ(false_tagged.GetRawData(), 6UL);

    panda::coretypes::TaggedValue true_tagged(true);
    ASSERT_EQ(true_tagged.GetRawData(), 7UL);
}

HWTEST(TaggedValueTest, UndefinedTest, testing::ext::TestSize.Level0)
{
    panda::coretypes::TaggedValue double_tagged(0.0);

    ASSERT_FALSE(double_tagged.IsUndefined());
    ASSERT_EQ(panda::coretypes::TaggedValue::Undefined().GetRawData(), panda::coretypes::TaggedValue::VALUE_UNDEFINED);
}

}  // namespace panda::test
