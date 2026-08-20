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
    CTE: BigInt relational operation with non-BigInt operand {{c.operand_val}} using operator '{{c.op}}'
tags: [compile-only, negative]
{% if 'as byte' in c.operand_val %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Byte'."
{% elif 'as short' in c.operand_val %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Short'."
{% elif 'as int' in c.operand_val %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Int'."
{% elif 'as long' in c.operand_val %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Long'."
{% elif (c.operand_val|string).endswith('f') %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Float'."
{% elif 'Double.' in c.operand_val|string %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Double'."
{% elif 'Float.' in c.operand_val|string %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Float'."
{% elif '.' in c.operand_val|string %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Double'."
{% else %}
expected_error: "Semantic error ESE0105: Operator '{{c.op}}' cannot be applied to types 'BigInt' and 'Int'."
{% endif %}
---*/

function main() {
    let a = {{c.b}};
    let b = {{c.operand_val}};
    let c = a {{c.op}} b;
}
{%- endfor %}
