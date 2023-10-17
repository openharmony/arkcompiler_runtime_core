/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "get_test_class.h"
#include "ets_coroutine.h"

#include "types/ets_class.h"
#include "types/ets_method.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace panda::ets::test {

class EtsClassTest : public testing::Test {
public:
    EtsClassTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsClassTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsClassTest);
    NO_MOVE_SEMANTIC(EtsClassTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

TEST_F(EtsClassTest, GetBase)
{
    {
        const char *source = R"(
            .language eTS
            .record A {}
            .record B <ets.extends = A> {}
            .record C <ets.extends = B> {}
        )";

        EtsClass *klass_a = GetTestClass(source, "LA;");
        EtsClass *klass_b = GetTestClass(source, "LB;");
        EtsClass *klass_c = GetTestClass(source, "LC;");
        ASSERT_NE(klass_a, nullptr);
        ASSERT_NE(klass_b, nullptr);
        ASSERT_NE(klass_c, nullptr);

        ASSERT_EQ(klass_c->GetBase(), klass_b);
        ASSERT_EQ(klass_b->GetBase(), klass_a);
        ASSERT_STREQ(klass_a->GetBase()->GetDescriptor(), "Lstd/core/Object;");
    }
    {
        const char *source = R"(
            .language eTS
            .record ITest <ets.abstract, ets.interface>{}
        )";

        EtsClass *klass_itest = GetTestClass(source, "LITest;");

        ASSERT_NE(klass_itest, nullptr);
        ASSERT_TRUE(klass_itest->IsInterface());
        ASSERT_EQ(klass_itest->GetBase(), nullptr);
    }
}

TEST_F(EtsClassTest, GetStaticFieldsNumber)
{
    {
        const char *source = R"(
            .language eTS
            .record A {
                f32 x <static>
                i32 y <static>
                f32 z <static>
            }
        )";

        EtsClass *klass = GetTestClass(source, "LA;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 3);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 0);
    }
    {
        const char *source = R"(
            .language eTS
            .record B {}
        )";

        EtsClass *klass = GetTestClass(source, "LB;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 0);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 0);
    }
}

TEST_F(EtsClassTest, GetInstanceFieldsNumber)
{
    {
        const char *source = R"(
            .language eTS
            .record TestObject {}
            .record A {
                TestObject x
                TestObject y
                TestObject z
                TestObject k
            }
        )";

        EtsClass *klass = GetTestClass(source, "LA;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 0);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 4);
    }
    {
        const char *source = R"(
            .language eTS
            .record B {
                f32 x <static>
            }
        )";

        EtsClass *klass = GetTestClass(source, "LB;");
        ASSERT_NE(klass, nullptr);
        ASSERT_EQ(klass->GetStaticFieldsNumber(), 1);
        ASSERT_EQ(klass->GetInstanceFieldsNumber(), 0);
    }
}

