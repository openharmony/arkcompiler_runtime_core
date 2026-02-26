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
desc: Casting conversion implicitly in the following arithmetic operations - Prefix Increment
---*/

function main(): int {
    let tests = new arktest.ArkTestsuite("prefix_conversion")
{% for t in types %}
    tests.addTest("test_{{loop.index}}", () => {
        let p: {{t.ptype}} = {{t.expr|safe}};
        let r: {{t.ptype}} = (() => {{t.prefix}}p)();
        arktest.assertEQ(p, {{t.result}})
        arktest.assertEQ(r, {{t.result}})
    })
{% endfor %}
    return tests.run()
}