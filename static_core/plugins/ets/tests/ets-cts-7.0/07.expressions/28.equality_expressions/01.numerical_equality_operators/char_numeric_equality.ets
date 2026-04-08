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
    If one operand is of type char and the other is of a numeric type,
    the result is true if and only if the operands have the same numeric value
    (the char Unicode code point equals the numeric value).
---*/

function main(): int {
    let suite = new arktest.ArkTestsuite("Char and numeric equality");

    suite.addTest("char 'a' (code point 97) equals int 97", () => {
        let c: char = c'a';
        let n: int = 97;
        arktest.assertTrue(c == n);
        arktest.assertTrue(n == c);
    });

    suite.addTest("char 'A' (code point 65) equals int 65", () => {
        let c: char = c'A';
        let n: int = 65;
        arktest.assertTrue(c == n);
        arktest.assertTrue(c === n);
    });

    suite.addTest("char 'a' does not equal int 98", () => {
        let c: char = c'a';
        let n: int = 98;
        arktest.assertFalse(c == n);
        arktest.assertTrue(c != n);
        arktest.assertTrue(c !== n);
    });

    suite.addTest("char 'z' (code point 122) equals double 122.0", () => {
        let c: char = c'z';
        let n: double = 122.0;
        arktest.assertTrue(c == n);
    });

    suite.addTest("char null (code point 0) equals int 0", () => {
        let c: char = c'\u0000';
        let n: int = 0;
        arktest.assertTrue(c == n);
    });

    return suite.run();
}
