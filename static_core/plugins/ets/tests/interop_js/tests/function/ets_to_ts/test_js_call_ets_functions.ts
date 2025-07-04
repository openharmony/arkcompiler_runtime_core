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

<<<<<<< HEAD
function callbackEtsFunctionJs(callback_handler: (x: number) => number): number {
    return callback_handler(0x55aa);
}

=======
function callbackEtsFunctionJs(callbackHandler: (x: number) => number): number {
    return callbackHandler(0x55aa);
}

function callbackEtsFunction(func: Function): number {
    return func(0x55aa);
}

function callbackEtsFunctionTwoParam(func: Function): number {
    return func(1, 2);
}

function callbackEtsFunctionSum(sumFunc: Function): boolean {
    ASSERT_TRUE(sumFunc() === 0);
    ASSERT_TRUE(sumFunc(1) === 1);
    ASSERT_TRUE(sumFunc(1, 2) === 3);
    ASSERT_TRUE(sumFunc(1, 2, 3) === 6);
    return true;
}

function testErrorCall(sumFunc: ()=> void, erroString: string): void {
    try {
        sumFunc();
    } catch (e: Error) {
        ASSERT_TRUE(e.toString() === erroString);
    }
}

>>>>>>> OpenHarmony_feature_20250328
function main(): void {
    let etsVm = globalThis.gtest.etsVm;

    // emulate import
    let ets_global = etsVm.getClass('Lets_functions/ETSGLOBAL;');
<<<<<<< HEAD
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
=======
    ets_global.callbackEtsFunctionJs = callbackEtsFunctionJs;

    let testCallBackEtsFunctionLambda = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionLambda');
    let callBackResult = testCallBackEtsFunctionLambda();
    ASSERT_TRUE(callBackResult === 0x55ab);

    let testCallBackEtsFunctionLambdaCapture = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionLambdaCapture');
    callBackResult = testCallBackEtsFunctionLambdaCapture();
    ASSERT_TRUE(callBackResult === 0x55ab);

    let testCallBackEtsFunctionOuter = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'testCallBackEtsFunctionOuter');
    callBackResult = testCallBackEtsFunctionOuter();
    ASSERT_TRUE(callBackResult === 0x55ab);

    let plusOne = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'plusOne');
    callBackResult = callbackEtsFunction(plusOne);
    ASSERT_TRUE(callBackResult === 0x55ab);

    let sumFunc = etsVm.getFunction('Lets_functions/ETSGLOBAL;', 'sumFunc');
    ASSERT_TRUE(callbackEtsFunctionSum(sumFunc));

    let sumFunction = etsVm.getClass('Lets_functions/ETSGLOBAL;').sumFunction;
    ASSERT_TRUE(callbackEtsFunctionSum(sumFunction));

    testErrorCall(()=> {
        callbackEtsFunctionTwoParam(plusOne);
    }, 'TypeError: no suitable method found for this number of arguments');

    testErrorCall(()=> {
        let sumClass = etsVm.getClass('Lets_functions/Sum;');
        callbackEtsFunctionSum(new sumClass());
    }, 'TypeError: is not callable');

    testErrorCall(()=> {
        callbackEtsFunctionSum(ets_global.sum);
    }, 'TypeError: is not callable');
>>>>>>> OpenHarmony_feature_20250328
}

main();
