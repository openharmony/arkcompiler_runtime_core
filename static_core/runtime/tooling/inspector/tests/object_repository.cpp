/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common.h"
#include "json_object_matcher.h"
#include "object_repository.h"
#include "tooling/debugger.h"
#include "runtime_options.h"
#include "runtime.h"
#include "types/numeric_id.h"
#include "assembly-emitter.h"
#include "assembly-parser.h"

// NOLINTBEGIN

namespace panda::tooling::inspector::test {

class ObjectRepositoryTest : public testing::Test {
protected:
    void SetUp()
    {
        RuntimeOptions options;
        options.SetShouldInitializeIntrinsics(false);
        options.SetShouldLoadBootPandaFiles(false);
        Runtime::Create(options);
        thread_ = panda::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }
    void TearDown()
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }
    panda::MTManagedThread *thread_ = nullptr;
};

TEST_F(ObjectRepositoryTest, S)
{
    const char *source = R"(
        .record Test {}

        .function i32 Test.foo() {
            ldai 111
            return
        }
    )";

    pandasm::Parser p;

    auto res = p.Parse(source, "source.pa");
    ASSERT_TRUE(res.HasValue());
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT(pf);
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    class_linker->AddPandaFile(std::move(pf));

    PandaString descriptor;
    auto *ext = class_linker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("Test"), &descriptor));
    ASSERT_NE(klass, nullptr);

    auto methods = klass->GetMethods();
    ASSERT_EQ(methods.size(), 1);
    ObjectHeader *cls_object = klass->GetManagedObject();

    ObjectRepository obj;

    auto cls_obj = obj.CreateObject(TypedValue::Reference(cls_object));
    ASSERT_EQ(cls_obj.GetObjectId(), RemoteObjectId(1));
    ASSERT_THAT(ToJson(cls_obj), JsonProperties(JsonProperty<JsonObject::StringT> {"description", testing::_},
                                                JsonProperty<JsonObject::StringT> {"objectId", "1"},
                                                JsonProperty<JsonObject::StringT> {"className", testing::_},
                                                JsonProperty<JsonObject::StringT> {"type", "object"}));

    auto null_obj = obj.CreateObject(TypedValue::Reference(nullptr));
    ASSERT_THAT(ToJson(null_obj),
                JsonProperties(JsonProperty<JsonObject::StringT> {"type", "object"},
                               JsonProperty<JsonObject::StringT> {"subtype", "null"},
                               JsonProperty<JsonObject::JsonObjPointer> {"value", testing::IsNull()}));

    auto inv_obj = obj.CreateObject(TypedValue::Invalid());
    ASSERT_THAT(ToJson(inv_obj), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "undefined"}));

    auto bool_obj = obj.CreateObject(TypedValue::U1(true));
    ASSERT_THAT(ToJson(bool_obj), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "boolean"},
                                                 JsonProperty<JsonObject::BoolT> {"value", true}));

    auto num_obj = obj.CreateObject(TypedValue::U16(43602));
    ASSERT_THAT(ToJson(num_obj), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "number"},
                                                JsonProperty<JsonObject::NumT> {"value", 43602}));

    auto neg_obj = obj.CreateObject(TypedValue::I32(-2345678));
    ASSERT_THAT(ToJson(neg_obj), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "number"},
                                                JsonProperty<JsonObject::NumT> {"value", -2345678}));

    auto huge_obj = obj.CreateObject(TypedValue::I64(200000000000002));
    ASSERT_THAT(ToJson(huge_obj),
                JsonProperties(JsonProperty<JsonObject::StringT> {"type", "bigint"},
                               JsonProperty<JsonObject::StringT> {"unserializableValue", "200000000000002"}));

    auto doub_obj = obj.CreateObject(TypedValue::F64(6.547));
    ASSERT_THAT(ToJson(doub_obj), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "number"},
                                                 JsonProperty<JsonObject::NumT> {"value", testing::DoubleEq(6.547)}));

    auto glob_obj1 = obj.CreateGlobalObject();
    ASSERT_THAT(ToJson(glob_obj1), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "object"},
                                                  JsonProperty<JsonObject::StringT> {"className", "[Global]"},
                                                  JsonProperty<JsonObject::StringT> {"description", "Global object"},
                                                  JsonProperty<JsonObject::StringT> {"objectId", "0"}));

    auto glob_obj2 = obj.CreateGlobalObject();
    ASSERT_THAT(ToJson(glob_obj2), JsonProperties(JsonProperty<JsonObject::StringT> {"type", "object"},
                                                  JsonProperty<JsonObject::StringT> {"className", "[Global]"},
                                                  JsonProperty<JsonObject::StringT> {"description", "Global object"},
                                                  JsonProperty<JsonObject::StringT> {"objectId", "0"}));

    PtDebugFrame frame(&methods[0], nullptr);
    std::map<std::string, TypedValue> locals;
    locals.emplace("a", TypedValue::U16(56));
    locals.emplace("ref", TypedValue::Reference(cls_object));

    auto frame_obj = obj.CreateFrameObject(frame, locals);
    ASSERT_EQ(frame_obj.GetObjectId().value(), RemoteObjectId(2));

    auto properties = obj.GetProperties(frame_obj.GetObjectId().value(), true);
    ASSERT_EQ(properties.size(), 2);
    ASSERT_EQ(properties[0].GetName(), "a");

    ASSERT_THAT(ToJson(frame_obj), JsonProperties(JsonProperty<JsonObject::StringT> {"description", "Frame #0"},
                                                  JsonProperty<JsonObject::StringT> {"objectId", "2"},
                                                  JsonProperty<JsonObject::StringT> {"className", ""},
                                                  JsonProperty<JsonObject::StringT> {"type", "object"}));
    ASSERT_THAT(ToObject(properties[0].ToJson()),
                JsonProperties(JsonProperty<JsonObject::StringT> {"name", "a"},
                               JsonProperty<JsonObject::JsonObjPointer> {
                                   "value", testing::Pointee(
                                                JsonProperties(JsonProperty<JsonObject::NumT> {"value", 56},
                                                               JsonProperty<JsonObject::StringT> {"type", "number"}))},
                               JsonProperty<JsonObject::BoolT> {"writable", testing::_},
                               JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
                               JsonProperty<JsonObject::BoolT> {"enumerable", testing::_}));
    ASSERT_THAT(ToObject(properties[1].ToJson()),
                JsonProperties(JsonProperty<JsonObject::StringT> {"name", "ref"},
                               JsonProperty<JsonObject::JsonObjPointer> {
                                   "value", testing::Pointee(JsonProperties(
                                                JsonProperty<JsonObject::StringT> {"objectId", "1"},
                                                JsonProperty<JsonObject::StringT> {"description", testing::_},
                                                JsonProperty<JsonObject::StringT> {"className", testing::_},
                                                JsonProperty<JsonObject::StringT> {"type", "object"}))},
                               JsonProperty<JsonObject::BoolT> {"writable", testing::_},
                               JsonProperty<JsonObject::BoolT> {"configurable", testing::_},
                               JsonProperty<JsonObject::BoolT> {"enumerable", testing::_}));
}
}  // namespace panda::tooling::inspector::test

// NOLINTEND