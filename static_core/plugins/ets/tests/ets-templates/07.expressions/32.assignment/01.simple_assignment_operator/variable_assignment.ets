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
  If none of the above is true (not field access, array indexing, or record indexing), then the following three steps are performed:
  1. lhsExpression is evaluated to produce a variable. If the evaluation completes abruptly, then so does the assignment expression. In that case, rhsExpression is not evaluated, and no assignment occurs.
  2. If the evaluation completes normally, then rhsExpression is evaluated. If the evaluation completes abruptly, then so does the assignment expression. In that case, no assignment occurs.
  3. If that evaluation completes normally, then the value of rhsExpression is converted to the type of the left-hand-side variable. In that case, the result of the conversion is assigned to the variable.

  {{c.desc}}
params: variable assignment
---*/

let result: string = ""

function getValue(): {{c.value_type}} {
    result = result + "getValue "
    {{c.value}}
}

function main() {
    result = "";

    {{c.main|indent}}

    arktest.assertEQ(result, {{c.assert_result}})
    arktest.assertEQ(v, {{c.assert_v}})
}
{% endfor %}
