/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: Readonly array type instantiation and access - {{c.type}}
---*/

function main(): int {
    let testsuite = new arktest.ArkTestsuite("Readonly array type instantiation and access - {{c.type}}");

    let y1: ReadonlyArray<{{c.type}}> = {{c.value}}
    testsuite.addTest("Test ReadonlyArray<T> syntax", () => {
        arktest.assertTrue(y1 instanceof ReadonlyArray)
        arktest.assertEQ(y1[0], {{c.expected_value}})
    });

    let y2: readonly {{c.type}}[] = {{c.value}}
    testsuite.addTest("Test readonly T[] syntax", () => {
        arktest.assertTrue(y2 instanceof ReadonlyArray)
        arktest.assertEQ(y2[0], {{c.expected_value}})
    });
    
    testsuite.addTest("Verify both syntaxes produce equivalent results", () => {
        arktest.assertEQ(y1.length, y2.length)
        for (let i = 0; i < y1.length; i++) {
            arktest.assertEQ(y1[i], y2[i])
        }
    });
    
    return testsuite.run();
}

{% endfor %}
