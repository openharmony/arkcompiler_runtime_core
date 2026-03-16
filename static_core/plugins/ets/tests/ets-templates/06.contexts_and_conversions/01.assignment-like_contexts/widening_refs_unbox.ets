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
    Assignment contexts allow the use a widening reference conversion followed by an unboxing conversion.
---*/
{% for case in cases %}
function foo{{loop.index}}<T extends {{case.type}}>(p: T): {{case.ptype}} {
    let src: T = p;
    let dst: {{case.ptype}} = src;
    return dst;
}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Context_widening_refs_unbox");

    {% for case in cases %}
        testSuite.addTest("unbox_{{loop.index}}", () => {
            let res: {{case.ptype}} = foo{{loop.index}}<{{case.type}}>(new {{case.type}}({{case.val|safe}}));
            arktest.assertEQ(res, {{case.res|safe}})
        }); 
    {% endfor %}

    return testSuite.run();

}
