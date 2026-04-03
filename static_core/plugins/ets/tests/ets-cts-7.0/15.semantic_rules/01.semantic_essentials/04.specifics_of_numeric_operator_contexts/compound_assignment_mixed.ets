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
desc: Compound assignment with mixed types
---*/

function main(): int {
    
    let testSuite = new arktest.ArkTestsuite("Compound_assignment_mixed");

    {%- for c in cases %}
        testSuite.addTest("assignment_{{c.description}}_{{loop.index}}", () => {
            let a: {{c.target_type}} = {{c.initial}};
            {%- if c.operand_type == "literal" %}
            a {{c.op}} {{c.operand}};
            {%- else %}
            let b: {{c.operand_type}} = {{c.operand}};
            a {{c.op}} b;
            {%- endif %}
    
            arktest.assertEQ(a, {{c.expected}});
        });
    {%- endfor %}
  
    return testSuite.run();
}
