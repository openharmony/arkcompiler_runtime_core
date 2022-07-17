/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "../json_property.h"
#include "test_logger.h"

#include "utils/json_parser.h"
#include "utils/logger.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

using testing::StrEq;

namespace panda::tooling::inspector::test {
TEST(JsonPropertyTest, GetsProperty)
{
    JsonObject object(R"({
        "a": 1.2,
        "b": {"s": {"x": "foo"}}
    })");
    const JsonObject &b = **object.GetValue<JsonObject::JsonObjPointer>("b");

    double doubleValue = 0;
    EXPECT_TRUE(GetProperty<JsonObject::NumT>(doubleValue, object, "a"));
    EXPECT_DOUBLE_EQ(doubleValue, 1.2);

    int intValue = 0;
    EXPECT_TRUE(GetProperty<JsonObject::NumT>(intValue, object, "a"));
    EXPECT_EQ(intValue, 1);

    const std::string *stringValue = nullptr;
    EXPECT_FALSE(GetProperty<JsonObject::StringT>(stringValue, object, "a"));
    EXPECT_EQ(stringValue, nullptr);

    const JsonObject *objectValue = nullptr;
    EXPECT_TRUE(GetProperty<JsonObject::JsonObjPointer>(objectValue, object, "b"));
    EXPECT_EQ(objectValue, &b);

    EXPECT_TRUE(GetProperty<JsonObject::StringT>(stringValue, object, "b", "s", "x"));
    EXPECT_THAT(stringValue, Pointee(StrEq("foo")));

    EXPECT_FALSE(GetProperty<JsonObject::NumT>(intValue, object, "b", "s", "x"));
    EXPECT_EQ(intValue, 1);

    EXPECT_FALSE(GetProperty<JsonObject::NumT>(intValue, object, "c"));
    EXPECT_EQ(intValue, 1);
}

TEST(JsonPropertyTest, GetsPropertyOrLogs)
{
    TestLogger logger(TestLogger::OUTPUT_ON_FAIL);
    JsonObject object(R"({"foo": {"bar": true}})");
    bool value = false;

    EXPECT_CALL(logger, LogLineInternal).Times(0);
    EXPECT_TRUE(GetPropertyOrLog<JsonObject::BoolT>(value, object, "foo", "bar"));
    EXPECT_EQ(value, true);

    EXPECT_CALL(logger, LogLineInternal(Logger::Level::INFO, Logger::Component::DEBUGGER, "No 'baz' property"))
        .RetiresOnSaturation();
    EXPECT_FALSE(GetPropertyOrLog<JsonObject::BoolT>(value, object, "baz"));
    EXPECT_EQ(value, true);

    EXPECT_CALL(logger, LogLineInternal(Logger::Level::INFO, Logger::Component::DEBUGGER, "No 'foo.baz' property"))
        .RetiresOnSaturation();
    EXPECT_FALSE(GetPropertyOrLog<JsonObject::BoolT>(value, object, "foo", "baz"));
    EXPECT_EQ(value, true);
}
}  // namespace panda::tooling::inspector::test
