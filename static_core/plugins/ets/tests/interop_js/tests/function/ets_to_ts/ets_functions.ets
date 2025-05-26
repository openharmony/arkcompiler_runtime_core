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

package ets_functions;

let callback_ets_function_js: JSValue;

function testCallBackEtsFunctionLambda(): number {
    return callback_ets_function_js((x: number) => x + 1);
}

function testCallBackEtsFunctionLambdaCapture(): number {
    let bias = 1;
    return callback_ets_function_js((x: number) => x + bias);
}

function plus_one(x: number): number {
    return x + 1;
}

function testCallBackEtsFunctionOuter(): number {
    return callback_ets_function_js(plus_one);
}

