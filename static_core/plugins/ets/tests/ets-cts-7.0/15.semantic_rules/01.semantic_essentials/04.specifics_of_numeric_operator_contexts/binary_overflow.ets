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
desc: Testing overflow for binary operators. On overflow, the type must be converted to int.
---*/

function main(): int {
    
    let testSuite = new arktest.ArkTestsuite("Binary_overflows");

    {%- for c in cases %}
        testSuite.addTest("overflow_{{c.type1}}_{{c.type2}}_{{loop.index}}", () => {
            let a: {{c.type1}} = {{c.val1}};
            let b: {{c.type2}} = {{c.val2}};
            let result = a {{c.op}} b;
    
            arktest.assertEQ(result, {{c.expected}});
            arktest.assertTrue(result instanceof int, "The type does not match the expected type");
        });
    {%- endfor %}
  
    return testSuite.run();
}
