/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Declaration methods distinguishable by signatures
---*/

{%- for c in cases %}
class A{{loop.index0}} {
    {{c.signature1}}: int { return 1 }
    {{c.signature2}}: int { return 2 }
}
{%- endfor %}

function main(): int {
    
    let testSuite = new arktest.ArkTestsuite("Declaration_metods");

    {%- for c in cases %}
        testSuite.addTest("methods_A{{loop.index0}}", () => {
            {{c.use}}
        });
    {%- endfor %}
  
    return testSuite.run();
}
