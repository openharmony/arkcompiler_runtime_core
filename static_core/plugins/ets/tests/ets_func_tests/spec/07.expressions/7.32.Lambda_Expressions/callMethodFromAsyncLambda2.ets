/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

/*---
desc: 07.32 Lambda Expressions
name: 07.expressions/32.lambda_expressions/callMethodFromAsyncLambda2
tags: []
---*/

type asyncLambda = () => Promise<void>

function regularFunc(): boolean {
    return true;
}

function testCallFromAsyncLambdaRegularFunc(): void {
    let is_call_async_lambda = false;
    let async_lambda: asyncLambda = async (): Promise<void> => { 
        is_call_async_lambda = regularFunc();
    }
    assertEQ(is_call_async_lambda, false)
    await async_lambda();
    assertEQ(is_call_async_lambda, true)
}

function main(): void {
    testCallFromAsyncLambdaRegularFunc();
}