TEST_F(EtsClassTest, GetFieldIDByName)
{
    const char *source = R"(
        .language eTS
        .record TestObject {}
        .record Test {
            TestObject x
            i32 y
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    ASSERT_EQ(klass->GetFieldIDByName("x", "F"), nullptr);
    ASSERT_NE(klass->GetFieldIDByName("x", "LTestObject;"), nullptr);
    ASSERT_EQ(klass->GetFieldIDByName("y", "Z"), nullptr);
    ASSERT_NE(klass->GetFieldIDByName("y", "I"), nullptr);

    EtsField *field_x = klass->GetFieldIDByName("x");
    EtsField *field_y = klass->GetFieldIDByName("y");
    ASSERT_NE(field_x, nullptr);
    ASSERT_NE(field_y, nullptr);
    ASSERT_FALSE(field_x->IsStatic());
    ASSERT_FALSE(field_y->IsStatic());

    ASSERT_EQ(field_x, klass->GetFieldIDByOffset(field_x->GetOffset()));
    ASSERT_EQ(field_y, klass->GetFieldIDByOffset(field_y->GetOffset()));
}

TEST_F(EtsClassTest, GetStaticFieldIDByName)
{
    const char *source = R"(
        .language eTS
        .record TestObject {}
        .record Test {
            TestObject x <static>
            i32 y <static>
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    ASSERT_EQ(klass->GetStaticFieldIDByName("x", "F"), nullptr);
    ASSERT_NE(klass->GetStaticFieldIDByName("x", "LTestObject;"), nullptr);
    ASSERT_EQ(klass->GetStaticFieldIDByName("y", "Z"), nullptr);
    ASSERT_NE(klass->GetStaticFieldIDByName("y", "I"), nullptr);

    EtsField *field_x = klass->GetStaticFieldIDByName("x");
    EtsField *field_y = klass->GetStaticFieldIDByName("y");
    ASSERT_NE(field_x, nullptr);
    ASSERT_NE(field_y, nullptr);
    ASSERT_TRUE(field_x->IsStatic());
    ASSERT_TRUE(field_y->IsStatic());

    ASSERT_EQ(field_x, klass->GetStaticFieldIDByOffset(field_x->GetOffset()));
    ASSERT_EQ(field_y, klass->GetStaticFieldIDByOffset(field_y->GetOffset()));
}

TEST_F(EtsClassTest, SetAndGetStaticFieldPrimitive)
{
    const char *source = R"(
        .language eTS
        .record Test {
            i32 x <static>
            f32 y <static>
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    EtsField *field_x = klass->GetStaticFieldIDByName("x");
    EtsField *field_y = klass->GetStaticFieldIDByName("y");
    ASSERT_NE(field_x, nullptr);
    ASSERT_NE(field_y, nullptr);

    int32_t nmb_1 = 53;
    float nmb_2 = 123.90;

    klass->SetStaticFieldPrimitive<int32_t>(field_x, nmb_1);
    klass->SetStaticFieldPrimitive<float>(field_y, nmb_2);

    ASSERT_EQ(klass->GetStaticFieldPrimitive<int32_t>(field_x), nmb_1);
    ASSERT_EQ(klass->GetStaticFieldPrimitive<float>(field_y), nmb_2);
}

TEST_F(EtsClassTest, SetAndGetStaticFieldPrimitiveByOffset)
{
    const char *source = R"(
        .language eTS
        .record Test {
            i32 x <static>
            f32 y <static>
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    std::size_t field_x_offset = klass->GetStaticFieldIDByName("x")->GetOffset();
    std::size_t field_y_offset = klass->GetStaticFieldIDByName("y")->GetOffset();

    int32_t nmb_1 = 53;
    float nmb_2 = 123.90;

    klass->SetStaticFieldPrimitive<int32_t>(field_x_offset, false, nmb_1);
    klass->SetStaticFieldPrimitive<float>(field_y_offset, false, nmb_2);

    ASSERT_EQ(klass->GetStaticFieldPrimitive<int32_t>(field_x_offset, false), nmb_1);
    ASSERT_EQ(klass->GetStaticFieldPrimitive<float>(field_y_offset, false), nmb_2);
}

TEST_F(EtsClassTest, SetAndGetFieldObject)
{
    const char *source = R"(
        .language eTS
        .record Foo {}
        .record Bar {
            Foo x <static>
            Foo y <static>
        }
    )";

    EtsClass *bar_klass = GetTestClass(source, "LBar;");
    EtsClass *foo_klass = GetTestClass(source, "LFoo;");
    ASSERT_NE(bar_klass, nullptr);
    ASSERT_NE(foo_klass, nullptr);

    EtsObject *foo_obj1 = EtsObject::Create(foo_klass);
    EtsObject *foo_obj2 = EtsObject::Create(foo_klass);
    ASSERT_NE(foo_obj1, nullptr);
    ASSERT_NE(foo_obj2, nullptr);

    EtsField *field_x = bar_klass->GetStaticFieldIDByName("x");
    EtsField *field_y = bar_klass->GetStaticFieldIDByName("y");
    ASSERT_NE(field_x, nullptr);
    ASSERT_NE(field_y, nullptr);

    bar_klass->SetStaticFieldObject(field_x, foo_obj1);
    bar_klass->SetStaticFieldObject(field_y, foo_obj2);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_x), foo_obj1);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_y), foo_obj2);

    bar_klass->SetStaticFieldObject(field_x, foo_obj2);
    bar_klass->SetStaticFieldObject(field_y, foo_obj1);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_x), foo_obj2);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_y), foo_obj1);
}

