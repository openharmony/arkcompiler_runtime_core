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

/*---
desc: >-
    BigInt relational operations with local variables.
---*/

function main(): int {
     let tests = new arktest.ArkTestsuite('BigInt relational operations with local variables')
{%- for c in cases %}
    tests.addTest(' {{c.desc}}. {{loop.index}}', () => {
        let a = {{c.b}}
        let b = {{c.operand_val}}
        arktest.assertEQ(a {{c.op}} b, {{c.expected_bool}})
    })
{% endfor %}
    return tests.run()
}
