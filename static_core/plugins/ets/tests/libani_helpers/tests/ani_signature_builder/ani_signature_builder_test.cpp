/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "ani_gtest.h"
#include "libani_helpers/ani_signature_builder.h"

namespace ark::ets::ani_helpers::testing {

// NOLINTNEXTLINE
using namespace arkts::ani_signature;

TEST(BuilderTest, PrimitiveTypes)
{
    ASSERT_EQ(Builder::BuildUndefined().Descriptor(), "Lstd/core/Object;");
    ASSERT_EQ(Builder::BuildNull().Descriptor(), "Lstd/core/Object;");
    ASSERT_EQ(Builder::BuildBoolean().Descriptor(), "Z");
    ASSERT_EQ(Builder::BuildChar().Descriptor(), "C");
    ASSERT_EQ(Builder::BuildByte().Descriptor(), "B");
    ASSERT_EQ(Builder::BuildShort().Descriptor(), "S");
    ASSERT_EQ(Builder::BuildInt().Descriptor(), "I");
    ASSERT_EQ(Builder::BuildLong().Descriptor(), "J");
    ASSERT_EQ(Builder::BuildFloat().Descriptor(), "F");
    ASSERT_EQ(Builder::BuildDouble().Descriptor(), "D");
}

TEST(BuilderTest, StdlibClass)
{
    SignatureBuilder sb {};
    sb.AddClass({"std.core", "String"});
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "Lstd/core/String;:V");
}

TEST(BuilderTest, ArrayTest)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildArray(Builder::BuildInt()));
    sb.SetReturn(Builder::BuildArray(Builder::BuildInt()));
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "[I:[I");

    SignatureBuilder sb2 {};
    sb2.Add(Builder::BuildArray(Builder::BuildArray(Builder::BuildClass("com.example.MyClass"))));
    sb2.SetReturn(Builder::BuildArray(Builder::BuildInt()));
    ASSERT_EQ(sb2.BuildSignatureDescriptor(), "[[Lcom/example/MyClass;:[I");
}

TEST(BuilderTest, ModuleAndNamespace)
{
    Module mod = Builder::BuildModule("com.example.Module");
    ASSERT_EQ(mod.Name(), "com.example.Module");
    ASSERT_EQ(mod.Descriptor(), "Lcom/example/Module;");

    Namespace ns1 = Builder::BuildNamespace("com.example.Namespace");
    ASSERT_EQ(ns1.Name(), "com.example.Namespace");
    ASSERT_EQ(ns1.Descriptor(), "Lcom/example/Namespace;");

    Namespace ns2 = Builder::BuildNamespace({"com", "example", "SubNamespace"});
    ASSERT_EQ(ns2.Name(), "com.example.SubNamespace");
    ASSERT_EQ(ns2.Descriptor(), "Lcom/example/SubNamespace;");
}

TEST(BuilderTest, ClassEnumPartialRequired)
{
    Type cls = Builder::BuildClass("com.example.MyClass");
    ASSERT_EQ(cls.Descriptor(), "Lcom/example/MyClass;");

    Type enumType = Builder::BuildEnum({"com", "example", "MyEnum"});
    ASSERT_EQ(enumType.Descriptor(), "Lcom/example/MyEnum;");

    Type partialType = Builder::BuildPartial("com.example.MyClass");
    ASSERT_EQ(partialType.Descriptor(), "Lcom/example/MyClass$partial;");

    Type requiredType = Builder::BuildRequired({"com", "example", "RequiredClass"});
    ASSERT_EQ(requiredType.Descriptor(), "Lcom/example/RequiredClass;");
}

TEST(BuilderTest, FunctionalObject)
{
    Type funcType = Builder::BuildFunctionalObject(2, false);
    ASSERT_EQ(funcType.Descriptor(), "Lstd/core/Function2;");

    Type funcTypeReset = Builder::BuildFunctionalObject(3, true);
    ASSERT_EQ(funcTypeReset.Descriptor(), "Lstd/core/FunctionR3;");
}

TEST(BuilderTest, SignatureDescriptorDirect)
{
    std::string sig1 = Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildFloat()});
    ASSERT_EQ(sig1, "IF:V");
    std::string sig2 =
        Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildFloat()}, Builder::BuildBoolean());
    ASSERT_EQ(sig2, "IF:Z");
}

TEST(SignatureBuilderTest, BasicSignatureBuilder)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt()).Add(Builder::BuildFloat()).AddClass("com.example.MyClass");
    sb.SetReturnDouble();
    std::string sig = sb.BuildSignatureDescriptor();
    ASSERT_EQ(sig, "IFLcom/example/MyClass;:D");
}

