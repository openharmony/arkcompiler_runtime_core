/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "taihe/callback.hpp"

#include <iostream>

#include "callbackTest.impl.hpp"
#include "taihe/string.hpp"
using namespace taihe;

namespace {
void test_cb_v(callback_view<void()> f)
{
    f();
}

void test_cb_i(callback_view<void(int32_t)> f)
{
    f(1);
}

void test_cb_s(callback_view<void(string_view, bool)> f)
{
    f("hello", true);
}

string test_cb_rs(callback_view<string(string_view)> f)
{
    taihe::string out = f("hello");
    return out;
}

void test_cb_struct(callback_view<::callbackTest::Data(::callbackTest::Data const &)> f)
{
    ::callbackTest::Data result = f(::callbackTest::Data {"a", "b", 1});
    return;
}
} // namespace
// The macros used below are automatically generated code.
// NOLINTBEGIN
TH_EXPORT_CPP_API_test_cb_v(test_cb_v);
TH_EXPORT_CPP_API_test_cb_i(test_cb_i);
TH_EXPORT_CPP_API_test_cb_s(test_cb_s);
TH_EXPORT_CPP_API_test_cb_rs(test_cb_rs);
TH_EXPORT_CPP_API_test_cb_struct(test_cb_struct);
// NOLINTEND
