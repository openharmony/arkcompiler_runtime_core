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
desc: Any type is assignable to 'Any'
---*/

let any_type_var: Any

function main(): int {
    let suite = new arktest.ArkTestsuite('All types are assignable to Any')

{% for v in values %}
    suite.addTest('{{loop.index}}', () => {
        let v = {{ v }}
        any_type_var = v
        arktest.assertEQ(v, any_type_var)
    })
{% endfor %}

    return suite.run()
}
