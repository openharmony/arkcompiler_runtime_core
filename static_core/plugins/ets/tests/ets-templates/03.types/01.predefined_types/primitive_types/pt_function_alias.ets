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
    The predefined value types are called primitive types.
    Primitive types have are aliases for the classes.
---*/

{% for p in primitives %}
function foo{{loop.index}}(p: {{p[0]}}): boolean {
    return p instanceof {{p[1]}}
}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Primitive_types_aliases_function_check");

    {% for p in primitives %}
        testSuite.addTest("alias_ftype_{{loop.index}}", () => {
            let a{{loop.index}}: {{p[1]}} = {{p[2]}}
            arktest.assertTrue(foo{{loop.index}}(a{{loop.index}}))
        });
    {%- endfor %}

    return testSuite.run();
}
