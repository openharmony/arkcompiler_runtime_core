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
#include <string>
#include "operand_types_print.h"

namespace ark::test {

TEST(typetests, test1)
{
    std::string_view descriptor1;
    ark::pandasm::Type type1 = ark::pandasm::Type::FromDescriptor(descriptor1);
    ASSERT_FALSE(type1.IsValid());

    std::string_view descriptor2 = "X";
    EXPECT_DEATH({ ark::pandasm::Type::FromDescriptor(descriptor2); }, ".*");

    std::string_view descriptor3 = "Z";
    std::string descriptor3Expect = "u1";
    ark::pandasm::Type type3 = ark::pandasm::Type::FromDescriptor(descriptor3);
    ASSERT_EQ(type3.GetComponentName(), descriptor3Expect);

    std::string_view descriptor4 = "Lobj.Obj;";
    std::string descriptor4Expect = "obj.Obj";
    ark::pandasm::Type type4 = ark::pandasm::Type::FromDescriptor(descriptor4);
    ASSERT_EQ(type4.GetComponentName(), descriptor4Expect);

    std::string_view descriptor5 = "[Lobj.Obj;";
    std::string descriptor5Expect = "obj.Obj";
    ark::pandasm::Type type5 = ark::pandasm::Type::FromDescriptor(descriptor5);
    ASSERT_EQ(type5.GetComponentName(), descriptor5Expect);
    ASSERT_EQ(type5.GetRank(), 1);

    std::string_view descriptor6 = "[[[Lobj.Obj;";
    std::string descriptor6Expect = "obj.Obj";
    ark::pandasm::Type type6 = ark::pandasm::Type::FromDescriptor(descriptor6);
    ASSERT_EQ(type6.GetComponentName(), descriptor6Expect);
    ASSERT_EQ(type6.GetRank(), 3);
}

TEST(typetests, malformed_union_descriptor_canonicalize_returns_empty)
{
    std::string_view malformedUnionDescriptor = "{UI}";
    ASSERT_TRUE(ark::pandasm::Type::CanonicalizeDescriptor(malformedUnionDescriptor).empty());

    std::string_view malformedArrayUnionDescriptor = "{U[I}";
    ASSERT_TRUE(ark::pandasm::Type::CanonicalizeDescriptor(malformedArrayUnionDescriptor).empty());

    std::string_view malformedNestedUnionDescriptor = "{U{U[I}}";
    ASSERT_TRUE(ark::pandasm::Type::CanonicalizeDescriptor(malformedNestedUnionDescriptor).empty());
}

TEST(typetests, malformed_descriptor_returns_invalid_type)
{
    std::string_view malformedArrayDescriptor = "[";
    ark::pandasm::Type malformedArrayType = ark::pandasm::Type::FromDescriptor(malformedArrayDescriptor);
    ASSERT_FALSE(malformedArrayType.IsValid());

    std::string_view malformedReferenceDescriptor = "L;";
    ark::pandasm::Type malformedReferenceType = ark::pandasm::Type::FromDescriptor(malformedReferenceDescriptor);
    ASSERT_FALSE(malformedReferenceType.IsValid());
}

TEST(typetests, malformed_type_name_returns_invalid_type)
{
    ark::pandasm::Type missingBaseType = ark::pandasm::Type::FromName("[]");
    ASSERT_FALSE(missingBaseType.IsValid());
    ASSERT_TRUE(missingBaseType.GetDescriptor().empty());

    ark::pandasm::Type missingBaseTypeRankTwo = ark::pandasm::Type::FromName("[][]");
    ASSERT_FALSE(missingBaseTypeRankTwo.IsValid());
    ASSERT_TRUE(missingBaseTypeRankTwo.GetDescriptor().empty());

    ark::pandasm::Type malformedSuffix = ark::pandasm::Type::FromName("i32]]");
    ASSERT_FALSE(malformedSuffix.IsValid());
    ASSERT_TRUE(malformedSuffix.GetDescriptor().empty());
}

}  // namespace ark::test
