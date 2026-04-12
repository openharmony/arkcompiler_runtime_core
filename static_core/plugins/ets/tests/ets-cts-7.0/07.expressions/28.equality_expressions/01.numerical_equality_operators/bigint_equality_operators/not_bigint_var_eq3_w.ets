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

{% yaml 'not_bigint.params.yaml' as yaml_file %}
{%- for c in yaml_file.cases %}

/*---
desc: >-
    If one operand is of type bigint, and the other is of a numeric type, then the
    result is false.
    If an equality expression is known at compile time to always evaluate to false
    or true at runtime, then a compile-time warning is issued.
expected_out: { compiler: ["Warning W65001: The equality expression is always false."] }
---*/

let b = 0n

function main() {
    let a = {{c.exp}}
    arktest.assertNE(a, b)
    arktest.assertFalse(a === b)
}
{%- endfor %}
