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
  desc: Fix semantics of ets.equals to match spec. Comparison of Char and Numeric is deprecated
  tags: []
  ---*/

function char_equals(a: char, b: char): boolean {
    return a == b
}

function char_union_equals(a: char | int, b: char | double): boolean {
    return a == b
}

function char_any_equals(a: Any, b: Any): boolean {
    return a == b
}


function main(): void {
    arktest.assertEQ(char_equals(c'a', c'a'), true)
    arktest.assertEQ(char_union_equals(1, 1.0), true)
    arktest.assertEQ(char_union_equals(c'a', c'a'), true)
    arktest.assertEQ(char_any_equals(c'a', c'a'), true)

    arktest.assertEQ(char_equals(c'a', c'b'), false)

    arktest.assertEQ(char_union_equals(c'a', 1.0), false)
    arktest.assertEQ(char_union_equals(1, c'b'), false)

    arktest.assertEQ(char_any_equals(1, c'b'), false)
    arktest.assertEQ(char_any_equals(1.0, c'b'), false)

    arktest.assertEQ(char_any_equals(c'a', c'a'.toInt()), false)
    arktest.assertEQ(char_any_equals(1.0, c'b'), false)
}