TEST_F(EtsClassTest, SetAndGetFieldObjectByOffset)
{
    const char *source = R"(
        .language eTS
        .record Foo {}
        .record Bar {
            Foo x <static>
            Foo y <static>
        }
    )";

    EtsClass *bar_klass = GetTestClass(source, "LBar;");
    EtsClass *foo_klass = GetTestClass(source, "LFoo;");
    ASSERT_NE(bar_klass, nullptr);
    ASSERT_NE(foo_klass, nullptr);

    EtsObject *foo_obj1 = EtsObject::Create(foo_klass);
    EtsObject *foo_obj2 = EtsObject::Create(foo_klass);
    ASSERT_NE(foo_obj1, nullptr);
    ASSERT_NE(foo_obj2, nullptr);

    EtsField *field_x = bar_klass->GetStaticFieldIDByName("x");
    EtsField *field_y = bar_klass->GetStaticFieldIDByName("y");
    ASSERT_NE(field_x, nullptr);
    ASSERT_NE(field_y, nullptr);

    bar_klass->SetStaticFieldObject(field_x->GetOffset(), false, foo_obj1);
    bar_klass->SetStaticFieldObject(field_y->GetOffset(), false, foo_obj2);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_x->GetOffset(), false), foo_obj1);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_y->GetOffset(), false), foo_obj2);

    bar_klass->SetStaticFieldObject(field_x, foo_obj2);
    bar_klass->SetStaticFieldObject(field_y, foo_obj1);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_x), foo_obj2);
    ASSERT_EQ(bar_klass->GetStaticFieldObject(field_y), foo_obj1);
}

TEST_F(EtsClassTest, GetDirectMethod)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record B <ets.extends = A> {}
        .record TestObject {}

        .function void A.foo1() {
            return.void
        }
        .function TestObject B.foo2(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            return
        }
    )";

    EtsClass *klass_a = GetTestClass(source, "LA;");
    EtsClass *klass_b = GetTestClass(source, "LB;");
    ASSERT_NE(klass_a, nullptr);
    ASSERT_NE(klass_b, nullptr);

    EtsMethod *method_foo1 = klass_a->GetDirectMethod("foo1", ":V");
    ASSERT_NE(method_foo1, nullptr);
    ASSERT_TRUE(!strcmp(method_foo1->GetName(), "foo1"));

    EtsMethod *method_foo2 = klass_b->GetDirectMethod("foo2", "IIFDF:LTestObject;");
    ASSERT_NE(method_foo2, nullptr);
    ASSERT_TRUE(!strcmp(method_foo2->GetName(), "foo2"));

    // GetDirectMethod can't find method from base class
    EtsMethod *method_foo1_from_klass_b = klass_b->GetDirectMethod("foo1", ":V");
    ASSERT_EQ(method_foo1_from_klass_b, nullptr);
}

TEST_F(EtsClassTest, GetMethod)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record B <ets.extends = A> {}
        .record TestObject {}

        .function void A.foo1() {
            return.void
        }
        .function TestObject B.foo2(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            return
        }
    )";

    EtsClass *klass_a = GetTestClass(source, "LA;");
    EtsClass *klass_b = GetTestClass(source, "LB;");
    ASSERT_NE(klass_a, nullptr);
    ASSERT_NE(klass_b, nullptr);

    EtsMethod *method_foo1 = klass_a->GetMethod("foo1", ":V");
    ASSERT_NE(method_foo1, nullptr);
    ASSERT_TRUE(!strcmp(method_foo1->GetName(), "foo1"));

    EtsMethod *method_foo2 = klass_b->GetMethod("foo2", "IIFDF:LTestObject;");
    ASSERT_NE(method_foo2, nullptr);
    ASSERT_TRUE(!strcmp(method_foo2->GetName(), "foo2"));

    // GetMethod can find method from base class
    EtsMethod *method_foo1_from_klass_b = klass_b->GetMethod("foo1");
    ASSERT_NE(method_foo1_from_klass_b, nullptr);
    ASSERT_TRUE(!strcmp(method_foo1_from_klass_b->GetName(), "foo1"));
}

