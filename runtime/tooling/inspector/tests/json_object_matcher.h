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

#ifndef PANDA_TOOLING_INSPECTOR_TEST_JSON_OBJECT_MATCHER_H
#define PANDA_TOOLING_INSPECTOR_TEST_JSON_OBJECT_MATCHER_H

#include "utils/json_parser.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

namespace panda::tooling::inspector::test {
template <typename PropertyType>
constexpr const char *PropertyTypeName();

template <>
constexpr const char *PropertyTypeName<JsonObject::ArrayT>()
{
    return "array";
}

template <>
constexpr const char *PropertyTypeName<JsonObject::BoolT>()
{
    return "boolean";
}

template <>
constexpr const char *PropertyTypeName<JsonObject::JsonObjPointer>()
{
    return "object";
}

template <>
constexpr const char *PropertyTypeName<JsonObject::NumT>()
{
    return "number";
}

template <>
constexpr const char *PropertyTypeName<JsonObject::StringT>()
{
    return "string";
}

template <typename PropertyType>
struct JsonProperty {
    const char *key;
    testing::Matcher<PropertyType> valueMatcher;

    template <typename OutputStream>
    void DescribeKeyTo(OutputStream *os) const
    {
        *os << "property " << std::quoted(key) << " of type " << PropertyTypeName<PropertyType>();
    }

    void DescribeTo(std::ostream *os) const
    {
        DescribeKeyTo(os);
        *os << " which ";
        valueMatcher.DescribeTo(os);
    }
};

template <typename PropertyType>
class HasJsonPropertyMatcher : public testing::MatcherInterface<const JsonObject &> {
public:
    explicit HasJsonPropertyMatcher(JsonProperty<PropertyType> property) : property_(std::move(property)) {}

    HasJsonPropertyMatcher(const char *key, testing::Matcher<PropertyType> valueMatcher) : property_ {key, valueMatcher}
    {
    }

    void DescribeTo(std::ostream *os) const override
    {
        *os << "has ";
        property_.DescribeTo(os);
    }

    void DescribeNegationTo(std::ostream *os) const override
    {
        *os << "does not have a ";
        property_.DescribeTo(os);
    }

    bool MatchAndExplain(const JsonObject &object, testing::MatchResultListener *os) const override
    {
        auto valuePtr = object.GetValue<PropertyType>(property_.key);

        if (!valuePtr) {
            *os << "no ";
            property_.DescribeKeyTo(os);
            return false;
        }

        return property_.valueMatcher.MatchAndExplain(*valuePtr, os);
    }

private:
    JsonProperty<PropertyType> property_;
};

template <typename... PropertyType>
class JsonObjectMatcher : public testing::MatcherInterface<const JsonObject &> {
public:
    explicit JsonObjectMatcher(JsonProperty<PropertyType>... property) : properties_ {property...} {}

    void DescribeTo(std::ostream *os) const override
    {
        *os << "is ";
        DescribeInternalTo(os);
    }

    void DescribeNegationTo(std::ostream *os) const override
    {
        *os << "isn't ";
        DescribeInternalTo(os);
    }

    bool MatchAndExplain(const JsonObject &object, testing::MatchResultListener *os) const override
    {
        if (object.GetSize() != sizeof...(PropertyType)) {
            *os << "number of properties doesn't match";
            return false;
        }

        return std::apply(
            [&](JsonProperty<PropertyType>... property) {
                return (HasJsonPropertyMatcher<PropertyType>(property).MatchAndExplain(object, os) && ...);
            },
            properties_);
    }

private:
    void DescribeInternalTo(std::ostream *os) const
    {
        if constexpr (sizeof...(PropertyType) == 0) {  // NOLINT(readability-braces-around-statements)
            *os << "an empty JsonObject";
        } else {  // NOLINT(readability-misleading-indentation)
            *os << "a JsonObject with properties: ";

            std::apply(
                [os](auto firstProperty, auto... property) {
                    firstProperty.DescribeTo(os);
                    ((*os << ", ", property.DescribeTo(os)), ...);
                },
                properties_);
        }
    }

    std::tuple<JsonProperty<PropertyType>...> properties_;
};

template <typename... PropertyType>
auto JsonProperties(JsonProperty<PropertyType>... property)
{
    return testing::Matcher<const JsonObject &>(new JsonObjectMatcher<PropertyType...>(property...));
}

template <typename... PropertyType>
auto JsonElements(testing::Matcher<PropertyType>... matcher)
{
    return testing::ElementsAre(
        Property(PropertyTypeName<PropertyType>(), &JsonObject::Value::Get<PropertyType>, Pointee(matcher))...);
}

template <typename PropertyType, template <typename...> class Container, typename... Param>
auto JsonElementsAreArray(const Container<testing::Matcher<PropertyType>, Param...> &container)
{
    std::vector<testing::Matcher<const JsonObject::Value &>> elements;

    std::transform(container.begin(), container.end(), std::back_inserter(elements), [](auto &matcher) {
        return Property(PropertyTypeName<PropertyType>(), &JsonObject::Value::Get<PropertyType>, Pointee(matcher));
    });

    return ElementsAreArray(elements);
}
}  // namespace panda::tooling::inspector::test

namespace panda {
inline std::ostream &operator<<(std::ostream &os, const JsonObject &object)
{
    os << '{';

    for (unsigned index = 0; index < object.GetSize(); ++index) {
        os << (index != 0 ? ", " : "") << std::quoted(object.GetKeyByIndex(index)) << ": ...";
    }

    return os << '}';
}
}  // namespace panda

#endif  // PANDA_TOOLING_INSPECTOR_TEST_JSON_OBJECT_MATCHER_H
