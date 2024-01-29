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
const { etsVm, getTestModule } = require('scenarios.test.js');

const etsMod = getTestModule('scenarios_test');

let ets_sum_rest_params = etsMod.getFunction("ets_sum_rest_params");
let ets_multiply_1arg_by_sum_rest_params = etsMod.getFunction("ets_multiply_1arg_by_sum_rest_params");
let ets_multiply_sum2args_by_sum_rest_params = etsMod.getFunction("ets_multiply_sum2args_by_sum_rest_params");
let ets_concat_strings_rest_params = etsMod.getFunction("ets_concat_strings_rest_params");
let ets_method_rest_params = etsMod.getFunction("ets_call_foo_rest_params");
let F = etsMod.getClass("RestParamsTest");
{
    ASSERT_EQ(ets_sum_rest_params(1, 2, 3), (1 + 2 + 3));
    ASSERT_EQ(ets_multiply_1arg_by_sum_rest_params(1, 2, 3, 4), (1)*(2+3+4));
    ASSERT_EQ(ets_multiply_sum2args_by_sum_rest_params(1, 2, 3, 4, 5), (1+2)*(3+4+5));
    ASSERT_EQ(ets_concat_strings_rest_params(), "");
    ASSERT_EQ(ets_concat_strings_rest_params('a', 'b', 'c', 'd'), "abcd");
    ASSERT_EQ(ets_method_rest_params(new F(), new F(), new F()), 9);
    let foo = new F();
    ASSERT_EQ(foo.sum_ints(1, 2, 3), (1 + 2 + 3));
}
