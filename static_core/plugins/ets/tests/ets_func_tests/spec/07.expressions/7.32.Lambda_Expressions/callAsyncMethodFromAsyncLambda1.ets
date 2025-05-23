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
name: 07.expressions/7.32.lambda_expressions/callAsyncMethodFromAsyncLambda1
tags: []
---*/

type asyncLambda = () => Promise<void>

class LambdaTests {
    async asyncFunc(): Promise<boolean> {
        return true;
    }

    testCallAsyncLambda(): void {
        let call_async_lambda = false;

        let async_lambda: asyncLambda = async (): Promise<void> => { 
            await this.asyncFunc();
            call_async_lambda = true;
        }

        assertEQ(call_async_lambda, false)
        await async_lambda();
        assertEQ(call_async_lambda, true)
    }
}

function main(): void {
    new LambdaTests().testCallAsyncLambda();
}
