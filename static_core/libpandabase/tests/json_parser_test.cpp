/*
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

#include <cstdio>

#include "utils/json_parser.h"
#include <gtest/gtest.h>

namespace panda::json_parser::test {
TEST(JsonParser, ParsePrimitive)
{
    auto str = R"(
    {
        "key_0" : "key_0.value",
        "key_1" : "\"key_1\"\\.\u0020value\n"
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_0"), "key_0.value");

    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_1"), "\"key_1\"\\. value\n");
}

// Issue: #14733
TEST(JsonParser, DISABLED_Arrays)
{
    auto str = R"(
    {
        "key_0" :
        [
            "elem0",
            [ "elem1.0", "elem1.1" ],
            "elem2"
        ],
        "key_1": []
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::ArrayT>("key_0"), nullptr);
    auto &main_array = *obj.GetValue<JsonObject::ArrayT>("key_0");

    // Check [0]:
    ASSERT_NE(main_array[0U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*main_array[0U].Get<JsonObject::StringT>(), "elem0");

    // Check [1]:
    ASSERT_NE(main_array[1U].Get<JsonObject::ArrayT>(), nullptr);
    auto &inner_array = *main_array[1U].Get<JsonObject::ArrayT>();

    ASSERT_NE(inner_array[0U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*inner_array[0U].Get<JsonObject::StringT>(), "elem1.0");

    ASSERT_NE(inner_array[1U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*inner_array[1U].Get<JsonObject::StringT>(), "elem1.1");

    // Check [2]:
    ASSERT_NE(main_array[2U].Get<JsonObject::StringT>(), nullptr);
    ASSERT_EQ(*main_array[2U].Get<JsonObject::StringT>(), "elem2");

    ASSERT_NE(obj.GetValue<JsonObject::ArrayT>("key_1"), nullptr);
    auto &empty_array = *obj.GetValue<JsonObject::ArrayT>("key_1");

    // Check [3]:
    ASSERT_EQ(empty_array.size(), 0U);
}

// Issue: #14733
TEST(JsonParser, DISABLED_NestedObject)
{
    auto str = R"(
    {
        "key_0"          : "key_0.value",
        "repeated_key_1" : "repeated_key_1.value0",
        "key_1" :
        {
            "key_0.0"        : "key_0.0.value",
            "repeated_key_1" : "repeated_key_1.value1",
            "repeated_key_2" : "repeated_key_2.value0"
        },
        "repeated_key_2" : "repeated_key_2.value1",
        "key_2" : {}
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    // Check key_0:
    ASSERT_NE(obj.GetValue<JsonObject::StringT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("key_0"), "key_0.value");

    // Check repeated_key_1 (in main obj):
    ASSERT_NE(obj.GetValue<JsonObject::StringT>("repeated_key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("repeated_key_1"), "repeated_key_1.value0");

    // Inner object:
    ASSERT_NE(obj.GetValue<JsonObject::JsonObjPointer>("key_1"), nullptr);
    const auto *inner_obj = obj.GetValue<JsonObject::JsonObjPointer>("key_1")->get();
    ASSERT_NE(inner_obj, nullptr);
    ASSERT_TRUE(inner_obj->IsValid());

    // Check key_0.0:
    ASSERT_NE(inner_obj->GetValue<JsonObject::StringT>("key_0.0"), nullptr);
    ASSERT_EQ(*inner_obj->GetValue<JsonObject::StringT>("key_0.0"), "key_0.0.value");

    // Check repeated_key_1:
    ASSERT_NE(inner_obj->GetValue<JsonObject::StringT>("repeated_key_1"), nullptr);
    ASSERT_EQ(*inner_obj->GetValue<JsonObject::StringT>("repeated_key_1"), "repeated_key_1.value1");

    // Check repeated_key_2:
    ASSERT_NE(inner_obj->GetValue<JsonObject::StringT>("repeated_key_2"), nullptr);
    ASSERT_EQ(*inner_obj->GetValue<JsonObject::StringT>("repeated_key_2"), "repeated_key_2.value0");

    // Check repeated_key_2 (in main obj):
    ASSERT_NE(obj.GetValue<JsonObject::StringT>("repeated_key_2"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::StringT>("repeated_key_2"), "repeated_key_2.value1");

    // Check empty inner object (key_2):
    ASSERT_NE(obj.GetValue<JsonObject::JsonObjPointer>("key_2"), nullptr);
    const auto *empty_obj = obj.GetValue<JsonObject::JsonObjPointer>("key_2")->get();
    ASSERT_NE(empty_obj, nullptr);
    ASSERT_TRUE(empty_obj->IsValid());
    ASSERT_EQ(empty_obj->GetSize(), 0U);
}

// Issue: #14733
TEST(JsonParser, DISABLED_Null)
{
    auto str = R"(
    {
        "key": null
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::JsonObjPointer>("key"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::JsonObjPointer>("key"), nullptr);
}

// Issue: #14733
TEST(JsonParser, DISABLED_Numbers)
{
    auto str = R"(
    {
        "key_0" : 0,
        "key_1" : 128,
        "key_2" : -256,
        "key_3" : .512,
        "key_4" : 1.024,
        "key_5" : -204.8
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_0"), 0U);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_1"), 128U);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_2"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_2"), -256L);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_3"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_3"), .512);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_4"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_4"), 1.024);

    ASSERT_NE(obj.GetValue<JsonObject::NumT>("key_5"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::NumT>("key_5"), -204.8);
}

TEST(JsonParser, Boolean)
{
    auto str = R"(
    {
        "key_0" : true,
        "key_1" : false
    }
    )";

    JsonObject obj(str);
    ASSERT_TRUE(obj.IsValid());

    ASSERT_NE(obj.GetValue<JsonObject::BoolT>("key_0"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::BoolT>("key_0"), true);

    ASSERT_NE(obj.GetValue<JsonObject::BoolT>("key_1"), nullptr);
    ASSERT_EQ(*obj.GetValue<JsonObject::BoolT>("key_1"), false);
}

TEST(JsonParser, InvalidJson)
{
    auto repeated_keys = R"(
    {
        "key_0" : "key_0.value0",
        "key_0" : "key_0.value1",
    }
    )";

    JsonObject obj(repeated_keys);
    ASSERT_FALSE(obj.IsValid());
}

}  // namespace panda::json_parser::test
