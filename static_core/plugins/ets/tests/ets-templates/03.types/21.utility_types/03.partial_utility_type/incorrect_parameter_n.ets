/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for t in types %}
{% for c in cases %}

{% set response = c.out | default("Semantic error ESE0065: type %%partial-C has no property named i") %}
/*---
desc: variables of Partial<T> are to be initialized with valid object literals.
        - {{t}}, {{c.doc}}
tags: [compile-only, negative]
expected_out: '{{response}}'
---*/

{{t}} C {
  j: int {% if t != "interface" %} = 0; {% endif %}
}

function foo() {
  return -1
}

const hello: String = "Hello World"

function main() {
    let tp: Partial<C> = {{c.argument}}
}

{% endfor %}
{% endfor %}
