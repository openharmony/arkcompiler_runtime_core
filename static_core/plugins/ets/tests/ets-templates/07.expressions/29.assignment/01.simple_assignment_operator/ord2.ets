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
desc: >-
  If the left-hand operand expression is a field access expression e.f (§7.11), possibly enclosed in one or more pairs of parentheses, then:
  - First, the expression e is evaluated. If evaluation of e completes abruptly, the assignment expression completes abruptly for the same reason.
  - Next, the right hand operand is evaluated. If evaluation of the right hand expression completes abruptly, the assignment expression completes abruptly for the same reason.
  - Finally, the variable denoted by e.f is assigned the value of the right hand operand as computed above.
params: normal completion, static
---*/

let result: string = ""

class A {
    public static fld: string = ""
}

function getVal(): string {
    result = result + "1"
    return "abc"
}

function main(): int {
    result = "";  // reset due to jit mode warmup
    A.fld = "";

    let s: string = ((A.fld)) = getVal()
    if (result != "1" || s != "abc") {
        return 1
    }
    return 0
}
