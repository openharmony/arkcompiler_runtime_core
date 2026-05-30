/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_TESTS_ANI_VERIFY_TUPLEVALUE_TEST_UTILS_H
#define PANDA_PLUGINS_ETS_TESTS_ANI_VERIFY_TUPLEVALUE_TEST_UTILS_H

#include <type_traits>
#include <string>
#include <string_view>

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace ark::ets::ani::verify::testing {

class VerifyAniTupleValueTestBase : public VerifyAniTest {
protected:
    static constexpr ani_size LOCAL_SCOPE_CAPACITY = 2;

    ani_tuple_value GetTupleWithCheck(std::string_view moduleName, std::string_view tupleGetterName)
    {
        auto ref = CallEtsFunction<ani_ref>(moduleName.data(), tupleGetterName.data());
        ani_boolean isUndefined = ANI_TRUE;
        if (env_->Reference_IsUndefined(ref, &isUndefined) != ANI_OK) {
            ADD_FAILURE() << "Reference_IsUndefined failed";
            return nullptr;
        }
        if (isUndefined != ANI_FALSE) {
            ADD_FAILURE() << "Tuple getter returned undefined reference";
            return nullptr;
        }
        return static_cast<ani_tuple_value>(ref);
    }

    ani_tuple_value GetTupleFromExpiredLocalScope(std::string_view moduleName, std::string_view tupleGetterName)
    {
        if (env_->CreateLocalScope(LOCAL_SCOPE_CAPACITY) != ANI_OK) {
            ADD_FAILURE() << "CreateLocalScope failed";
            return nullptr;
        }
        ani_tuple_value tupleValue = GetTupleWithCheck(moduleName, tupleGetterName);
        if (env_->DestroyLocalScope() != ANI_OK) {
            ADD_FAILURE() << "DestroyLocalScope failed";
            return nullptr;
        }
        return tupleValue;
    }

    void CreateObject(ani_object *object)
    {
        ani_class cls {};
        ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
        ASSERT_EQ(env_->Object_New(cls, ctor, object), ANI_OK);
        ASSERT_NE(*object, nullptr);
    }

    void CreateString(std::string_view value, ani_string *string)
    {
        ASSERT_EQ(env_->String_NewUTF8(value.data(), value.size(), string), ANI_OK);
        ASSERT_NE(*string, nullptr);
    }

    void CreateArray(ani_size length, ani_array *array)
    {
        ani_ref undefinedRef {};
        ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
        ASSERT_EQ(env_->Array_New(length, undefinedRef, array), ANI_OK);
        ASSERT_NE(*array, nullptr);
    }

    void CreateError(ani_error *error)
    {
        ani_ref undefinedArg {};
        ASSERT_EQ(env_->GetUndefined(&undefinedArg), ANI_OK);

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("std.core.Error", &cls), ANI_OK);
        ani_method ctor {};
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{std.core.ErrorOptions}:", &ctor), ANI_OK);

        ani_object errObject {};
        ASSERT_EQ(env_->Object_New(cls, ctor, &errObject, undefinedArg, undefinedArg), ANI_OK);
        *error = static_cast<ani_error>(errObject);
        ASSERT_NE(*error, nullptr);
    }

    template <class T, class U>
    static void AssertAniValueEqual(T actual, U expected)
    {
        using ActualT = std::decay_t<T>;

        if constexpr (std::is_same_v<ActualT, ani_float>) {
            ASSERT_FLOAT_EQ(actual, static_cast<ani_float>(expected));
        } else if constexpr (std::is_same_v<ActualT, ani_double>) {
            ASSERT_DOUBLE_EQ(actual, static_cast<ani_double>(expected));
        } else {
            ASSERT_EQ(actual, static_cast<ActualT>(expected));
        }
    }

    void AssertStringRefEqual(ani_ref actual, std::string_view expected)
    {
        ASSERT_NE(actual, nullptr);
        std::string actualString;
        GetStdString(reinterpret_cast<ani_string>(actual), actualString);
        ASSERT_EQ(actualString, expected);
    }
};

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg)

#endif  // PANDA_PLUGINS_ETS_TESTS_ANI_VERIFY_TUPLEVALUE_TEST_UTILS_H
