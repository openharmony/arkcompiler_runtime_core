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
desc: Qualified name
---*/
{%- for qname in qualified_name %}
class Clazz{{loop.index0}} {
    static staticField: int = 1;
    static staticMethod(): int { return 2; }

    field: int = 3;
    method(): int { return 4; }
}

let instance{{loop.index0}}: Clazz{{loop.index0}};
instance{{loop.index0}} = new Clazz{{loop.index0}}();
{%- endfor %}

function main(): int {
    
    let testSuite = new arktest.ArkTestsuite("Qualified_names");

    {%- for qname in qualified_name %}
        testSuite.addTest("qualified_names_{{loop.index0}}", () => {
            arktest.assertEQ( {{qname}} )
        });
    {%- endfor %}
  
    return testSuite.run();
}
