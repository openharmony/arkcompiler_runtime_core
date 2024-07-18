/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "types/ets_object.h"

#include "ets_vm.h"
#include "ets_coroutine.h"
#include "ets_class_linker_extension.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::test {

class EtsObjectTest : public testing::Test {
public:
    EtsObjectTest()
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

        SetClassesPandasmSources();
    }

    ~EtsObjectTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsObjectTest);
    NO_MOVE_SEMANTIC(EtsObjectTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

    void SetClassesPandasmSources();
    EtsClass *GetTestClass(std::string className);

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
    std::unordered_map<std::string, const char *> sources_;

protected:
    const std::unordered_map<std::string, const char *> &GetSources()
    {
        return sources_;
    }
};

void EtsObjectTest::SetClassesPandasmSources()
{
    const char *source = R"(
        .language eTS
        .record Rectangle {
            i32 Width
            f32 Height
            i64 Color <static>
        }
    )";
    sources_["Rectangle"] = source;

    source = R"(
        .language eTS
        .record Triangle {
            i32 firSide
            i32 secSide
            i32 thirdSide
            i64 Color <static>
        }
    )";
    sources_["Triangle"] = source;

    source = R"(
        .language eTS
        .record Foo {
            i32 member
        }
        .record Bar {
            Foo foo1
            Foo foo2
        }
    )";
    sources_["Foo"] = source;
    sources_["Bar"] = source;
}

EtsClass *EtsObjectTest::GetTestClass(std::string className)
{
    std::unordered_map<std::string, const char *> sources = GetSources();
    pandasm::Parser p;

    auto res = p.Parse(sources[className]);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));

    className.insert(0, 1, 'L');
    className.push_back(';');

    EtsClass *klass = coroutine_->GetPandaVM()->GetClassLinker()->GetClass(className.c_str());
    ASSERT(klass);
    return klass;
}

TEST_F(EtsObjectTest, GetClass)
{
    EtsClass *klass = nullptr;
    EtsObject *obj = nullptr;

    for (const auto &it : GetSources()) {
        klass = GetTestClass(it.first);
        obj = EtsObject::Create(klass);
        ASSERT_EQ(klass, obj->GetClass());
    }
}

TEST_F(EtsObjectTest, SetClass)
{
    EtsClass *klass1 = GetTestClass("Rectangle");
    EtsClass *klass2 = GetTestClass("Triangle");
    EtsObject *obj = EtsObject::Create(klass1);
    ASSERT_EQ(obj->GetClass(), klass1);
    obj->SetClass(klass2);
    ASSERT_EQ(obj->GetClass(), klass2);
}

TEST_F(EtsObjectTest, IsInstanceOf)
{
    EtsClass *klass1 = GetTestClass("Rectangle");
    EtsClass *klass2 = GetTestClass("Triangle");
    EtsObject *obj1 = EtsObject::Create(klass1);
    EtsObject *obj2 = EtsObject::Create(klass2);

    ASSERT_TRUE(obj1->IsInstanceOf(klass1));
    ASSERT_TRUE(obj2->IsInstanceOf(klass2));
    ASSERT_FALSE(obj1->IsInstanceOf(klass2));
    ASSERT_FALSE(obj2->IsInstanceOf(klass1));
}

TEST_F(EtsObjectTest, GetAndSetFieldObject)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");

    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);

    EtsField *foo1Field = barKlass->GetFieldIDByName("foo1");
    EtsField *foo2Field = barKlass->GetFieldIDByName("foo2");

    barObj->SetFieldObject(foo1Field, fooObj1);
    barObj->SetFieldObject(foo2Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj2);

    EtsObject *res = barObj->GetAndSetFieldObject(foo2Field->GetOffset(), fooObj1, std::memory_order_relaxed);
    ASSERT_EQ(res, fooObj2);                                // returned pointer was in foo2_field
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj1);  // now in foo2_field is pointer to Foo_obj1

    res = barObj->GetAndSetFieldObject(foo1Field->GetOffset(), fooObj2, std::memory_order_relaxed);
    ASSERT_EQ(res, fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj2);
}

TEST_F(EtsObjectTest, SetAndGetFieldObject)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");

    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);

    EtsField *foo1Field = barKlass->GetFieldIDByName("foo1");
    EtsField *foo2Field = barKlass->GetFieldIDByName("foo2");

    barObj->SetFieldObject(foo1Field, fooObj1);
    barObj->SetFieldObject(foo2Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj2);

    barObj->SetFieldObject(foo1Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj2);
    barObj->SetFieldObject(foo2Field, fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj1);
}

TEST_F(EtsObjectTest, SetAndGetFieldPrimitive)
{
    EtsClass *klass = GetTestClass("Rectangle");
    EtsObject *obj = EtsObject::Create(klass);
    EtsField *field = klass->GetFieldIDByName("Width");
    int32_t testNmb1 = 77;
    obj->SetFieldPrimitive<int32_t>(field, testNmb1);
    ASSERT_EQ(obj->GetFieldPrimitive<int32_t>(field), testNmb1);

    field = klass->GetFieldIDByName("Height");
    float testNmb2 = 111.11;
    obj->SetFieldPrimitive<float>(field, testNmb2);
    ASSERT_EQ(obj->GetFieldPrimitive<float>(field), testNmb2);
}

TEST_F(EtsObjectTest, CompareAndSetFieldPrimitive)
{
    EtsClass *klass = GetTestClass("Rectangle");
    EtsObject *obj = EtsObject::Create(klass);
    EtsField *field = klass->GetFieldIDByName("Width");
    int32_t firNmb = 134;
    int32_t secNmb = 12;
    obj->SetFieldPrimitive(field, firNmb);
    obj->CompareAndSetFieldPrimitive(field->GetOffset(), firNmb, secNmb, std::memory_order_relaxed, true);
    ASSERT_EQ(obj->GetFieldPrimitive<int32_t>(field), secNmb);
}

TEST_F(EtsObjectTest, CompareAndSetFieldObject)
{
    EtsClass *barKlass = GetTestClass("Bar");
    EtsClass *fooKlass = GetTestClass("Foo");

    EtsObject *barObj = EtsObject::Create(barKlass);
    EtsObject *fooObj1 = EtsObject::Create(fooKlass);
    EtsObject *fooObj2 = EtsObject::Create(fooKlass);

    EtsField *foo1Field = barKlass->GetFieldIDByName("foo1");
    EtsField *foo2Field = barKlass->GetFieldIDByName("foo2");

    barObj->SetFieldObject(foo1Field, fooObj1);
    barObj->SetFieldObject(foo2Field, fooObj2);
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj1);
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj2);

    ASSERT_TRUE(
        barObj->CompareAndSetFieldObject(foo1Field->GetOffset(), fooObj1, fooObj2, std::memory_order_relaxed, false));
    ASSERT_EQ(barObj->GetFieldObject(foo1Field), fooObj2);

    ASSERT_TRUE(
        barObj->CompareAndSetFieldObject(foo2Field->GetOffset(), fooObj2, fooObj1, std::memory_order_relaxed, false));
    ASSERT_EQ(barObj->GetFieldObject(foo2Field), fooObj1);
}

}  // namespace ark::ets::test

// NOLINTEND(readability-magic-numbers)
