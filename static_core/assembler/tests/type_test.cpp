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

// MAX_UNION_DEPTH is 8: the depth counter increments for every union component
// (including primitives), so the effective max union-nesting is 8 levels.
// In practice, the deepest real-world union nesting is 2-3 levels (union types
// are associative and normalized to flat by the type system).
// A flat union with many components does NOT hit the limit (all siblings share depth 1).
static constexpr int MAX_UNION_DEPTH = 8;

TEST(typetests, test1)
{
    std::string_view descriptor1;
    ark::pandasm::Type type1 = ark::pandasm::Type::FromDescriptor(descriptor1);
    ASSERT_FALSE(type1.IsValid());

    // Unknown descriptor "X" is not in reversePrimitiveTypes. FromDescriptorComponent
    // uses LOG(FATAL) because the map should contain all valid primitive descriptors —
    // an unknown descriptor indicates a corrupted/malicious .abc file, and aborting is
    // appropriate for the developer tools that call FromDescriptor.
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

// Helper: build a deeply-nested union descriptor with N levels of {U wrappers
// around a two-component leaf {UIF}.  E.g. N=2 → "{U{U{UIF}}}"
static std::string BuildNestedUnionDescriptor(int nestingDepth)
{
    std::string desc;
    for (int i = 0; i < nestingDepth; ++i) {
        desc += "{U";
    }
    desc += "{UIF}";
    for (int i = 0; i < nestingDepth; ++i) {
        desc += "}";
    }
    return desc;
}

// Boundary test: depth counter increments for EVERY union component (including
// primitives).  With MAX_UNION_DEPTH=8:
//   - N wrappers → inner union at depth N, its primitives at depth N+1
//   - N=7: primitives at depth 8 → 8 > 8 is false → passes
//   - N=8: primitives at depth 9 → 9 > 8 is true → fails
TEST(typetests, union_depth_limit_boundary)
{
    // At the limit (7 wrappers + {UIF}): depth 8 for leaf primitives — passes
    std::string atLimit = BuildNestedUnionDescriptor(MAX_UNION_DEPTH - 1);
    ark::pandasm::Type typeAtLimit = ark::pandasm::Type::FromDescriptor(atLimit);
    // Depth check did NOT trigger: FromDescriptorImpl returned a non-empty result
    ASSERT_FALSE(typeAtLimit.GetName().empty());

    // Over the limit (8 wrappers + {UIF}): depth 9 for leaf primitives — fails
    std::string overLimit = BuildNestedUnionDescriptor(MAX_UNION_DEPTH);
    ark::pandasm::Type typeOverLimit = ark::pandasm::Type::FromDescriptor(overLimit);
    // Depth check triggered: FromDescriptorImpl returned {"", 0}, FromDescriptor returned Type()
    ASSERT_TRUE(typeOverLimit.GetName().empty());
    ASSERT_FALSE(typeOverLimit.IsValid());
}

// A flat union with many components does NOT hit the depth limit because all
// sibling components are called at the same depth (1), not accumulating.
TEST(typetests, flat_union_many_components_not_limited_by_depth)
{
    // Build "{U" + N * "I" + "F" + "}" — N+1 components, all at depth 1
    constexpr int FLAT_UNION_COMPONENT_COUNT = 200;
    std::string flatDesc = "{U";
    for (int i = 0; i < FLAT_UNION_COMPONENT_COUNT; ++i) {
        flatDesc += "I";
    }
    flatDesc += "F}";

    ark::pandasm::Type flatType = ark::pandasm::Type::FromDescriptor(flatDesc);
    ASSERT_FALSE(flatType.GetName().empty());
    // After Canonicalize, duplicate "i32" components are removed, leaving {Uf32,i32}
    ASSERT_TRUE(flatType.IsValid());
}

TEST(typetests, malformed_descriptor_returns_invalid_type)
{
    std::string_view malformedArrayDescriptor = "[";
    ark::pandasm::Type malformedArrayType = ark::pandasm::Type::FromDescriptor(malformedArrayDescriptor);
    ASSERT_FALSE(malformedArrayType.IsValid());

    std::string_view malformedReferenceDescriptor = "L;";
    ark::pandasm::Type malformedReferenceType = ark::pandasm::Type::FromDescriptor(malformedReferenceDescriptor);
    ASSERT_FALSE(malformedReferenceType.IsValid());

    ark::pandasm::Type malformedReferenceSuffix = ark::pandasm::Type::FromDescriptor("L];");
    ASSERT_EQ(malformedReferenceSuffix.GetNameWithoutRank(), "]");
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
