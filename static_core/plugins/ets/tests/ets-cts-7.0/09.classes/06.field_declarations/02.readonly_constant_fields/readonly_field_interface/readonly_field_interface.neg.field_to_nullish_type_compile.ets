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
{% yaml "readonly_field_interface.data.yaml" as yaml_file %}
{% for c in yaml_file.cases2 + yaml_file.cases4 %}
/*---
desc: >- 
   Checking interface field implementation with late initialization and nullish type.
expected_out: >- 
    Semantic error ESE0376: Late-initialized field cannot be nullish types or possibly nullish types.
tags: [compile-only, negative]
---*/

interface I {
    readonly field: {{c.type}}
}

class A implements I {
    field!: {{c.type}}
}

function main(): void {}

{% endfor %}

