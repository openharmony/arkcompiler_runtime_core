/**
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

{% for c in cases %}
/*---
desc:
    If the type of any operand is byte or short, then the operand is converted to int. 
    If operands are of different integer types, then the operand of a smaller type is converted to a larger type by using Widening Numeric Conversions.
tags: [compile-only, negative]
expected_out: "{{c.expected}}"
---*/

function main() {
    let a: {{c.overflow_type}} = {{c.use}}
}

{% endfor %}
