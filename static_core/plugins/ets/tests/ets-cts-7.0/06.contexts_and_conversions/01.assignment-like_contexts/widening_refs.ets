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
    Assignment contexts allow the use a widening reference conversion.
---*/

interface I {}
class A implements I {}
class B extends A {}
class C extends B {}

{%- for case in cases %}
{{case.defs|safe}}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Context_widening_references");

    {% for case in cases %}
        testSuite.addTest("reference_{{loop.index}}", () => {
            let src: {{case.src_type|safe}} = new {{case.src_class|safe}}();
            let dst: {{case.dst_type|safe}} = src;  // this assignment is tested
            arktest.assertEQ(dst, src)
        }); 
    {% endfor %}

    return testSuite.run();

}
