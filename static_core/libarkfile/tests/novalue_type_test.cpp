/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "libarkfile/file_items.h"

namespace ark::panda_file::test {

TEST(NovalueTypeTest, TypeIdValue)
{
    ASSERT_EQ(Type::TypeId::NOVALUE, static_cast<Type::TypeId>(0xf));
}

TEST(NovalueTypeTest, IsPrimitive)
{
    Type t(Type::TypeId::NOVALUE);
    ASSERT_TRUE(t.IsPrimitive());
    ASSERT_FALSE(t.IsReference());
}

TEST(NovalueTypeTest, SignatureRoundTrip)
{
    Type t(Type::TypeId::NOVALUE);
    const char *sig = Type::GetSignatureByTypeId(t);
    ASSERT_STREQ(sig, "X");

    Type roundTrip = Type::GetTypeIdBySignature('X');
    ASSERT_EQ(roundTrip.GetId(), Type::TypeId::NOVALUE);
}

TEST(NovalueTypeTest, TypeProperties)
{
    Type t(Type::TypeId::NOVALUE);
    ASSERT_FALSE(t.IsSigned());
    ASSERT_FALSE(t.IsNumeric());
    ASSERT_FALSE(t.IsFloat());
    ASSERT_FALSE(t.IsIntegral());
    ASSERT_EQ(t.GetBitWidth(), 0UL);
}

}  // namespace ark::panda_file::test
