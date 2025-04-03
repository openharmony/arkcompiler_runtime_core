/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"
#include "plugins/ets/runtime/ani/ani_mangle.h"

namespace ark::ets::ani::testing {

class MangleDescriptorTest : public AniTest {};

TEST_F(MangleDescriptorTest, Format_NewToOld)
{
    PandaString desc;

    desc = Mangle::ConvertDescriptor("a");
    EXPECT_STREQ(desc.c_str(), "La;");

    desc = Mangle::ConvertDescriptor("a.b");
    EXPECT_STREQ(desc.c_str(), "La/b;");

    desc = Mangle::ConvertDescriptor("a.b.c");
    EXPECT_STREQ(desc.c_str(), "La/b/c;");
}

TEST_F(MangleDescriptorTest, Format_OldToOld)
{
    PandaString desc;

    desc = Mangle::ConvertDescriptor("La;");
    EXPECT_STREQ(desc.c_str(), "La;");

    desc = Mangle::ConvertDescriptor("La/b;");
    EXPECT_STREQ(desc.c_str(), "La/b;");

    desc = Mangle::ConvertDescriptor("La/b/c;");
    EXPECT_STREQ(desc.c_str(), "La/b/c;");
}

TEST_F(MangleDescriptorTest, Format_Wrong)
{
    PandaString desc;

    desc = Mangle::ConvertDescriptor("");
    EXPECT_STREQ(desc.c_str(), "");

    desc = Mangle::ConvertDescriptor("a;");
    EXPECT_STREQ(desc.c_str(), "a;");

    desc = Mangle::ConvertDescriptor("a/b");
    EXPECT_STREQ(desc.c_str(), "a/b");

    desc = Mangle::ConvertDescriptor("a/b/c");
    EXPECT_STREQ(desc.c_str(), "a/b/c");
}

TEST_F(MangleDescriptorTest, FindModule)
{
    ani_module mod {};
    EXPECT_EQ(env_->FindModule("mm.mangle_descriptor_test", &mod), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindModule_OldFormat)
{
    ani_module mod {};
    EXPECT_EQ(env_->FindModule("Lmm/mangle_descriptor_test;", &mod), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindNamespace)
{
    ani_namespace ns {};
    EXPECT_EQ(env_->FindNamespace("mm.mangle_descriptor_test.rls", &ns), ANI_OK);
    EXPECT_EQ(env_->FindNamespace("mm.mangle_descriptor_test.rls.ns", &ns), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindNamespace_OldFormat)
{
    ani_namespace ns {};
    EXPECT_EQ(env_->FindNamespace("Lmm/mangle_descriptor_test/rls;", &ns), ANI_OK);
    EXPECT_EQ(env_->FindNamespace("Lmm/mangle_descriptor_test/rls/ns;", &ns), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindClass)
{
    ani_class cls {};
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.A", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.B", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.A", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.B", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.ns.A", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("mm.mangle_descriptor_test.rls.ns.B", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindClass_OldFormat)
{
    ani_class cls {};
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/A;", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/B;", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/A;", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/B;", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/ns/A;", &cls), ANI_OK);
    EXPECT_EQ(env_->FindClass("Lmm/mangle_descriptor_test/rls/ns/B;", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindEnum)
{
    ani_enum enm {};
    EXPECT_EQ(env_->FindEnum("mm.mangle_descriptor_test.E", &enm), ANI_OK);
    EXPECT_EQ(env_->FindEnum("mm.mangle_descriptor_test.rls.E", &enm), ANI_OK);
    EXPECT_EQ(env_->FindEnum("mm.mangle_descriptor_test.rls.ns.E", &enm), ANI_OK);
}

TEST_F(MangleDescriptorTest, FindEnum_OldFormat)
{
    ani_enum enm {};
    EXPECT_EQ(env_->FindEnum("Lmm/mangle_descriptor_test/E;", &enm), ANI_OK);
    EXPECT_EQ(env_->FindEnum("Lmm/mangle_descriptor_test/rls/E;", &enm), ANI_OK);
    EXPECT_EQ(env_->FindEnum("Lmm/mangle_descriptor_test/rls/ns/E;", &enm), ANI_OK);
}

TEST_F(MangleDescriptorTest, Module_FindNamespace)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule("mm.mangle_descriptor_test", &mod), ANI_OK);

    ani_namespace ns {};
    EXPECT_EQ(env_->Module_FindNamespace(mod, "rls", &ns), ANI_OK);
    EXPECT_EQ(env_->Module_FindNamespace(mod, "rls.ns", &ns), ANI_OK);
}

TEST_F(MangleDescriptorTest, Module_FindNamespace_OldFormat)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule("Lmm/mangle_descriptor_test;", &mod), ANI_OK);

    ani_namespace ns {};
    EXPECT_EQ(env_->Module_FindNamespace(mod, "Lrls;", &ns), ANI_OK);
    EXPECT_EQ(env_->Module_FindNamespace(mod, "Lrls/ns;", &ns), ANI_OK);
}

TEST_F(MangleDescriptorTest, Module_FindClass)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule("mm.mangle_descriptor_test", &mod), ANI_OK);

    ani_class cls {};
    EXPECT_EQ(env_->Module_FindClass(mod, "A", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "B", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "rls.A", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "rls.B", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "rls.ns.A", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "rls.ns.B", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, Module_FindClass_OldFormat)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule("Lmm/mangle_descriptor_test;", &mod), ANI_OK);

    ani_class cls {};
    EXPECT_EQ(env_->Module_FindClass(mod, "LA;", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "LB;", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "Lrls/A;", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "Lrls/B;", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "Lrls/ns/A;", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "Lrls/ns/B;", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, Module_FindEnum)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule("mm.mangle_descriptor_test", &mod), ANI_OK);

    ani_enum enm {};
    EXPECT_EQ(env_->Module_FindEnum(mod, "E", &enm), ANI_OK);
    EXPECT_EQ(env_->Module_FindEnum(mod, "rls.E", &enm), ANI_OK);
    EXPECT_EQ(env_->Module_FindEnum(mod, "rls.ns.E", &enm), ANI_OK);
}

TEST_F(MangleDescriptorTest, Module_FindEnum_OldFormat)
{
    ani_module mod {};
    ASSERT_EQ(env_->FindModule("Lmm/mangle_descriptor_test;", &mod), ANI_OK);

    ani_class cls {};
    EXPECT_EQ(env_->Module_FindClass(mod, "LE;", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "Lrls/E;", &cls), ANI_OK);
    EXPECT_EQ(env_->Module_FindClass(mod, "Lrls/ns/E;", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, Namespace_FindNamespace)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("mm.mangle_descriptor_test.rls", &ns), ANI_OK);

    ani_namespace ns2 {};
    EXPECT_EQ(env_->Namespace_FindNamespace(ns, "ns", &ns2), ANI_OK);
}

TEST_F(MangleDescriptorTest, Namespace_FindNamespace_OldFormat)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lmm/mangle_descriptor_test/rls;", &ns), ANI_OK);

    ani_namespace ns2 {};
    EXPECT_EQ(env_->Namespace_FindNamespace(ns, "Lns;", &ns2), ANI_OK);
}

TEST_F(MangleDescriptorTest, Namespace_FindClass)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("mm.mangle_descriptor_test.rls", &ns), ANI_OK);

    ani_class cls {};
    EXPECT_EQ(env_->Namespace_FindClass(ns, "A", &cls), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindClass(ns, "B", &cls), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindClass(ns, "ns.A", &cls), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindClass(ns, "ns.B", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, Namespace_FindClass_OldFormat)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lmm/mangle_descriptor_test/rls;", &ns), ANI_OK);

    ani_class cls {};
    EXPECT_EQ(env_->Namespace_FindClass(ns, "LA;", &cls), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindClass(ns, "LB;", &cls), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindClass(ns, "Lns/A;", &cls), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindClass(ns, "Lns/B;", &cls), ANI_OK);
}

TEST_F(MangleDescriptorTest, Namespace_FindEnum)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("mm.mangle_descriptor_test.rls", &ns), ANI_OK);

    ani_enum enm {};
    EXPECT_EQ(env_->Namespace_FindEnum(ns, "E", &enm), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindEnum(ns, "ns.E", &enm), ANI_OK);
}

TEST_F(MangleDescriptorTest, Namespace_FindEnum_OldFormat)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lmm/mangle_descriptor_test/rls;", &ns), ANI_OK);

    ani_enum enm {};
    EXPECT_EQ(env_->Namespace_FindEnum(ns, "LE;", &enm), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindEnum(ns, "Lns/E;", &enm), ANI_OK);
}

}  // namespace ark::ets::ani::testing
