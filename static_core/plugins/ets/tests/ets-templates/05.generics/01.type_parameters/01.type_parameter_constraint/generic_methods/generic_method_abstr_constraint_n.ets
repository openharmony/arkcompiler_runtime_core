/*---
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
---*/

{% yaml "generic_method_abstr_and_with_body_constraint.data.yaml" as dt %}
{% for mm in dt.abstr_or_native_method_modifiers %}
{% for c in dt.constraints %}

/*---
desc: >-
    If type parameter T has type constraint S, then the actual type of the generic instantiation must be
    compatible with S (see Type Compatibility).
expected_out:
    - "Semantic error ESE0127: No matching call signature for foo()"
    - "{{c.expected_out}}"
tags: [compile-only, negative]
---*/

{% if "Color" in c %}
enum Color { Red, Green, Blue}
{% endif %}

abstract class Point {
	{{mm}} foo<X extends {{c.type}}>(): void;
}

class A extends Point {
    foo<X extends {{c.type}}>(): void {}
}

function main() {
    new A().foo<{{c.use}}>()
}

{% endfor %}
{% endfor %}
