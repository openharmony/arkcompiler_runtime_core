/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ets_interop_js_gtest.h"

namespace ark::ets::interop::js::testing {

class EtsInteropScenariosJsToEtsConflictTypes : public EtsInteropTest {};

// NOTE(splatov) #17937 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_array)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictArray");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsConflictTypes, Test_function_arg_type_conflict_arraybuffer)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictArraybuffer");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17379 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_boolean)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictBoolean");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17939 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_dataview)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictDataview");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17939 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_date)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictDate");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17940 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_error)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictError");
    ASSERT_EQ(ret, true);
}

// NOTE(splatov) #17939 enable this after interop is implemented in this direction
TEST_F(EtsInteropScenariosJsToEtsConflictTypes, DISABLED_Test_function_arg_type_conflict_map)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictMap");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsConflictTypes, Test_function_arg_type_conflict_object)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictObject");
    ASSERT_EQ(ret, true);
}

TEST_F(EtsInteropScenariosJsToEtsConflictTypes, Test_function_arg_type_conflict_string)
{
    auto ret = CallEtsMethod<bool>("TestFunctionArgTypeConflictString");
    ASSERT_EQ(ret, true);
}

}  // namespace ark::ets::interop::js::testing
