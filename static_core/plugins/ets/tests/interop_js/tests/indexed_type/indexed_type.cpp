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
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsIndexedType : public EtsInteropTest {};

TEST_F(EtsIndexedType, getArrValueByIndex)
{
    auto ret = CallEtsMethod<bool>("getArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, changeArrValueByIndex)
{
    auto ret = CallEtsMethod<bool>("changeArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkLengthArr)
{
    auto ret = CallEtsMethod<bool>("checkLengthArr");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkAllArrValue)
{
    auto ret = CallEtsMethod<bool>("checkAllArrValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, getCustomArrValueByIndex)
{
    auto ret = CallEtsMethod<bool>("getCustomArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, changeCustomArrValueByIndex)
{
    auto ret = CallEtsMethod<bool>("changeCustomArrValueByIndex");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkLengthCustomArr)
{
    auto ret = CallEtsMethod<bool>("checkLengthCustomArr");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, checkAllCustomArrValue)
{
    auto ret = CallEtsMethod<bool>("checkAllCustomArrValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, getRecordValue)
{
    auto ret = CallEtsMethod<bool>("getRecordValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, changeRecordValue)
{
    auto ret = CallEtsMethod<bool>("changeRecordValue");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_getRecordValueIndex)
{
    auto ret = CallEtsMethod<bool>("getRecordValueIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_changeRecordValueIndex)
{
    auto ret = CallEtsMethod<bool>("changrRecordValueIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_checkRecordValueLength)
{
    auto ret = CallEtsMethod<bool>("checkRecordValueLength");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #17821
TEST_F(EtsIndexedType, DISABLED_checkRecordValue)
{
    auto ret = CallEtsMethod<bool>("checkRecordValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, getTypedArrayValueByIndex)
{
    auto ret = CallEtsMethod<bool>("getTypedArrayValueByIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18131
TEST_F(EtsIndexedType, DISABLED_changeTypedArrayValueByIndex)
{
    auto ret = CallEtsMethod<bool>("changeTypedArrayValueByIndex");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18131
TEST_F(EtsIndexedType, DISABLED_checkLengthTypedArray)
{
    auto ret = CallEtsMethod<bool>("checkLengthTypedArray");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18131
TEST_F(EtsIndexedType, DISABLED_checkAllTypedArrayValue)
{
    auto ret = CallEtsMethod<bool>("checkAllTypedArrayValue");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, getValueFromProxyRecord)
{
    auto ret = CallEtsMethod<bool>("getValueFromProxyRecord");
    ASSERT_EQ(ret, true);
}
TEST_F(EtsIndexedType, changeValueFromProxyRecord)
{
    auto ret = CallEtsMethod<bool>("changeValueFromProxyRecord");
    ASSERT_EQ(ret, true);
}
// NOTE(andreypetukhov) enable after fixibng #18238
TEST_F(EtsIndexedType, DISABLED_getValueFromProxyByKeyRecord)
{
    auto ret = CallEtsMethod<bool>("getValueFromProxyByKeyRecord");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing