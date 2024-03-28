/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
{%- for op in ops %}
/*---
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    Each operand, separately, of a shift operator.
params: "{{op.name}}, {{c.vtype}} or {{c.rtype}} cannot be promoted to primitive integral."
tags: [compile-only, negative]
---*/

function main(): int {
  let v: {{c.vtype}} = {{c.vval}};
  let r: {{c.rtype}} = {{c.rval}};
  if (v {{op.op}} r != 0) {
    return 1;
  }
  return 0;
}
{% endfor %}
{% endfor %}
