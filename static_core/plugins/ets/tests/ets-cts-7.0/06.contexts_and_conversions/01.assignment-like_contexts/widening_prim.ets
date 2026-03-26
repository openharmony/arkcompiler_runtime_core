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
    Assignment contexts allow the use of the widening primitive conversion.
---*/

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Context_widening_primitive");

    {% for case in cases %}
        testSuite.addTest("primitive{{loop.index}}", () => {
            let orig: {{case.src_type}} = {{case.src_val|safe}};
            let src: {{case.src_type}} = orig;
            // testing this assignment
            let dst: {{case.dst_type}} = {% if case.src_to_dst is defined %}{{case.src_to_dst|safe}}{% else %}src{% endif %}; 
            arktest.assertEQ(dst, {{case.dst_val}})
        }); 
    {%- endfor %}

    return testSuite.run();

}
