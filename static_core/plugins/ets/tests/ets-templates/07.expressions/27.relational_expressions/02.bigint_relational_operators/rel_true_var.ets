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
{% yaml 'rel_true.params.yaml' as yaml_file %}

{%- for c in yaml_file.cases %}

/*---
desc: CTE - BigInt vs numeric relational comparison with {{c.op}}
tags: [compile-only, negative]
{% if 'Double.' in c.exp %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Double'."
{% elif 'Float.' in c.exp %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Float'."
{% elif '.' in c.exp %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Double'."
{% else %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Int'."
{% endif %}
---*/

let b = {{c.b}}

function main() {
    let _ = b {{c.op}} {{c.exp}}
}
{%- endfor %}