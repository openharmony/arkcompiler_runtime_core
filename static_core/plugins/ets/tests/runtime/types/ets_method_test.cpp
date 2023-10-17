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

#include "types/ets_method.h"
#include "napi/ets_scoped_objects_fix.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace panda::ets::test {

class EtsMethodTest : public testing::Test {
public:
    EtsMethodTest()
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

    ~EtsMethodTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsMethodTest);
    NO_MOVE_SEMANTIC(EtsMethodTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        PandaEtsNapiEnv *env = coroutine_->GetEtsNapiEnv();

        s_ = new napi::ScopedManagedCodeFix(env);
    }

    void TearDown() override
    {
        delete s_;
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
    napi::ScopedManagedCodeFix *s_ = nullptr;

protected:
    napi::ScopedManagedCodeFix *GetScopedManagedCodeFix()
    {
        return s_;
    }
};

TEST_F(EtsMethodTest, Invoke)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo() {
            ldai 111
            return
        }
        .function i32 Test.goo() {
            ldai 222
            return
        }
        .function i32 Test.sum() {
            call Test.foo
            sta v0
            call Test.goo
            add2 v0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo_method = klass->GetMethod("foo");
    ASSERT(foo_method);
    EtsMethod *goo_method = klass->GetMethod("goo");
    ASSERT(goo_method);
    EtsMethod *sum_method = klass->GetMethod("sum");
    ASSERT(sum_method);

    EtsValue res = foo_method->Invoke(GetScopedManagedCodeFix(), nullptr);
    ASSERT_EQ(res.GetAs<int32_t>(), 111);
    res = goo_method->Invoke(GetScopedManagedCodeFix(), nullptr);
    ASSERT_EQ(res.GetAs<int32_t>(), 222);
    res = sum_method->Invoke(GetScopedManagedCodeFix(), nullptr);
    ASSERT_EQ(res.GetAs<int32_t>(), 333);
}

TEST_F(EtsMethodTest, GetNumArgSlots)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo1() <static, access.function=public> {
            ldai 0
            return
        }
        .function i32 Test.foo2(f32 a0) <static, access.function=private> {
            ldai 0
            return
        }
        .function i32 Test.foo3(i32 a0, i32 a1, f32 a2, f32 a3, f32 a4) <static, access.function=public> {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1_method = klass->GetMethod("foo1");
    ASSERT(foo1_method);
    EtsMethod *foo2_method = klass->GetMethod("foo2");
    ASSERT(foo2_method);
    EtsMethod *foo3_method = klass->GetMethod("foo3");
    ASSERT(foo3_method);

    ASSERT_TRUE(foo1_method->IsStatic());
    ASSERT_TRUE(foo2_method->IsStatic());
    ASSERT_TRUE(foo3_method->IsStatic());
    ASSERT_TRUE(foo1_method->IsPublic());
    ASSERT_FALSE(foo2_method->IsPublic());
    ASSERT_TRUE(foo3_method->IsPublic());

    ASSERT_EQ(foo1_method->GetNumArgSlots(), 0);
    ASSERT_EQ(foo2_method->GetNumArgSlots(), 1);
    ASSERT_EQ(foo3_method->GetNumArgSlots(), 5);
}

TEST_F(EtsMethodTest, GetArgType)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo(u1 a0, i8 a1, u16 a2, i16 a3, i32 a4, i64 a5, f32 a6, f64 a7) {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo_method = klass->GetMethod("foo");
    ASSERT(foo_method);

    std::vector<EtsType> expected_arg_types = {EtsType::BOOLEAN, EtsType::BYTE, EtsType::CHAR,  EtsType::SHORT,
                                               EtsType::INT,     EtsType::LONG, EtsType::FLOAT, EtsType::DOUBLE};
    EtsType arg_type;

    for (std::size_t i = 0; i < expected_arg_types.size(); i++) {
        arg_type = foo_method->GetArgType(i);
        ASSERT_EQ(arg_type, expected_arg_types[i]);
    }
}

TEST_F(EtsMethodTest, GetReturnValueType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function u1 Test.foo0() { return }
        .function i8 Test.foo1() { return }
        .function u16 Test.foo2() { return }
        .function i16 Test.foo3() { return }
        .function i32 Test.foo4() { return }
        .function i64 Test.foo5() { return }
        .function f32 Test.foo6() { return }
        .function f64 Test.foo7() { return }
        .function TestObject Test.foo8() { return }
        .function void Test.foo9() { return }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    std::vector<EtsType> return_val_types = {EtsType::BOOLEAN, EtsType::BYTE, EtsType::CHAR,  EtsType::SHORT,
                                             EtsType::INT,     EtsType::LONG, EtsType::FLOAT, EtsType::DOUBLE,
                                             EtsType::OBJECT,  EtsType::VOID};
    std::vector<EtsMethod *> methods;
    EtsMethod *current_method = nullptr;

    for (std::size_t i = 0; i < return_val_types.size(); i++) {
        std::string foo_name("foo");
        current_method = klass->GetMethod((foo_name + std::to_string(i)).data());
        ASSERT(current_method);
        methods.push_back(current_method);
    }
    for (std::size_t i = 0; i < return_val_types.size(); i++) {
        ASSERT_EQ(methods[i]->GetReturnValueType(), return_val_types[i]);
    }
}

