/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    The two forms have the same semantics regardless of the number of the symbols '=' used in the operator.
    The form of the operator '===' is identical to '==', and '!==' is identical to '!='.
---*/
class A {}
class B {}
enum Color { Red, Green, Blue }

function eq2<T, U>(a: T, b: U): boolean {
    return a == b
}

function eq3<T, U>(a: T, b: U): boolean {
    return a === b
}

function ne2<T, U>(a: T, b: U): boolean {
    return a != b
}

function ne3<T, U>(a: T, b: U): boolean {
    return a !== b
}

function main(): int {
    let tests = new arktest.ArkTestsuite("EqualityExpressions");
{% for c in cases %}
    tests.addTest("EqualityExpressions_{{loop.index}}", () => {
        let a = {{c.a}}
        let b = {{c.b}}
        arktest.assertEQ(eq2(a, b), eq3(a, b))
        arktest.assertEQ(ne2(a, b), ne3(a, b))
    })
{% endfor %}
    return tests.run()
}
