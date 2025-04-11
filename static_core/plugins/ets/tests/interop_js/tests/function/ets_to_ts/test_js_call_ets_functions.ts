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

function callbackEtsFunctionJs(callback_handler: (x: number) => number): number {
    return callback_handler(0x55aa);
}

function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    // emulate import
    let ets_global = etsVm.getClass('Lets_functions/ETSGLOBAL;');
    ets_global.callback_ets_function_js = callbackEtsFunctionJs;

    let testCallBackEtsFunctionLambda = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionLambda');
    let call_back_res = testCallBackEtsFunctionLambda();
    ASSERT_TRUE(call_back_res === 0x55ab);

    let testCallBackEtsFunctionLambdaCapture = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionLambdaCapture');
    call_back_res = testCallBackEtsFunctionLambdaCapture();
    ASSERT_TRUE(call_back_res === 0x55ab);

    let testCallBackEtsFunctionOuter = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionOuter');
    call_back_res = testCallBackEtsFunctionOuter();
    ASSERT_TRUE(call_back_res === 0x55ab);
}

main();
