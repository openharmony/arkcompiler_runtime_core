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

#include "types/ets_object.h"

#include "ets_vm.h"
#include "ets_coroutine.h"
#include "ets_class_linker_extension.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace panda::ets::test {

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
    EtsClass *GetTestClass(std::string class_name);

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

EtsClass *EtsObjectTest::GetTestClass(std::string class_name)
{
    std::unordered_map<std::string, const char *> sources = GetSources();
    pandasm::Parser p;

    auto res = p.Parse(sources[class_name]);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    auto class_linker = Runtime::GetCurrent()->GetClassLinker();
    class_linker->AddPandaFile(std::move(pf));

    class_name.insert(0, 1, 'L');
    class_name.push_back(';');

    EtsClass *klass = coroutine_->GetPandaVM()->GetClassLinker()->GetClass(class_name.c_str());
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
    EtsClass *bar_klass = GetTestClass("Bar");
    EtsClass *foo_klass = GetTestClass("Foo");

    EtsObject *bar_obj = EtsObject::Create(bar_klass);
    EtsObject *foo_obj1 = EtsObject::Create(foo_klass);
    EtsObject *foo_obj2 = EtsObject::Create(foo_klass);

    EtsField *foo1_field = bar_klass->GetFieldIDByName("foo1");
    EtsField *foo2_field = bar_klass->GetFieldIDByName("foo2");

    bar_obj->SetFieldObject(foo1_field, foo_obj1);
    bar_obj->SetFieldObject(foo2_field, foo_obj2);
    ASSERT_EQ(bar_obj->GetFieldObject(foo1_field), foo_obj1);
    ASSERT_EQ(bar_obj->GetFieldObject(foo2_field), foo_obj2);

    EtsObject *res = bar_obj->GetAndSetFieldObject(foo2_field->GetOffset(), foo_obj1, std::memory_order_relaxed);
    ASSERT_EQ(res, foo_obj2);                                  // returned pointer was in foo2_field
    ASSERT_EQ(bar_obj->GetFieldObject(foo2_field), foo_obj1);  // now in foo2_field is pointer to Foo_obj1

    res = bar_obj->GetAndSetFieldObject(foo1_field->GetOffset(), foo_obj2, std::memory_order_relaxed);
    ASSERT_EQ(res, foo_obj1);
    ASSERT_EQ(bar_obj->GetFieldObject(foo1_field), foo_obj2);
}

TEST_F(EtsObjectTest, SetAndGetFieldObject)
{
    EtsClass *bar_klass = GetTestClass("Bar");
    EtsClass *foo_klass = GetTestClass("Foo");

    EtsObject *bar_obj = EtsObject::Create(bar_klass);
    EtsObject *foo_obj1 = EtsObject::Create(foo_klass);
    EtsObject *foo_obj2 = EtsObject::Create(foo_klass);

    EtsField *foo1_field = bar_klass->GetFieldIDByName("foo1");
    EtsField *foo2_field = bar_klass->GetFieldIDByName("foo2");

    bar_obj->SetFieldObject(foo1_field, foo_obj1);
    bar_obj->SetFieldObject(foo2_field, foo_obj2);
    ASSERT_EQ(bar_obj->GetFieldObject(foo1_field), foo_obj1);
    ASSERT_EQ(bar_obj->GetFieldObject(foo2_field), foo_obj2);

    bar_obj->SetFieldObject(foo1_field, foo_obj2);
    ASSERT_EQ(bar_obj->GetFieldObject(foo1_field), foo_obj2);
    bar_obj->SetFieldObject(foo2_field, foo_obj1);
    ASSERT_EQ(bar_obj->GetFieldObject(foo2_field), foo_obj1);
}

TEST_F(EtsObjectTest, SetAndGetFieldPrimitive)
{
    EtsClass *klass = GetTestClass("Rectangle");
    EtsObject *obj = EtsObject::Create(klass);
    EtsField *field = klass->GetFieldIDByName("Width");
    int32_t test_nmb1 = 77;
    obj->SetFieldPrimitive<int32_t>(field, test_nmb1);
    ASSERT_EQ(obj->GetFieldPrimitive<int32_t>(field), test_nmb1);

    field = klass->GetFieldIDByName("Height");
    float test_nmb2 = 111.11;
    obj->SetFieldPrimitive<float>(field, test_nmb2);
    ASSERT_EQ(obj->GetFieldPrimitive<float>(field), test_nmb2);
}

TEST_F(EtsObjectTest, CompareAndSetFieldPrimitive)
{
    EtsClass *klass = GetTestClass("Rectangle");
    EtsObject *obj = EtsObject::Create(klass);
    EtsField *field = klass->GetFieldIDByName("Width");
    int32_t fir_nmb = 134;
    int32_t sec_nmb = 12;
    obj->SetFieldPrimitive(field, fir_nmb);
    obj->CompareAndSetFieldPrimitive(field->GetOffset(), fir_nmb, sec_nmb, std::memory_order_relaxed, true);
    ASSERT_EQ(obj->GetFieldPrimitive<int32_t>(field), sec_nmb);
}

TEST_F(EtsObjectTest, CompareAndSetFieldObject)
{
    EtsClass *bar_klass = GetTestClass("Bar");
    EtsClass *foo_klass = GetTestClass("Foo");

    EtsObject *bar_obj = EtsObject::Create(bar_klass);
    EtsObject *foo_obj1 = EtsObject::Create(foo_klass);
    EtsObject *foo_obj2 = EtsObject::Create(foo_klass);

    EtsField *foo1_field = bar_klass->GetFieldIDByName("foo1");
    EtsField *foo2_field = bar_klass->GetFieldIDByName("foo2");

    bar_obj->SetFieldObject(foo1_field, foo_obj1);
    bar_obj->SetFieldObject(foo2_field, foo_obj2);
    ASSERT_EQ(bar_obj->GetFieldObject(foo1_field), foo_obj1);
    ASSERT_EQ(bar_obj->GetFieldObject(foo2_field), foo_obj2);

    ASSERT_TRUE(bar_obj->CompareAndSetFieldObject(foo1_field->GetOffset(), foo_obj1, foo_obj2,
                                                  std::memory_order_relaxed, false));
    ASSERT_EQ(bar_obj->GetFieldObject(foo1_field), foo_obj2);

    ASSERT_TRUE(bar_obj->CompareAndSetFieldObject(foo2_field->GetOffset(), foo_obj2, foo_obj1,
                                                  std::memory_order_relaxed, false));
    ASSERT_EQ(bar_obj->GetFieldObject(foo2_field), foo_obj1);
}

}  // namespace panda::ets::test

// NOLINTEND(readability-magic-numbers)
