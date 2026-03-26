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
    Any conversion that is not explicitly allowed is forbidden.
---*/

{% for c in cases %}
    {{c.defs}}
{%- endfor %}

function main(): int {
    
    let testSuite = new arktest.ArkTestsuite("Forbidden_casts");

    {% for c in cases %}
        testSuite.addTest("forbidden_cast_{{loop.index}}", () => {
            let src{{loop.index}}: {{c.stype}} = {{c.sval|safe}};
            let expectedException{{loop.index}} = (e: Error): boolean => { return (e instanceof ClassCastError) };
            arktest.expectThrow(() => { let dst{{loop.index}}: {{c.dtype}} = src{{loop.index}} as {{c.dtype}}; }, expectedException{{loop.index}});
        });
    {%- endfor %}
  
    return testSuite.run();
}
