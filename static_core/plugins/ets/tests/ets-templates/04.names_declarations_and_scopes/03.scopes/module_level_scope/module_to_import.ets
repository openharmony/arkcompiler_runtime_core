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
desc: module scope to import but not exported
files: ['./module_to_export.ets']
---*/
{%- for m in modules %}
import { {{m.entity}} } from './module_to_export'
{%- endfor %}

{%- for m in modules %}
{{m.decl}}
{%- endfor %}

function main(): int {
    
    let testSuite = new arktest.ArkTestsuite("Modile_to_import");

    {%- for m in modules %}
        testSuite.addTest("scope{{loop.index0}}", () => {
            arktest.assertEQ( {{m.use}}, {{m.expected}} )
        });
    {%- endfor %}
  
    return testSuite.run();
}
