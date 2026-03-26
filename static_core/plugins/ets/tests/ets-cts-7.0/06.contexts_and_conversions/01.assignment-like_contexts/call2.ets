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
    Invocation contexts allow a boxing conversion followed by a widening reference conversion.
    Boxing conversions
---*/
{% for c in cases %}
class A{{loop.index}} {
    {%- for v in c['values'] %}
        public fld_{{loop.index}}: {{c.type}};
    {%- endfor %}

    public constructor({% for v in c['values'] %}p{{loop.index}}: {{c.type}}{{ ", " if not loop.last }}{% endfor %}) {
        {%- for v in c['values'] %}
            this.fld_{{loop.index}} = p{{loop.index}};
        {%- endfor %}
    }

    public foo({% for v in c['values'] %}p{{loop.index}}: {{c.type}}{{ ", " if not loop.last }}{% endfor %}): void {
        {%- for v in c['values'] %}
            arktest.assertNE(this.fld_{{loop.index}}, null)
        {%- endfor %}
    }
}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Alias_calls_boxed");

    {% for c in cases %}
    {% set outer_loop_index = loop.index %}
        testSuite.addTest("context_call_boxed_alias_{{loop.index}}", () => {
            {%- for v in c['values'] %}
            let v{{outer_loop_index}}_{{loop.index}}: {{c.alias_type}} = {{v|safe}};
            {%- endfor %}

            let c{{outer_loop_index}} = new A{{outer_loop_index}}({% for v in c['values'] %}v{{outer_loop_index}}_{{loop.index}}{{ ", " if not loop.last }}{% endfor %});
            c{{outer_loop_index}}.foo({% for v in c['values'] %}v{{outer_loop_index}}_{{loop.index}}{{ ", " if not loop.last }}{% endfor %});
        });
    {%- endfor %}

     return testSuite.run();
}