static void TestGetPrimitiveClass(const char *primitive_name)
{
    EtsString *input_name = EtsString::CreateFromMUtf8(primitive_name);
    ASSERT_NE(input_name, nullptr);

    auto *klass = EtsClass::GetPrimitiveClass(input_name);
    ASSERT_NE(klass, nullptr);

    ASSERT_TRUE(input_name->StringsAreEqual(klass->GetName()->AsObject()));
}

TEST_F(EtsClassTest, GetPrimitiveClass)
{
    TestGetPrimitiveClass("void");
    TestGetPrimitiveClass("boolean");
    TestGetPrimitiveClass("byte");
    TestGetPrimitiveClass("char");
    TestGetPrimitiveClass("short");
    TestGetPrimitiveClass("int");
    TestGetPrimitiveClass("long");
    TestGetPrimitiveClass("float");
    TestGetPrimitiveClass("double");
}

TEST_F(EtsClassTest, SetAndGetName)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record AB {}
        .record ABC {}
    )";

    EtsClass *klass = GetTestClass(source, "LA;");
    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(EtsString::CreateFromMUtf8("A")->AsObject()));

    klass = GetTestClass(source, "LAB;");
    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(EtsString::CreateFromMUtf8("AB")->AsObject()));

    klass = GetTestClass(source, "LABC;");
    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(EtsString::CreateFromMUtf8("ABC")->AsObject()));

    EtsString *name = EtsString::CreateFromMUtf8("TestNameA");
    klass->SetName(name);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(name->AsObject()));

    name = EtsString::CreateFromMUtf8("TestNameB");
    klass->SetName(name);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(name->AsObject()));

    name = EtsString::CreateFromMUtf8("TestNameC");
    klass->SetName(name);
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(name->AsObject()));
}

TEST_F(EtsClassTest, CompareAndSetName)
{
    const char *source = R"(
        .language eTS
        .record Test {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    EtsString *old_name = EtsString::CreateFromMUtf8("TestOldName");
    EtsString *new_name = EtsString::CreateFromMUtf8("TestNewName");
    ASSERT_NE(old_name, nullptr);
    ASSERT_NE(new_name, nullptr);

    klass->SetName(old_name);

    ASSERT_TRUE(klass->CompareAndSetName(old_name, new_name));
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(new_name->AsObject()));

    ASSERT_TRUE(klass->CompareAndSetName(new_name, old_name));
    ASSERT_TRUE(klass->GetName()->StringsAreEqual(old_name->AsObject()));
}

