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

{% yaml "readonly_static_field_class.data.yaml" as yaml_file %}

{% for c in yaml_file.cases2 %}
/*---
desc: Checking for changing the value in derived class. (lambda, not init static field, not default value)
expected_out: >- 
    Semantic error ESE4002: Cannot assign to a readonly field fieldA
tags: [compile-only, negative]
---*/

class Base {
    static readonly fieldA: {{c.type}}
}

class A extends Base {}

function main(){
    const lambda = () => {
        A.fieldA = {{c.check_value}}
    }
}
{% endfor %} 