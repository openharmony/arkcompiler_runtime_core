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
desc: >-
    Fixed-size array can be created by using Array Literal or constructors defined for type FixedArray<T>,
    where T must be a type preserved by Type Erasure.
tags: [compile-only, negative]
expected_out: "{{c.exp_out}}"
---*/

{%-if c.decl is defined%}
{{c.decl}}
{%- endif %}

function main() {
   {{c.use}}
}

{%- endfor %}
