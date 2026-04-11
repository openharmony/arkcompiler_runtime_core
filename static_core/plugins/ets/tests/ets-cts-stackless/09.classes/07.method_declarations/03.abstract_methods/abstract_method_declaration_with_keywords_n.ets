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

{% for m in modifiers %}

/*---
desc: >-
    Abstract method declaration with '{{mod}}' keyword.
    A compile-time error occurs if
    - An abstract method is declared private
    - The method declaration contains another modifier (static, final, native, or async) along
    with the modifier abstract.
tags: [compile-only, negative]
expected_out: "{{m.expected}}"
---*/

abstract class A {
    {{m.mod}} abstract run(): {% if m.mod != "async" %}void{% else %}Promise{%endif %};
}

{%- endfor %}
