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

#include <gtest/gtest.h>
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/types/ets_method_signature.h"

namespace ark::ets::test {

TEST(EtsTypeNeverTest, PandaToEtsConversion)
{
    auto pandaType = panda_file::Type(panda_file::Type::TypeId::NOVALUE);
    auto etsType = ConvertPandaTypeToEtsType(pandaType);
    ASSERT_EQ(etsType, EtsType::NOVALUE);
}

TEST(EtsTypeNeverTest, EtsToPandaConversion)
{
    auto roundTrip = ConvertEtsTypeToPandaType(EtsType::NOVALUE);
    ASSERT_EQ(roundTrip.GetId(), panda_file::Type::TypeId::NOVALUE);
}

TEST(EtsTypeNeverTest, ToString)
{
    ASSERT_EQ(EtsTypeToString(EtsType::NOVALUE), "novalue");
}

TEST(EtsMethodSignatureNeverTest, NeverAsReturnTypeValid)
{
    EtsMethodSignature sig("I:X");
    ASSERT_TRUE(sig.IsValid());
}

TEST(EtsMethodSignatureNeverTest, NeverAsParamInvalid)
{
    EtsMethodSignature sig("X:I");
    ASSERT_FALSE(sig.IsValid());
}

TEST(EtsMethodSignatureNeverTest, NeverNoParamsValid)
{
    EtsMethodSignature sig(":X");
    ASSERT_TRUE(sig.IsValid());
}

TEST(EtsMethodSignatureNeverTest, VoidAsParamInvalid)
{
    EtsMethodSignature sig("V:I");
    ASSERT_FALSE(sig.IsValid());
}

TEST(EtsMethodSignatureNeverTest, MixedParamsWithNeverReturn)
{
    EtsMethodSignature sig("IJ:X");
    ASSERT_TRUE(sig.IsValid());
}

}  // namespace ark::ets::test
