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
    Invocation contexts allow a widening primitive conversion.
    Widening primitives conversion
---*/

{% for c in cases %}
class A{{loop.index}} {
    public fld1: {{c.ctype}};
    public fld2: {{c.ctype}};

    public constructor(p: {{c.ctype}}) {
        this.fld1 = p;
    }

    public foo(p: {{c.ctype}}): void {
        this.fld2 = p;
    }
}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Context_widening_primitive_call");

    {% for c in cases %}
        {% set outer_loop_index = loop.index %}
        testSuite.addTest("primitive_call{{loop.index}}", () => {
            {% for t in c['values'] %}
                let v{{outer_loop_index}}_{{loop.index}}: {{t.type}} = {{t.val|safe}};
                let obj{{outer_loop_index}}_{{loop.index}} = new A{{outer_loop_index}}({% if t.obj_param_trans is defined %}{{t.obj_param_trans|safe}}(v{{outer_loop_index}}_{{loop.index}}){% else %}v{{outer_loop_index}}_{{loop.index}}{% endif %});
                obj{{outer_loop_index}}_{{loop.index}}.foo({% if t.obj_param_trans is defined %}{{t.obj_param_trans|safe}}(v{{outer_loop_index}}_{{loop.index}}){% else %}v{{outer_loop_index}}_{{loop.index}}{% endif %});
                arktest.assertEQ(obj{{outer_loop_index}}_{{loop.index}}.fld1, {{t.val2|safe}})
                arktest.assertEQ(obj{{outer_loop_index}}_{{loop.index}}.fld2, {{t.val2|safe}})
            {%- endfor %}
        }); 
    {%- endfor %}

    return testSuite.run();

}
