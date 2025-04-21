/**
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

function plusOne(x: number): number {
    return x + 1;
}

function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    let callback_js_function_ets = etsVm.getFunction('Lets_function/ETSGLOBAL;', 'callback_js_function_ets');

    // test lambda function
    let call_back_res = callback_js_function_ets((val: number) => val + 1);
    ASSERT_TRUE(call_back_res === 0x55ab);

    // test lambda function with capture
    let bias = 1;
    call_back_res = callback_js_function_ets((val: number) => val + bias);
    ASSERT_TRUE(call_back_res === 0x55ab);

    // test normal function
    call_back_res = callback_js_function_ets(plusOne);
    ASSERT_TRUE(call_back_res === 0x55ab);
}

main();
