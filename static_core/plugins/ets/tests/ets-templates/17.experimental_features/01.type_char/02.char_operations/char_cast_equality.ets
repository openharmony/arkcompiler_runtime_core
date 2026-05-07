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
    When comparing char == (b as char | undefined), the compiler emits
    checkcast std.core.Char followed by Char.equals. Passing a Char
    with the same value yields true. Passing null or non-Char numeric
    boxed types throws ClassCastError from the checkcast instruction.
---*/

function cmp(a: char, b: Any): boolean {
    return a == (b as char | undefined)
}

function expectClassCastError(fn: () => void): void {
    try {
        fn()
        arktest.failingAssertion("Expected ClassCastError but no exception was thrown")
    } catch (e) {
        arktest.assertTrue(e instanceof ClassCastError,
            "Expected ClassCastError, got " + Class.of(e).getName())
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("char_checkcast_equality");

    suite.addTest("cmp with Char same value returns true", () => {
        arktest.assertTrue(cmp(c'a', c'a'))
    });

    suite.addTest("cmp with Char different value returns false", () => {
        arktest.assertFalse(cmp(c'a', c'b'))
    });

    suite.addTest("cmp with Char boundary null char", () => {
        arktest.assertTrue(cmp(c'\u0000', c'\u0000'))
    });

    suite.addTest("cmp with Char boundary max char", () => {
        arktest.assertTrue(cmp(c'\uFFFF', c'\uFFFF'))
    });

    return suite.run();
}
