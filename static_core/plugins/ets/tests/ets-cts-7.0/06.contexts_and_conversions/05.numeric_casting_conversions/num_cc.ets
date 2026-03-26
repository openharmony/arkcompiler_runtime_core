/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
{%- for c in cases %}

/*---
desc: A numeric casting conversion occurs if the target type and the expression type are both numeric or char.
params: from {{c.from_type}} to {{c.to_type}}
---*/

function main(): int {
    let tests = new arktest.ArkTestsuite("numeric_conversion")
{% for v in c['values'] %}
    tests.addTest("test_{{loop.index}}", () => {
        let val: {{c.from_type}} = {{v.sval}}
        let expected: {{c.to_type}} = {{v.dval}}
        let result: {{c.to_type}} = {{c.from_type|capitalize}}.to{{c.to_type|capitalize}}(val)
        arktest.assertEQ(expected, result)
    })
{% endfor %}
    return tests.run()
}
{%- endfor %}
