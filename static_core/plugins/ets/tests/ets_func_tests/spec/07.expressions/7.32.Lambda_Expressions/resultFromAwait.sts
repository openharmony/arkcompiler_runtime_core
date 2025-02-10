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
name: 07.expressions/7.32.lambda_expressions/resultFromAwait
tags: []
---*/

type asyncLambda = () => Promise<boolean>

function regularFunc(): boolean {
    return true;
}

function main(): void {
    let booleanPromise: asyncLambda = async (): Promise<boolean> => {
        return Promise.resolve<boolean>(regularFunc());
    }

    let is_true: Boolean = new Boolean(true);

    let is_resolution2: Boolean = await booleanPromise();

    assertTrue( is_resolution2 instanceof Boolean)
    assertEQ(is_resolution2, is_true)
}
