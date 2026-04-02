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
desc: Widening primitive types
---*/
{% for wid in widening_primitives %}
let origin{{loop.index}}: {{wid.origin}} = 0;
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Context_widdening_primitives");

    {% for wid in widening_primitives %}
        {% set outer_loop_index = loop.index %}
        testSuite.addTest("widening_primitive{{loop.index}}", () => {
            {%- for t in wid['dest'] %}
                let x{{t}}{{outer_loop_index}}: {{t}} = origin{{outer_loop_index}};
            {%- endfor %}
        }); 
    {%- endfor %}

    return testSuite.run();

}
