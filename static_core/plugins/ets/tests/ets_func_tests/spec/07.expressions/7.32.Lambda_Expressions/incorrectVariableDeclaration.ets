/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: 07.31 Lambda Expressions
name: 07.expressions/7.32.lambda_expressions/incorrectVariableDeclaration
tags: [negative, compile-only]
---*/

type fooLambda = () => int

function foo() {
    let y: int = 1;
    let x = () => { return y+1 }; // must be - let x: fooLambda = () => { return y+1 };
    console.log(x());
}
