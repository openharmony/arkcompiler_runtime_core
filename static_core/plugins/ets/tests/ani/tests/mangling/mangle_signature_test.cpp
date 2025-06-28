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

class MangleSignatureTest : public AniTest {};

TEST_F(MangleSignatureTest, FormatVoid_NewToOld)
{
    PandaString desc;

    desc = Mangle::ConvertSignature(":");
    EXPECT_STREQ(desc.c_str(), ":V");
}

TEST_F(MangleSignatureTest, FormatVoid_OldToOld)
{
    PandaString desc;

    desc = Mangle::ConvertSignature(":V");
    EXPECT_STREQ(desc.c_str(), ":V");
}

TEST_F(MangleSignatureTest, FormatPrimitives_NewToOld)
{
    PandaString desc;

    // Check 'boolean'
    desc = Mangle::ConvertSignature("z:");
    EXPECT_STREQ(desc.c_str(), "Z:V");
    desc = Mangle::ConvertSignature(":z");
    EXPECT_STREQ(desc.c_str(), ":Z");

    // Check 'char'
    desc = Mangle::ConvertSignature("c:");
    EXPECT_STREQ(desc.c_str(), "C:V");
    desc = Mangle::ConvertSignature(":c");
    EXPECT_STREQ(desc.c_str(), ":C");

    // Check 'byte'
    desc = Mangle::ConvertSignature("b:");
    EXPECT_STREQ(desc.c_str(), "B:V");
    desc = Mangle::ConvertSignature(":b");
    EXPECT_STREQ(desc.c_str(), ":B");

    // Check 'short'
    desc = Mangle::ConvertSignature("s:");
    EXPECT_STREQ(desc.c_str(), "S:V");
    desc = Mangle::ConvertSignature(":s");
    EXPECT_STREQ(desc.c_str(), ":S");

    // Check 'int'
    desc = Mangle::ConvertSignature("i:");
    EXPECT_STREQ(desc.c_str(), "I:V");
    desc = Mangle::ConvertSignature(":i");
    EXPECT_STREQ(desc.c_str(), ":I");

    // Check 'long'
    desc = Mangle::ConvertSignature("l:");
    EXPECT_STREQ(desc.c_str(), "J:V");
    desc = Mangle::ConvertSignature(":l");
    EXPECT_STREQ(desc.c_str(), ":J");

    // Check 'float'
    desc = Mangle::ConvertSignature("f:");
    EXPECT_STREQ(desc.c_str(), "F:V");
    desc = Mangle::ConvertSignature(":f");
    EXPECT_STREQ(desc.c_str(), ":F");

    // Check 'double'
    desc = Mangle::ConvertSignature("d:");
    EXPECT_STREQ(desc.c_str(), "D:V");
    desc = Mangle::ConvertSignature(":d");
    EXPECT_STREQ(desc.c_str(), ":D");

    // Check mixed primitives
    desc = Mangle::ConvertSignature("zcbsilfd:z");
    EXPECT_STREQ(desc.c_str(), "ZCBSIJFD:Z");
}

