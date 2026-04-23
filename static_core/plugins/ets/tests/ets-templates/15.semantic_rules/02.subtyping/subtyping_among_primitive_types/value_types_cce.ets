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
{% yaml "value_types.params.yaml" as data %}
/*---
desc: >-
    Value type is a subtype of another value type if both are of the same type
---*/

let isCCE = (e: Error) => e instanceof ClassCastError
let src: Any, dst: Any

function main(): int {
    let suite = new arktest.ArkTestsuite("Check subtyping among value types")
{% for c1 in data.cases %}
{% for c2 in data.cases %}
{% if c1.type != c2.type %}
    suite.addTest("{{c1.type}} is not subtype of {{c2.type}}", () => {
        let v: {{c1.type}} = {{c1.val}};
        (() => { src = v })();
        arktest.expectThrow(() => { dst = src as {{c2.type}} }, isCCE);
    })
{%- endif %}
{%- endfor %}
{%- endfor %}
    return suite.run()
}
