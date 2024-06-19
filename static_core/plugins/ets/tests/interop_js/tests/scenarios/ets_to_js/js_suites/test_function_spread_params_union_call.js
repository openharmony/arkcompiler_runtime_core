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
const GCJSRuntimeCleanup = etsMod.getFunction('GCJSRuntimeCleanup');
const function_rest_params_union = etsMod.getFunction('function_rest_params_union');
const function_spread_params_union = etsMod.getFunction('function_spread_params_union');

{
  ASSERT_EQ(function_spread_params_union(), '10 abc 30');
}
{
  const arr = ['ab', 20, 'cd'];
  ASSERT_EQ(function_rest_params_union(...arr), 'ab 20 cd');
}

GCJSRuntimeCleanup();
