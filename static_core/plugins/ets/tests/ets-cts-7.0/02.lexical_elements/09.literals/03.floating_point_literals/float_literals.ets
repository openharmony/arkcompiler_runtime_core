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
desc: Floating-point literals
---*/

function main(): int {
    let testsuite = new arktest.ArkTestsuite("Cases with correct float point literals");
    
    {% for f in float_literals -%}
    testsuite.addTest("Test literal: {{f}}", () => {

        arktest.assertTrue({{f}} instanceof float, "a float literal is not infer as a float type");

        let x{{loop.index}}: float = {{f}};
        arktest.assertEQ(x{{loop.index}}, {{f}});
    });
    {%- endfor %}

    return testsuite.run();
}
