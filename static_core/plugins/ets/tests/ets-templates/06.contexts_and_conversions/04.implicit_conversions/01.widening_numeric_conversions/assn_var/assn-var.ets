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
desc: Assignment context, variable, widening primitive conversion, implicit
params: from {{c.from_type}} to {{c.to_type}}
test_suffix: "{{c.from_type}}_{{c.to_type}}"
---*/
{{c.defs}}
let d: {{c.to_type}}

function test_case0(): void {
{%- for v in c['values'] %}
    let s{{loop.index}}: {{c.from_type}} = {{v.expr|safe}}
    d = s{{loop.index}}  // this var assignment is tested
    arktest.assertEQ(d, {{v.val}})
{%- endfor %}
}

function foo(): {{c.from_type}} {
    return {{c.sample_value.from}}
}

function test_case1(): void {
    let v: {{c.to_type}};
    let executed = false;
    let f = () => {
        v = foo()
        executed = true
    };
    f();
    arktest.assertTrue(executed)
    arktest.assertEQ(v, {{c.sample_value.from}})
}

function test_case2(): void {
    let d: {{c.to_type}};
    let executed = false;
    let f = () => {
        d = {{c.sample_value.from}}
        executed = true
    };
    f();
    arktest.assertTrue(executed)
    arktest.assertEQ(d, {{c.sample_value.from}})
}

function test_capture_read_and_write(): void {
    let v: {{c.to_type}} = {{c.sample_value.to_1}}
    let result: {{c.to_type}} = {{c.sample_value.to_2}}
    let f = () => {
        result = v
        v = {{c.sample_value.to_2}}
    };
    f();
    arktest.assertEQ(result, {{c.sample_value.to_1}})
    arktest.assertEQ(v, {{c.sample_value.to_2}})
}

function main(): void {
    test_case0()
    test_case1()
    test_case2()
    test_capture_read_and_write()
}
{%- endfor %}
