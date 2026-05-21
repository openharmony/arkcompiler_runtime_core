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

{%- for c in cases %}
/*---
desc: >-
    Receiver type is the type of the receiver parameter in a function, function type, and lambda with receiver.
    A receiver type can be an interface type, a class type, or an array type. Otherwise, a compile-time error occurs.
tags: [compile-only, negative]
expected_out: "{{c.out}}"
---*/

{{c.decl}}
{%- if c.use is defined %}

function main() {
    {{c.use|indent}}
}
{%- endif %}

{%- endfor %}
