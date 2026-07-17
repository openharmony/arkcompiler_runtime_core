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
    Type of a numeric conditional expression. If the value of condition
    is unknown at compile time, then the expression type is a union of types
    whenTrue and whenFalse further normalized in accordance with
    the process discussed in Union Types Normalization.
---*/

let trueVar = true

function main(): int {
    let suite = new arktest.ArkTestsuite('Numeric types in condition expression')
{%- for c in cases %}
    suite.addTest('Explicit type: {{c.type}}', () => {
        let x: {{c.type}} = {{c.not_op}} trueVar ? {{c.whenTrue}} : {{c.whenFalse}};
        {{c.check|indent|indent}}
    })

    suite.addTest('Inferred type: {{c.type}}', () => {
        let x = {{c.not_op}} trueVar ? {{c.whenTrue}} : {{c.whenFalse}};
        {{c.check|indent|indent}}
    })
{% endfor %}

    return suite.run()
}
