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
{% for c in yaml_file.cases + yaml_file.cases2 + yaml_file.cases3 + yaml_file.cases4 %}
/*---
desc: >- 
    Checking interface field implementation in class as readonly with direct initialization.
expected_out: >- 
    Semantic error ESE0337: Cannot implement interface I: property 'fieldI' must have a setter, but the implementation is readonly
tags: [compile-only, negative]
---*/

interface I {
    fieldI: {{c.type}}
}

class A implements I {
    readonly fieldI: {{c.check_type}} = {{c.base_value}}
}

function main(): void {}

{% endfor %}