TEST_F(EtsClassTest, IsInSamePackage)
{
    ASSERT_TRUE(EtsClass::IsInSamePackage("LA;", "LA;"));
    ASSERT_TRUE(EtsClass::IsInSamePackage("LA/B;", "LA/B;"));
    ASSERT_TRUE(EtsClass::IsInSamePackage("LA/B/C;", "LA/B/C;"));

    ASSERT_FALSE(EtsClass::IsInSamePackage("LA/B;", "LA/B/C;"));
    ASSERT_FALSE(EtsClass::IsInSamePackage("LA/B/C;", "LA/B;"));

    {
        const char *source = R"(
            .language eTS
            .record Test {}
        )";

        EtsClass *klass = GetTestClass(source, "LTest;");
        ASSERT_NE(klass, nullptr);

        EtsArray *array_1 = EtsObjectArray::Create(klass, 1000);
        EtsArray *array_2 = EtsFloatArray::Create(1000);
        ASSERT_NE(array_1, nullptr);
        ASSERT_NE(array_2, nullptr);

        EtsClass *klass_1 = array_1->GetClass();
        EtsClass *klass_2 = array_2->GetClass();
        ASSERT_NE(klass_1, nullptr);
        ASSERT_NE(klass_2, nullptr);

        ASSERT_TRUE(klass_1->IsInSamePackage(klass_2));
    }
    {
        EtsArray *array_1 = EtsFloatArray::Create(1000);
        EtsArray *array_2 = EtsIntArray::Create(1000);
        ASSERT_NE(array_1, nullptr);
        ASSERT_NE(array_2, nullptr);

        EtsClass *klass1 = array_1->GetClass();
        EtsClass *klass2 = array_2->GetClass();
        ASSERT_NE(klass1, nullptr);
        ASSERT_NE(klass2, nullptr);

        ASSERT_TRUE(klass1->IsInSamePackage(klass2));
    }
    {
        const char *source = R"(
            .language eTS
            .record Test.A {}
            .record Test.B {}
            .record Fake.A {}
        )";

        EtsClass *klass_test_a = GetTestClass(source, "LTest/A;");
        EtsClass *klass_test_b = GetTestClass(source, "LTest/B;");
        EtsClass *klass_fake_a = GetTestClass(source, "LFake/A;");
        ASSERT_NE(klass_test_a, nullptr);
        ASSERT_NE(klass_test_b, nullptr);
        ASSERT_NE(klass_fake_a, nullptr);

        ASSERT_TRUE(klass_test_a->IsInSamePackage(klass_test_a));
        ASSERT_TRUE(klass_test_b->IsInSamePackage(klass_test_b));
        ASSERT_TRUE(klass_fake_a->IsInSamePackage(klass_fake_a));

        ASSERT_TRUE(klass_test_a->IsInSamePackage(klass_test_b));
        ASSERT_TRUE(klass_test_b->IsInSamePackage(klass_test_a));

        ASSERT_FALSE(klass_test_a->IsInSamePackage(klass_fake_a));
        ASSERT_FALSE(klass_fake_a->IsInSamePackage(klass_test_a));
    }
    {
        const char *source = R"(
            .language eTS
            .record A.B.C {}
            .record A.B {}
        )";

        EtsClass *klass_abc = GetTestClass(source, "LA/B/C;");
        EtsClass *klass_ab = GetTestClass(source, "LA/B;");
        ASSERT_NE(klass_abc, nullptr);
        ASSERT_NE(klass_ab, nullptr);

        ASSERT_FALSE(klass_abc->IsInSamePackage(klass_ab));
        ASSERT_FALSE(klass_ab->IsInSamePackage(klass_abc));
    }
}

TEST_F(EtsClassTest, SetAndGetSuperClass)
{
    const char *sources = R"(
        .language eTS
        .record A {}
        .record B {}
    )";

    EtsClass *klass = GetTestClass(sources, "LA;");
    EtsClass *super_klass = GetTestClass(sources, "LB;");
    ASSERT_NE(klass, nullptr);
    ASSERT_NE(super_klass, nullptr);

    ASSERT_EQ(klass->GetSuperClass(), nullptr);
    klass->SetSuperClass(super_klass);
    ASSERT_EQ(klass->GetSuperClass(), super_klass);
}

TEST_F(EtsClassTest, IsSubClass)
{
    const char *source = R"(
        .language eTS
        .record A {}
        .record B <ets.extends = A> {}
    )";
    EtsClass *klass_a = GetTestClass(source, "LA;");
    EtsClass *klass_b = GetTestClass(source, "LB;");
    ASSERT_NE(klass_a, nullptr);
    ASSERT_NE(klass_b, nullptr);

    ASSERT_TRUE(klass_b->IsSubClass(klass_a));

    ASSERT_TRUE(klass_a->IsSubClass(klass_a));
    ASSERT_TRUE(klass_b->IsSubClass(klass_b));
}

