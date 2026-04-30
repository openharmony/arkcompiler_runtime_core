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

#include <gtest/gtest.h>
#include "verification/type/type_type.h"

namespace ark::verifier::test {

TEST(VerifierTypeTest, FromTypeIdReturnsBot)
{
    Type t = Type::FromTypeId(panda_file::Type::TypeId::NOVALUE);
    ASSERT_TRUE(t.IsBuiltin());
    ASSERT_EQ(t.GetBuiltin(), Type::Builtin::BOT);
    ASSERT_EQ(t, Type::Bot());
}

TEST(VerifierTypeTest, VoidIsTop)
{
    Type t = Type::FromTypeId(panda_file::Type::TypeId::VOID);
    ASSERT_TRUE(t.IsBuiltin());
    ASSERT_EQ(t.GetBuiltin(), Type::Builtin::TOP);
    ASSERT_EQ(t, Type::Top());
}

TEST(VerifierTypeTest, BotIsSubtypeOfTop)
{
    ASSERT_TRUE(IsSubtype(Type::Bot(), Type::Top(), nullptr));
}

TEST(VerifierTypeTest, BotIsSubtypeOfAllPrimitiveBuiltins)
{
    using B = Type::Builtin;
    for (auto builtin : {B::U1, B::I8, B::U8, B::I16, B::U16, B::I32, B::U32, B::F32, B::F64, B::I64, B::U64}) {
        ASSERT_TRUE(IsSubtype(Type::Bot(), Type(builtin), nullptr))
            << "BOT should be subtype of builtin " << static_cast<int>(builtin);
    }
}

TEST(VerifierTypeTest, BotIsSubtypeOfReference)
{
    ASSERT_TRUE(IsSubtype(Type::Bot(), Type::Reference(), nullptr));
}

TEST(VerifierTypeTest, TopIsNotSubtypeOfBot)
{
    ASSERT_FALSE(IsSubtype(Type::Top(), Type::Bot(), nullptr));
}

TEST(VerifierTypeTest, BotIsSubtypeOfItself)
{
    ASSERT_TRUE(IsSubtype(Type::Bot(), Type::Bot(), nullptr));
}

TEST(VerifierTypeTest, BotIsConsistent)
{
    ASSERT_TRUE(Type::Bot().IsConsistent());
}

}  // namespace ark::verifier::test
