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

{% for c in cases %}
/*---
desc: |-
  Equality operators (see Equality Expressions) and relational operators Relational Expressions) can be used if:
  - both operands are of char type; or
  - one operand is of char type and other is of a numeric type (see char Conversions for Relational and Equality Operands);
  - otherwise, a compile-time error occurs.
tags: [compile-only, negative]
expected_out: "{{c.expected_out}}"
---*/

{{c.decl}}
let v: boolean = false

function main() {
    {{c.main|indent}}
}
{%- endfor %}