TEST_F(EtsClassTest, SetAndGetFlags)
{
    const char *source = R"(
        .language eTS
        .record Test {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    uint32_t flags = 0U | 1U << 18U | 1U << 19U | 1U << 20U | 1U << 21U | 1U << 22U;

    klass->SetFlags(flags);
    ASSERT_EQ(klass->GetFlags(), flags);
    ASSERT_TRUE(klass->IsReference());
    ASSERT_TRUE(klass->IsSoftReference());
    ASSERT_TRUE(klass->IsWeakReference());
    ASSERT_TRUE(klass->IsFinalizerReference());
    ASSERT_TRUE(klass->IsPhantomReference());
    ASSERT_TRUE(klass->IsFinalizable());
}

TEST_F(EtsClassTest, SetAndGetComponentType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    uint32_t array_length = 100;
    auto *array_1 = EtsObjectArray::Create(klass, array_length);
    auto *array_2 = EtsObjectArray::Create(klass, array_length);

    ASSERT_NE(array_1, nullptr);
    ASSERT_NE(array_2, nullptr);
    ASSERT_EQ(array_1->GetClass()->GetComponentType(), array_2->GetClass()->GetComponentType());

    source = R"(
        .language eTS
        .record TestObject {}
    )";

    EtsClass *component_type = GetTestClass(source, "LTestObject;");
    ASSERT_NE(component_type, nullptr);

    klass->SetComponentType(component_type);
    ASSERT_EQ(klass->GetComponentType(), component_type);

    klass->SetComponentType(nullptr);
    ASSERT_EQ(klass->GetComponentType(), nullptr);
}

TEST_F(EtsClassTest, EnumerateMethods)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .function void Test.foo1() {}
        .function void Test.foo2() {}
        .function void Test.foo3() {}
        .function void Test.foo4() {}
        .function void Test.foo5() {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    std::size_t methods_vector_size = 5;
    std::vector<EtsMethod *> methods(methods_vector_size);
    std::vector<EtsMethod *> enumerate_methods(methods_vector_size);

    methods.push_back(klass->GetMethod("foo1"));
    methods.push_back(klass->GetMethod("foo2"));
    methods.push_back(klass->GetMethod("foo3"));
    methods.push_back(klass->GetMethod("foo4"));
    methods.push_back(klass->GetMethod("foo5"));

    klass->EnumerateMethods([&](EtsMethod *method) {
        enumerate_methods.push_back(method);
        return enumerate_methods.size() == methods_vector_size;
    });

    for (std::size_t i = 0; i < methods_vector_size; ++i) {
        ASSERT_EQ(methods[i], enumerate_methods[i]);
    }
}

TEST_F(EtsClassTest, EnumerateInterfaces)
{
    const char *source = R"(
        .language eTS
        .record A <ets.abstract, ets.interface> {}
        .record B <ets.abstract, ets.interface> {}
        .record C <ets.abstract, ets.interface> {}
        .record D <ets.abstract, ets.interface> {}
        .record E <ets.abstract, ets.interface> {}
        .record Test <ets.implements=A, ets.implements=B, ets.implements=C, ets.implements=D, ets.implements=E> {}
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT_NE(klass, nullptr);

    std::size_t interface_vector_size = 5;
    std::vector<EtsClass *> interfaces(interface_vector_size);
    std::vector<EtsClass *> enumerate_interfaces(interface_vector_size);

    interfaces.push_back(GetTestClass(source, "LA;"));
    interfaces.push_back(GetTestClass(source, "LB;"));
    interfaces.push_back(GetTestClass(source, "LC;"));
    interfaces.push_back(GetTestClass(source, "LD;"));
    interfaces.push_back(GetTestClass(source, "LE;"));

    klass->EnumerateInterfaces([&](EtsClass *interface) {
        enumerate_interfaces.push_back(interface);
        return enumerate_interfaces.size() == interface_vector_size;
    });

    for (std::size_t i = 0; i < interface_vector_size; ++i) {
        ASSERT_EQ(interfaces[i], enumerate_interfaces[i]);
    }
}

}  // namespace panda::ets::test

// NOLINTEND(readability-magic-numbers)