TEST_F(EtsMethodTest, GetMethodSignature)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function TestObject Test.foo1(i32 a0) {
            return.obj
        }
        .function i32 Test.foo2(i32 a0, f32 a1, f64 a2) {
            ldai 0
            return
        }
        .function u1 Test.foo3(i32 a0, i32 a1, f32 a2, f64 a3, f32 a4) {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1_method = klass->GetMethod("foo1");
    ASSERT(foo1_method);
    EtsMethod *foo2_method = klass->GetMethod("foo2");
    ASSERT(foo2_method);
    EtsMethod *foo3_method = klass->GetMethod("foo3");
    ASSERT(foo3_method);

    ASSERT_EQ(foo1_method->GetMethodSignature(), "I:LTestObject;");
    ASSERT_EQ(foo2_method->GetMethodSignature(), "IFD:I");
    ASSERT_EQ(foo3_method->GetMethodSignature(), "IIFDF:Z");
}

TEST_F(EtsMethodTest, GetLineNumFromBytecodeOffset)
{
    const char *source = R"(            # line 1
        .language eTS                   # line 2
        .record Test {}                 # line 3
        .function void Test.foo() {     # line 4
            mov v0, v1                  # line 5, offset 0, size 2
            mov v0, v256                # line 6, offset 2, size 5
            movi v0, 1                  # line 7, offset 7, size 2
            movi v0, 256                # line 8, offset 9, size 4
            return.void                 # line 9, offset 13, size 1
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);
    EtsMethod *foo_method = klass->GetMethod("foo");
    ASSERT(foo_method);

    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(0), 5);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(1), 5);

    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(2), 6);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(3), 6);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(4), 6);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(5), 6);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(6), 6);

    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(7), 7);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(8), 7);

    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(9), 8);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(10), 8);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(11), 8);
    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(12), 8);

    ASSERT_EQ(foo_method->GetLineNumFromBytecodeOffset(13), 9);
}

TEST_F(EtsMethodTest, GetName)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo1() {
            ldai 0
            return
        }
        .function i32 Test.foo2(f32 a0) {
            ldai 0
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1_method = klass->GetMethod("foo1");
    ASSERT(foo1_method);
    EtsMethod *foo2_method = klass->GetMethod("foo2");
    ASSERT(foo2_method);

    ASSERT_TRUE(!strcmp(foo1_method->GetName(), "foo1"));
    ASSERT_TRUE(!strcmp(foo2_method->GetName(), "foo2"));

    EtsString *str1 = foo1_method->GetNameString();
    EtsString *str2 = EtsString::CreateFromMUtf8("foo1");
    ASSERT_TRUE(str1->StringsAreEqual(reinterpret_cast<EtsObject *>(str2)));
    str1 = foo2_method->GetNameString();
    str2 = EtsString::CreateFromMUtf8("foo2");
    ASSERT_TRUE(str1->StringsAreEqual(reinterpret_cast<EtsObject *>(str2)));
}

TEST_F(EtsMethodTest, ResolveArgType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function i32 Test.foo1(TestObject a0, f32 a1) {
            return
        }
        .function i32 Test.foo2(TestObject a0, TestObject a1) {
            return
        }
        .function i32 Test.foo3(f32 a0, f32 a1) {
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1_method = klass->GetMethod("foo1");
    ASSERT(foo1_method);
    EtsMethod *foo2_method = klass->GetMethod("foo2");
    ASSERT(foo2_method);
    EtsMethod *foo3_method = klass->GetMethod("foo3");
    ASSERT(foo3_method);

    ASSERT_EQ(foo1_method->ResolveArgType(0), foo2_method->ResolveArgType(0));
    ASSERT_EQ(foo2_method->ResolveArgType(0), foo2_method->ResolveArgType(1));

    ASSERT_EQ(foo1_method->ResolveArgType(1), foo3_method->ResolveArgType(0));
    ASSERT_EQ(foo3_method->ResolveArgType(0), foo3_method->ResolveArgType(1));
}

TEST_F(EtsMethodTest, ResolveReturnType)
{
    const char *source = R"(
        .language eTS
        .record Test {}
        .record TestObject {}

        .function TestObject Test.foo1() {
            return
        }
        .function TestObject Test.foo2() {
            return
        }
        .function i32 Test.foo3() {
            return
        }
        .function i32 Test.foo4() {
            return
        }
    )";

    EtsClass *klass = GetTestClass(source, "LTest;");
    ASSERT(klass);

    EtsMethod *foo1_method = klass->GetMethod("foo1");
    ASSERT(foo1_method);
    EtsMethod *foo2_method = klass->GetMethod("foo2");
    ASSERT(foo2_method);
    EtsMethod *foo3_method = klass->GetMethod("foo3");
    ASSERT(foo3_method);
    EtsMethod *foo4_method = klass->GetMethod("foo4");
    ASSERT(foo4_method);

    ASSERT_EQ(foo1_method->ResolveReturnType(), foo2_method->ResolveReturnType());
    ASSERT_EQ(foo3_method->ResolveReturnType(), foo4_method->ResolveReturnType());
}

TEST_F(EtsMethodTest, GetClassSourceFile)
{
    const char *source = R"(
        .language eTS
        .record Test {}

        .function i32 Test.foo() {
            return
        }
    )";

    std::string source_filename = "EtsMethodTestSource.pa";

    EtsClass *klass = GetTestClass(source, "LTest;", source_filename);
    ASSERT(klass);
    EtsMethod *foo_method = klass->GetMethod("foo");
    ASSERT(foo_method);

    auto result = foo_method->GetClassSourceFile();
    ASSERT(result.data);

    ASSERT_TRUE(!strcmp(reinterpret_cast<const char *>(result.data), source_filename.data()));
}

}  // namespace panda::ets::test

// NOLINTEND(readability-magic-numbers)