TEST(BuilderTest, SpecialNames)
{
    ASSERT_EQ(Builder::BuildConstructorName(), "<ctor>");
    ASSERT_EQ(Builder::BuildSetterName("value"), "<set>value");
    ASSERT_EQ(Builder::BuildGetterName("value"), "<get>value");
}

TEST(SignatureBuilderTest, ComplexSignatureBuilder)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt())
        .Add(Builder::BuildFloat())
        .Add(Builder::BuildBoolean())
        .AddClass({"com", "example", "ComplexClass"})
        .AddEnum("com.example.MyEnum")
        .AddPartial("com.example.PartialClass")
        .AddRequired({"com", "example", "RequiredClass"})
        .AddFunctionalObject(4, true);
    sb.SetReturnClass("com.example.ReturnClass");

    std::string sig = sb.BuildSignatureDescriptor();
    std::string expectedSig =
        "IFZLcom/example/ComplexClass;"
        "Lcom/example/MyEnum;Lcom/example/PartialClass$partial;"
        "Lcom/example/RequiredClass;"
        "Lstd/core/FunctionR4;"
        ":Lcom/example/ReturnClass;";
    ASSERT_EQ(sig, expectedSig);
}

TEST(SignatureBuilderTest, MultipleBuilders)
{
    SignatureBuilder sb1;
    SignatureBuilder sb2;
    sb1.Add(Builder::BuildInt()).Add(Builder::BuildFloat()).SetReturnBoolean();
    sb2.Add(Builder::BuildLong()).Add(Builder::BuildDouble()).SetReturnChar();

    ASSERT_EQ(sb1.BuildSignatureDescriptor(), "IF:Z");
    ASSERT_EQ(sb2.BuildSignatureDescriptor(), "JD:C");
}

TEST(BuilderTest, SignatureDescriptorVoidReturn)
{
    std::string sig = Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildClass("com.example.Demo")});
    ASSERT_EQ(sig, "ILcom/example/Demo;:V");
}

TEST(SignatureBuilderTest, OverrideReturnType)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt()).Add(Builder::BuildBoolean());
    sb.SetReturnInt();
    sb.SetReturnChar();
    std::string sig = sb.BuildSignatureDescriptor();
    ASSERT_EQ(sig, "IZ:C");
}

TEST(SignatureBuilderTest, DefaultReturnTypeVoid)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt());
    std::string sig = sb.BuildSignatureDescriptor();
    ASSERT_EQ(sig, "I:V");
}

TEST(SignatureBuilderExtraTest, UndefinedAndNullReturn)
{
    SignatureBuilder sb1;
    sb1.Add(Builder::BuildInt()).Add(Builder::BuildFloat());
    sb1.SetReturnUndefined();
    ASSERT_EQ(sb1.BuildSignatureDescriptor(), "IF:Lstd/core/Object;");

    SignatureBuilder sb2;
    sb2.Add(Builder::BuildInt()).Add(Builder::BuildFloat());
    sb2.SetReturnNull();
    ASSERT_EQ(sb2.BuildSignatureDescriptor(), "IF:Lstd/core/Object;");
}

TEST(SignatureBuilderExtraTest, AddTypeDirectly)
{
    SignatureBuilder sb {};
    Type intType = Builder::BuildInt();
    Type floatType = Builder::BuildFloat();
    sb.Add(intType).Add(floatType);
    sb.SetReturnBoolean();
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "IF:Z");
}

TEST(SignatureBuilderExtraTest, ClassAndEnumMixed)
{
    SignatureBuilder sb {};
    sb.AddClass("com.example.ClassA")
        .AddEnum({"com", "example", "EnumB"})
        .AddPartial("com.example.PartialC")
        .AddRequired({"com", "example", "RequiredD"});
    sb.SetReturnClass("com.example.ReturnE");
    std::string expectedSig =
        "Lcom/example/ClassA;"
        "Lcom/example/EnumB;"
        "Lcom/example/PartialC$partial;"
        "Lcom/example/RequiredD;"
        ":Lcom/example/ReturnE;";
    ASSERT_EQ(sb.BuildSignatureDescriptor(), expectedSig);
}

TEST(SignatureBuilderExtraTest, SetReturnFunctionalObject)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildInt()).Add(Builder::BuildFloat());
    sb.SetReturnFunctionalObject(3, true);
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "IF:Lstd/core/FunctionR3;");
}

TEST(SignatureBuilderExtraTest, MultipleSetReturnCalls)
{
    SignatureBuilder sb {};
    sb.Add(Builder::BuildBoolean()).Add(Builder::BuildByte());
    sb.SetReturnInt();
    sb.SetReturnLong();
    ASSERT_EQ(sb.BuildSignatureDescriptor(), "ZB:J");
}

