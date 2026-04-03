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
desc: Type of each argument corresponding to the rest parameter without the spread operator
      must be compatible with the element type of the array rest type parameter.
---*/
{% for c in cases %}
{{ c.definition }}
{%- endfor %}

function main(): int {
    
    let testSuite = new arktest.ArkTestsuite("Rest_parameter_with_optional");

    {% for c in cases %}
        testSuite.addTest("case_{{loop.index}}", () => {
            {%- for assertion in c.assertions%}
            arktest.assertEQ({{ assertion.call }}, {{ assertion.result }} )
            {%- endfor %}
        }); 
    {%- endfor %}

    return testSuite.run();
}