TEST_F(MangleSignatureTest, FormatPrimitives_OldToOld)
{
    PandaString desc;

    // Check 'boolean'
    desc = Mangle::ConvertSignature("Z:V");
    EXPECT_STREQ(desc.c_str(), "Z:V");
    desc = Mangle::ConvertSignature(":Z");
    EXPECT_STREQ(desc.c_str(), ":Z");

    // Check 'char'
    desc = Mangle::ConvertSignature("C:V");
    EXPECT_STREQ(desc.c_str(), "C:V");
    desc = Mangle::ConvertSignature(":C");
    EXPECT_STREQ(desc.c_str(), ":C");

    // Check 'byte'
    desc = Mangle::ConvertSignature("B:V");
    EXPECT_STREQ(desc.c_str(), "B:V");
    desc = Mangle::ConvertSignature(":B");
    EXPECT_STREQ(desc.c_str(), ":B");

    // Check 'short'
    desc = Mangle::ConvertSignature("S:V");
    EXPECT_STREQ(desc.c_str(), "S:V");
    desc = Mangle::ConvertSignature(":S");
    EXPECT_STREQ(desc.c_str(), ":S");

    // Check 'int'
    desc = Mangle::ConvertSignature("I:V");
    EXPECT_STREQ(desc.c_str(), "I:V");
    desc = Mangle::ConvertSignature(":I");
    EXPECT_STREQ(desc.c_str(), ":I");

    // Check 'long'
    desc = Mangle::ConvertSignature("J:V");
    EXPECT_STREQ(desc.c_str(), "J:V");
    desc = Mangle::ConvertSignature(":J");
    EXPECT_STREQ(desc.c_str(), ":J");

    // Check 'float'
    desc = Mangle::ConvertSignature("F:V");
    EXPECT_STREQ(desc.c_str(), "F:V");
    desc = Mangle::ConvertSignature(":F");
    EXPECT_STREQ(desc.c_str(), ":F");

    // Check 'double'
    desc = Mangle::ConvertSignature("D:V");
    EXPECT_STREQ(desc.c_str(), "D:V");
    desc = Mangle::ConvertSignature(":D");
    EXPECT_STREQ(desc.c_str(), ":D");

    // Check mixed primitives
    desc = Mangle::ConvertSignature("ZCBSIJFD:Z");
    EXPECT_STREQ(desc.c_str(), "ZCBSIJFD:Z");
}

