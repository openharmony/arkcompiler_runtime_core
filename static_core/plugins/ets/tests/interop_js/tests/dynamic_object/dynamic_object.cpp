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
#include "ets_interop_js_gtest.h"

namespace panda::ets::interop::js::testing {

class EtsInteropJsJSValue : public EtsInteropTest {};

TEST_F(EtsInteropJsJSValue, double2jsvalue)
{
    constexpr double TEST_VALUE = 3.6;

    // Call ets method
    auto ret = CallEtsMethod<napi_value>("double2jsvalue", TEST_VALUE);
    ASSERT_TRUE(ret.has_value());

    // Get double from napi_value
    double v;
    napi_status status = napi_get_value_double(GetJsEnv(), ret.value(), &v);
    ASSERT_EQ(status, napi_ok);

    // Check result
    ASSERT_EQ(v, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, jsvalue2double)
{
    constexpr double TEST_VALUE = 53.23;

    // Create napi_value from double
    napi_value js_value;
    ASSERT_EQ(napi_ok, napi_create_double(GetJsEnv(), TEST_VALUE, &js_value));

    // Call ets method
    auto ret = CallEtsMethod<double>("jsvalue2double", js_value);

    // Check result
    ASSERT_EQ(ret, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, get_property_from_jsvalue)
{
    napi_env env = GetJsEnv();

    constexpr double TEST_VALUE = 67.78;
    // Create js object:
    //  {
    //      .prop = 67.78
    //  }
    napi_value js_obj;
    napi_value js_value;
    ASSERT_EQ(napi_ok, napi_create_object(env, &js_obj));
    ASSERT_EQ(napi_ok, napi_create_double(env, TEST_VALUE, &js_value));
    ASSERT_EQ(napi_ok, napi_set_named_property(env, js_obj, "prop", js_value));

    // Call ets method
    auto ret = CallEtsMethod<double>("get_property_from_jsvalue", js_obj);

    // Check result
    ASSERT_EQ(ret, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, get_property_from_jsvalue2)
{
    napi_env env = GetJsEnv();

    constexpr double TEST_VALUE = 674.6;
    // Create js object:
    //  {
    //      .prop_1 = {
    //          .prop_2 = 674.6
    //      }
    //  }
    napi_value js_obj {};
    napi_value js_prop_1 {};
    napi_value js_prop_2 {};
    ASSERT_EQ(napi_ok, napi_create_object(env, &js_obj));
    ASSERT_EQ(napi_ok, napi_create_object(env, &js_prop_1));
    ASSERT_EQ(napi_ok, napi_create_double(env, TEST_VALUE, &js_prop_2));
    ASSERT_EQ(napi_ok, napi_set_named_property(env, js_obj, "prop_1", js_prop_1));
    ASSERT_EQ(napi_ok, napi_set_named_property(env, js_prop_1, "prop_2", js_prop_2));

    // Call ets method
    auto ret = CallEtsMethod<double>("get_property_from_jsvalue2", js_obj);

    // Check result
    ASSERT_EQ(ret, TEST_VALUE);
}

TEST_F(EtsInteropJsJSValue, set_property_to_jsvalue)
{
    napi_env env = GetJsEnv();

    constexpr double TEST_VALUE = 54.064;
    // Create js object:
    //  {
    //  }
    napi_value js_obj {};
    ASSERT_EQ(napi_ok, napi_create_object(env, &js_obj));

    // Call ets method
    auto ret = CallEtsMethod<napi_value>("set_property_to_jsvalue", js_obj, TEST_VALUE);
    ASSERT_TRUE(ret.has_value());

    // Return js object:
    //  {
    //      .prop = 54.064
    //  }
    napi_value js_prop {};
    double val {};
    ASSERT_EQ(napi_ok, napi_get_named_property(env, js_obj, "prop", &js_prop));
    ASSERT_EQ(napi_ok, napi_get_value_double(env, js_prop, &val));

    // Check result
    ASSERT_EQ(val, TEST_VALUE);
}

}  // namespace panda::ets::interop::js::testing
