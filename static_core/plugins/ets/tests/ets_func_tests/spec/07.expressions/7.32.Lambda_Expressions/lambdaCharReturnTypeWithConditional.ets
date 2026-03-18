/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: >-
  Lambda expression type is a function type determined by its signature.
  When the lambda body is a ternary conditional expression whose both branches
  return Char, the lambda type is () => Char and can be passed to a function
  expecting that type. Boxed Boolean is unboxed to its underlying boolean value
  when used as the condition operand.
name: 07.expressions/7.32.lambda_expressions/lambdaCharReturnTypeWithConditional
tags: []
---*/

function acceptChar(fn: () => Char): Char {
    return fn();
}

function main(): void {
    const c1: Char = Int.toChar(2);
    const c2: Char = Int.toChar(3);

    // Lambda body uses boxed Boolean in ternary; both branches return Char,
    // so the lambda correctly types as () => Char.
    // new Boolean(false) unboxes to false → c2 is selected.
    const result: Char = acceptChar(() => new Boolean(false) ? c1 : c2);
    arktest.assertEQ(result, c2);

    // new Boolean(true) unboxes to true → c1 is selected.
    const result2: Char = acceptChar(() => new Boolean(true) ? c1 : c2);
    arktest.assertEQ(result2, c1);
}