TEST_F(MangleSignatureTest, FormatNullAndUndefined_NewToOld)
{
    PandaString desc;

    desc = Mangle::ConvertSignature("N:");
    EXPECT_STREQ(desc.c_str(), "Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":N");
    EXPECT_STREQ(desc.c_str(), ":Lstd/core/Object;");

    desc = Mangle::ConvertSignature("U:");
    EXPECT_STREQ(desc.c_str(), "Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":U");
    EXPECT_STREQ(desc.c_str(), ":Lstd/core/Object;");
}

TEST_F(MangleSignatureTest, FormatReferences_NewToOld)
{
    PandaString desc;

    // Check 'class'
    desc = Mangle::ConvertSignature("C{std.core.Object}:");
    EXPECT_STREQ(desc.c_str(), "Lstd/core/Object;:V");
    desc = Mangle::ConvertSignature(":C{std.core.Object}");
    EXPECT_STREQ(desc.c_str(), ":Lstd/core/Object;");

    // Check 'enume'
    desc = Mangle::ConvertSignature("E{a.b.c.Color}:");
    EXPECT_STREQ(desc.c_str(), "La/b/c/Color;:V");
    desc = Mangle::ConvertSignature(":E{a.b.c.Color}");
    EXPECT_STREQ(desc.c_str(), ":La/b/c/Color;");

    // Check 'Partial<T>'
    desc = Mangle::ConvertSignature("P{a.b.c.X}:");
    EXPECT_STREQ(desc.c_str(), "La/b/c/X$partial;:V");
    desc = Mangle::ConvertSignature(":P{a.b.c.X}");
    EXPECT_STREQ(desc.c_str(), ":La/b/c/X$partial;");
}

TEST_F(MangleSignatureTest, FormatPrimitivesFixedArray_NewToOld)
{
    PandaString desc;

    // Check 'FixedArray<boolean>'
    desc = Mangle::ConvertSignature("A{z}:");
    EXPECT_STREQ(desc.c_str(), "[Z:V");
    desc = Mangle::ConvertSignature(":A{z}");
    EXPECT_STREQ(desc.c_str(), ":[Z");

    // Check 'FixedArray<char>'
    desc = Mangle::ConvertSignature("A{c}:");
    EXPECT_STREQ(desc.c_str(), "[C:V");
    desc = Mangle::ConvertSignature(":A{c}");
    EXPECT_STREQ(desc.c_str(), ":[C");

    // Check 'FixedArray<byte>'
    desc = Mangle::ConvertSignature("A{b}:");
    EXPECT_STREQ(desc.c_str(), "[B:V");
    desc = Mangle::ConvertSignature(":A{b}");
    EXPECT_STREQ(desc.c_str(), ":[B");

    // Check 'FixedArray<short>'
    desc = Mangle::ConvertSignature("A{s}:");
    EXPECT_STREQ(desc.c_str(), "[S:V");
    desc = Mangle::ConvertSignature(":A{s}");
    EXPECT_STREQ(desc.c_str(), ":[S");

    // Check 'FixedArray<int>'
    desc = Mangle::ConvertSignature("i:");
    EXPECT_STREQ(desc.c_str(), "I:V");
    desc = Mangle::ConvertSignature(":i");
    EXPECT_STREQ(desc.c_str(), ":I");

    // Check 'FixedArray<long>'
    desc = Mangle::ConvertSignature("A{l}:");
    EXPECT_STREQ(desc.c_str(), "[J:V");
    desc = Mangle::ConvertSignature(":A{l}");
    EXPECT_STREQ(desc.c_str(), ":[J");

    // Check 'FixedArray<float>'
    desc = Mangle::ConvertSignature("A{f}:");
    EXPECT_STREQ(desc.c_str(), "[F:V");
    desc = Mangle::ConvertSignature(":A{f}");
    EXPECT_STREQ(desc.c_str(), ":[F");

    // Check 'FixedArray<double>'
    desc = Mangle::ConvertSignature("A{d}:");
    EXPECT_STREQ(desc.c_str(), "[D:V");
    desc = Mangle::ConvertSignature(":A{d}");
    EXPECT_STREQ(desc.c_str(), ":[D");

    // Check mixed 'FixedArray' types
    desc = Mangle::ConvertSignature("A{z}A{c}A{b}A{s}A{i}A{l}A{f}A{d}:A{b}");
    EXPECT_STREQ(desc.c_str(), "[Z[C[B[S[I[J[F[D:[B");

    // Check nested 'FixedArray' types
    desc = Mangle::ConvertSignature("A{A{A{A{A{c}}}}}:A{b}");
    EXPECT_STREQ(desc.c_str(), "[[[[[C:[B");
}

TEST_F(MangleSignatureTest, FormatPrimitivesFixedArray_OldToOld)
{
    PandaString desc;

    // Check 'FixedArray<boolean>'
    desc = Mangle::ConvertSignature("[Z:V");
    EXPECT_STREQ(desc.c_str(), "[Z:V");
    desc = Mangle::ConvertSignature(":[Z");
    EXPECT_STREQ(desc.c_str(), ":[Z");

    // Check 'FixedArray<char>'
    desc = Mangle::ConvertSignature("[C:V");
    EXPECT_STREQ(desc.c_str(), "[C:V");
    desc = Mangle::ConvertSignature(":[C");
    EXPECT_STREQ(desc.c_str(), ":[C");

    // Check 'FixedArray<byte>'
    desc = Mangle::ConvertSignature("[B:V");
    EXPECT_STREQ(desc.c_str(), "[B:V");
    desc = Mangle::ConvertSignature(":[B");
    EXPECT_STREQ(desc.c_str(), ":[B");

    // Check 'FixedArray<short>'
    desc = Mangle::ConvertSignature("[S:V");
    EXPECT_STREQ(desc.c_str(), "[S:V");
    desc = Mangle::ConvertSignature(":[S");
    EXPECT_STREQ(desc.c_str(), ":[S");

    // Check 'FixedArray<int>'
    desc = Mangle::ConvertSignature("I:V");
    EXPECT_STREQ(desc.c_str(), "I:V");
    desc = Mangle::ConvertSignature(":I");
    EXPECT_STREQ(desc.c_str(), ":I");

    // Check 'FixedArray<long>'
    desc = Mangle::ConvertSignature("[J:V");
    EXPECT_STREQ(desc.c_str(), "[J:V");
    desc = Mangle::ConvertSignature(":[J");
    EXPECT_STREQ(desc.c_str(), ":[J");

    // Check 'FixedArray<float>'
    desc = Mangle::ConvertSignature("[F:V");
    EXPECT_STREQ(desc.c_str(), "[F:V");
    desc = Mangle::ConvertSignature(":[F");
    EXPECT_STREQ(desc.c_str(), ":[F");

    // Check 'FixedArray<double>'
    desc = Mangle::ConvertSignature("[D:V");
    EXPECT_STREQ(desc.c_str(), "[D:V");
    desc = Mangle::ConvertSignature(":[D");
    EXPECT_STREQ(desc.c_str(), ":[D");

    // Check mixed 'FixedArray' types
    desc = Mangle::ConvertSignature("[Z[C[B[S[I[J[F[D:[B");
    EXPECT_STREQ(desc.c_str(), "[Z[C[B[S[I[J[F[D:[B");

    // Check nested 'FixedArray' types
    desc = Mangle::ConvertSignature("[[[[[C:[B");
    EXPECT_STREQ(desc.c_str(), "[[[[[C:[B");
}

TEST_F(MangleSignatureTest, FormatReferencesFixedArray_NewToOld)
{
    PandaString desc;

    desc = Mangle::ConvertSignature("A{C{std.core.String}}dA{A{E{a.b.Color}}}:A{P{a.b.X}}");
    EXPECT_STREQ(desc.c_str(), "[Lstd/core/String;D[[La/b/Color;:[La/b/X$partial;");
}

TEST_F(MangleSignatureTest, FormatReferencesFixedArray_OldToOld)
{
    PandaString desc;

    desc = Mangle::ConvertSignature("[Lstd/core/String;D[[La/b/Color;:[La/b/X$partial;");
    EXPECT_STREQ(desc.c_str(), "[Lstd/core/String;D[[La/b/Color;:[La/b/X$partial;");
}

TEST_F(MangleSignatureTest, Format_Wrong)
{
    PandaString desc;

    desc = Mangle::ConvertSignature("");
    EXPECT_STREQ(desc.c_str(), "");

    desc = Mangle::ConvertSignature("b:A{A{c}");
    EXPECT_STREQ(desc.c_str(), "b:A{A{c}");

    desc = Mangle::ConvertSignature("A{A{c}}}:z");
    EXPECT_STREQ(desc.c_str(), "A{A{c}}}:z");

    desc = Mangle::ConvertSignature("A{A{c}}");
    EXPECT_STREQ(desc.c_str(), "A{A{c}}");

    desc = Mangle::ConvertSignature("C{a:b}:f");
    EXPECT_STREQ(desc.c_str(), "La:b;:F");
}

TEST_F(MangleSignatureTest, Module_FindFunction)
{
    PandaString desc;

    ani_module m {};
    ASSERT_EQ(env_->FindModule("msig", &m), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", ":", &fn), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "z:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "c:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "s:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "i:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "l:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "f:", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "d:", &fn), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "dC{msig.A}C{msig.B}:E{msig.E}", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "P{msig.A}C{escompat.Array}:", &fn), ANI_OK);
}