TEST(SignatureBuilderExtraTest, EmptySignature)
{
    SignatureBuilder sb {};
    ASSERT_EQ(sb.BuildSignatureDescriptor(), ":V");
}

TEST(SignatureBuilderExtraTest, SingleArgumentSignatureDescriptor)
{
    std::string sig = Builder::BuildSignatureDescriptor({Builder::BuildInt()}, Builder::BuildLong());
    ASSERT_EQ(sig, "I:J");
}

TEST(SignatureBuilderExtraTest, EmptyArgsSignatureDescriptor)
{
    std::string sig = Builder::BuildSignatureDescriptor({}, Builder::BuildNull());
    ASSERT_EQ(sig, ":Lstd/core/Object;");
}

TEST(SignatureBuilderExtraTest, OverloadConsistency)
{
    ASSERT_EQ(Builder::BuildNamespace("com.example1.example2.example3.Test").Descriptor(),
              Builder::BuildNamespace({"com", "example1", "example2", "example3", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("com.example1.example2.Test").Descriptor(),
              Builder::BuildNamespace({"com", "example1", "example2", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("com.example.Test").Descriptor(),
              Builder::BuildNamespace({"com", "example", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("com.Test").Descriptor(), Builder::BuildNamespace({"com", "Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("Test").Descriptor(), Builder::BuildNamespace({"Test"}).Descriptor());
    ASSERT_EQ(Builder::BuildNamespace("").Descriptor(), Builder::BuildNamespace({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency2)
{
    ASSERT_EQ(Builder::BuildClass("com.example1.example2.example3.TestClass").Descriptor(),
              Builder::BuildClass({"com", "example1", "example2", "example3", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("com.example1.example2.TestClass").Descriptor(),
              Builder::BuildClass({"com", "example1", "example2", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("com.example.TestClass").Descriptor(),
              Builder::BuildClass({"com", "example", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("com.TestClass").Descriptor(),
              Builder::BuildClass({"com", "TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("TestClass").Descriptor(), Builder::BuildClass({"TestClass"}).Descriptor());
    ASSERT_EQ(Builder::BuildClass("").Descriptor(), Builder::BuildClass({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency3)
{
    ASSERT_EQ(Builder::BuildEnum("com.example1.example2.example3.TestEnum").Descriptor(),
              Builder::BuildEnum({"com", "example1", "example2", "example3", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("com.example1.example2.TestEnum").Descriptor(),
              Builder::BuildEnum({"com", "example1", "example2", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("com.example.TestEnum").Descriptor(),
              Builder::BuildEnum({"com", "example", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("com.TestEnum").Descriptor(), Builder::BuildEnum({"com", "TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("TestEnum").Descriptor(), Builder::BuildEnum({"TestEnum"}).Descriptor());
    ASSERT_EQ(Builder::BuildEnum("").Descriptor(), Builder::BuildEnum({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency4)
{
    ASSERT_EQ(Builder::BuildPartial("com.example1.example2.example3.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "example1", "example2", "example3", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("com.example1.example2.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "example1", "example2", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("com.example.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "example", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("com.TestPartial").Descriptor(),
              Builder::BuildPartial({"com", "TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("TestPartial").Descriptor(), Builder::BuildPartial({"TestPartial"}).Descriptor());
    ASSERT_EQ(Builder::BuildPartial("").Descriptor(), Builder::BuildPartial({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, OverloadConsistency5)
{
    ASSERT_EQ(Builder::BuildRequired("com.example1.example2.example3.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "example1", "example2", "example3", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("com.example1.example2.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "example1", "example2", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("com.example.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "example", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("com.TestRequired").Descriptor(),
              Builder::BuildRequired({"com", "TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("TestRequired").Descriptor(),
              Builder::BuildRequired({"TestRequired"}).Descriptor());
    ASSERT_EQ(Builder::BuildRequired("").Descriptor(), Builder::BuildRequired({}).Descriptor());
}

TEST(SignatureBuilderExtraTest, AllPrimitiveTypes)
{
    SignatureBuilder sb {};
    sb.AddUndefined().AddNull().AddBoolean().AddChar().AddByte().AddShort().AddInt().AddLong().AddFloat().AddDouble();
    sb.SetReturnDouble();
    std::string expectedSig =
        "Lstd/core/Object;"
        "Lstd/core/Object;"
        "Z"
        "C"
        "B"
        "S"
        "I"
        "J"
        "F"
        "D"
        ":D";
    ASSERT_EQ(sb.BuildSignatureDescriptor(), expectedSig);
}

}  // namespace ark::ets::ani_helpers::testing