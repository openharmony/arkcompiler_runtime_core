/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    If an initializer is absent in a field declaration, then the default value 
    for optional fields - undefined
---*/

enum Color { Red, Green, Blue }

{% for c in cases %}
class A{{loop.index}} {
    param?: {{c.type}}
    static sparam?: {{c.type}}
}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Optional_class_fields");

    {% for c in cases %}
        testSuite.addTest("type_{{loop.index}}", () => {
            //static optional field check
            arktest.assertEQ(A{{loop.index}}.sparam, undefined)
            let a{{loop.index}} = new A{{loop.index}}()
            //optional field check
            arktest.assertEQ(a{{loop.index}}.param, undefined)
            {% if c.type != "never" %}
            const val{{loop.index}}: {{c.type}} = {{c.value}}
            //type to supertype type|undefined assignment check
            a{{loop.index}}.param = val{{loop.index}}
            {%- endif %}
        });
    {%- endfor %}

    return testSuite.run();
}