TEST_F(MangleSignatureTest, Module_FindFunction_OldFormat)
{
    PandaString desc;

    ani_module m {};
    ASSERT_EQ(env_->FindModule("msig", &m), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", ":V", &fn), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "Z:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "C:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "S:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "I:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "J:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "F:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "D:V", &fn), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "DLmsig/A;Lmsig/B;:Lmsig/E;", &fn), ANI_OK);
    EXPECT_EQ(env_->Module_FindFunction(m, "foo", "Lmsig/A$partial;Lescompat/Array;:V", &fn), ANI_OK);
}

TEST_F(MangleSignatureTest, Namespace_FindFunction)
{
    PandaString desc;

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("msig.rls", &ns), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", ":", &fn), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "z:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "c:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "s:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "i:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "l:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "f:", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "d:", &fn), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "dC{msig.rls.A}C{msig.rls.B}:E{msig.rls.E}", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "P{msig.A}C{escompat.Array}:", &fn), ANI_OK);
}

TEST_F(MangleSignatureTest, Namespace_FindFunction_OldFormat)
{
    PandaString desc;

    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("msig.rls", &ns), ANI_OK);

    ani_function fn {};
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", ":V", &fn), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "Z:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "C:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "S:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "I:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "J:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "F:V", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "D:V", &fn), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "DLmsig/rls/A;Lmsig/rls/B;:Lmsig/rls/E;", &fn), ANI_OK);
    EXPECT_EQ(env_->Namespace_FindFunction(ns, "foo", "Lmsig/A$partial;Lescompat/Array;:V", &fn), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindMethod)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", ":", &method), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "z:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "c:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "s:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "i:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "l:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "f:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "d:", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "dC{msig.A}C{msig.B}:E{msig.E}", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "P{msig.A}C{escompat.Array}:", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindMethod_OldFormat)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lmsig/F;", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", ":V", &method), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "Z:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "C:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "S:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "I:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "J:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "F:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "D:V", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "DLmsig/A;Lmsig/B;:Lmsig/E;", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindMethod(cls, "foo", "Lmsig/A$partial;Lescompat/Array;:V", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindStaticMethod)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.G", &cls), ANI_OK);

    ani_static_method method {};
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", ":", &method), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "z:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "c:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "s:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "i:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "l:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "f:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "d:", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "dC{msig.A}C{msig.B}:E{msig.E}", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "P{msig.A}C{escompat.Array}:", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindStaticMethod_OldFormat)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lmsig/G;", &cls), ANI_OK);

    ani_static_method method {};
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", ":V", &method), ANI_OK);

    // Check primitives
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "Z:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "C:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "S:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "I:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "J:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "F:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "D:V", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "DLmsig/A;Lmsig/B;:Lmsig/E;", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindStaticMethod(cls, "foo", "Lmsig/A$partial;Lescompat/Array;:V", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindIndexableGetter)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindIndexableGetter(cls, "d:i", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindIndexableGetter_OldFormat)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lmsig/F;", &cls), ANI_OK);

    ani_method method {};
    EXPECT_EQ(env_->Class_FindIndexableGetter(cls, "D:I", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindIndexableSetter)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method method {};
    // Check primitives
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dz:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dc:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "ds:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "di:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dl:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "df:", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dd:", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "dC{std.core.String}:", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_FindIndexableSetter_OldFormat)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lmsig/F;", &cls), ANI_OK);

    ani_method method {};

    // Check primitives
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DZ:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DC:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DS:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DI:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DJ:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DF:V", &method), ANI_OK);
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DD:V", &method), ANI_OK);

    // Check references
    EXPECT_EQ(env_->Class_FindIndexableSetter(cls, "DLstd/core/String;:V", &method), ANI_OK);
}

TEST_F(MangleSignatureTest, Class_CallStaticMethodByName)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.G", &cls), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "foo", "iii:i", &result, 1, 2U, 3U), ANI_OK);
    ASSERT_EQ(result, 1 + 2U + 3U);
}

TEST_F(MangleSignatureTest, Class_CallStaticMethodByName_OldFormat)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lmsig/G;", &cls), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Int(cls, "foo", "III:I", &result, 1, 2U, 3U), ANI_OK);
    ASSERT_EQ(result, 1 + 2U + 3U);
}

TEST_F(MangleSignatureTest, Object_CallMethodByName)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("msig.F", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);

    ani_object object {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "foo", "ii:i", &result, 1, 2U), ANI_OK);
    ASSERT_EQ(result, 1 + 2U);
}

TEST_F(MangleSignatureTest, Object_CallMethodByName_OldFormat)
{
    PandaString desc;

    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lmsig/F;", &cls), ANI_OK);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":V", &ctor), ANI_OK);

    ani_object object {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);

    ani_int result {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ASSERT_EQ(env_->Object_CallMethodByName_Int(object, "foo", "II:I", &result, 1, 2U), ANI_OK);
    ASSERT_EQ(result, 1 + 2U);
}

}  // namespace ark::ets::ani::testing
