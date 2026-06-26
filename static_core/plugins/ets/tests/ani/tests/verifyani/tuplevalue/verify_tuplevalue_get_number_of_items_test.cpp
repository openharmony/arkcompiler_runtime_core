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

#include "plugins/ets/tests/ani/tests/verifyani/tuplevalue/verify_tuplevalue_test_utils.h"

// NOLINTBEGIN(readability-magic-numbers)

namespace ark::ets::ani::verify::testing {

class TupleValueGetNumberOfItemsTest : public VerifyAniTupleValueTestBase {
protected:
    static constexpr std::string_view MODULE_NAME = "verify_tuplevalue_get_number_of_items_test";

    ani_tuple_value GetEmptyTuple()
    {
        return GetTupleWithCheck(MODULE_NAME, "getEmptyTuple");
    }

    ani_tuple_value GetPrimitiveTuple()
    {
        return GetTupleWithCheck(MODULE_NAME, "getPrimitiveTuple");
    }
};

TEST_F(TupleValueGetNumberOfItemsTest, wrong_env)
{
    ani_tuple_value tupleValue = GetPrimitiveTuple();
    ani_size result = 0U;

    ASSERT_EQ(env_->c_api->TupleValue_GetNumberOfItems(nullptr, tupleValue, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"tuple_value", "ani_tuple_value"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, wrong_tuple)
{
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(nullptr, &result), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "reference is nullptr [ERROR]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, expired_local_scope_tuple)
{
    ani_tuple_value tupleValue = GetTupleFromExpiredLocalScope(MODULE_NAME, "getPrimitiveTuple");
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tupleValue, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "reference not found (may be deleted, out of scope, or corrupted) [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, null_input_tuple)
{
    ani_tuple_value tupleValue {};
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&tupleValue)), ANI_OK);
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tupleValue, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "wrong reference type: null [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, undefined_input_tuple)
{
    ani_tuple_value tupleValue {};
    ASSERT_EQ(env_->GetUndefined(reinterpret_cast<ani_ref *>(&tupleValue)), ANI_OK);
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tupleValue, &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "wrong reference type: undefined [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, object_input_tuple)
{
    ani_object object {};
    CreateObject(&object);
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(reinterpret_cast<ani_tuple_value>(object), &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "wrong reference type: ani_object [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, string_input_tuple)
{
    ani_string string {};
    CreateString("tuple-value", &string);
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(reinterpret_cast<ani_tuple_value>(string), &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "wrong reference type: ani_string [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, error_input_tuple)
{
    ani_error aniError {};
    CreateError(&aniError);
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(reinterpret_cast<ani_tuple_value>(aniError), &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "wrong reference type: ani_error [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, array_input_tuple)
{
    ani_array array {};
    CreateArray(3U, &array);
    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(reinterpret_cast<ani_tuple_value>(array), &result), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value", "wrong reference type: ani_array [FATAL]"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, wrong_result)
{
    ani_tuple_value tupleValue = GetPrimitiveTuple();

    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tupleValue, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"tuple_value", "ani_tuple_value"},
        {"result", "ani_size *", "nullptr for storing 'ani_size' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->TupleValue_GetNumberOfItems(nullptr, nullptr, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "env is nullptr [ERROR]"},
        {"tuple_value", "ani_tuple_value", "reference is nullptr [ERROR]"},
        {"result", "ani_size *", "nullptr for storing 'ani_size' [ERROR]"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, throw_error)
{
    ani_tuple_value tupleValue = GetPrimitiveTuple();
    ThrowError();

    ani_size result = 0U;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tupleValue, &result), ANI_PENDING_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has a pending exception [ERROR]"},
        {"tuple_value", "ani_tuple_value"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("TupleValue_GetNumberOfItems", testLines);
}

TEST_F(TupleValueGetNumberOfItemsTest, success_empty_tuple)
{
    ani_tuple_value tupleValue = GetEmptyTuple();
    ani_size result = 1U;

    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tupleValue, &result), ANI_OK);
    ASSERT_EQ(result, 0U);
}

TEST_F(TupleValueGetNumberOfItemsTest, success_non_empty_tuple)
{
    ani_tuple_value tupleValue = GetPrimitiveTuple();
    ani_size result = 0U;

    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tupleValue, &result), ANI_OK);
    ASSERT_EQ(result, 8U);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(readability-magic-numbers)
