/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Numeric contexts apply to the operands of an arithmetic operator.
params: {{c.type1}}, {{c.type2}}
tags: [{{c.tags}}]
---*/

{{c.defs}}

function main(): int {
  let v1: {{c.type1}} = {{c.v1|safe}};
  let v2: {{c.type2}} = {{c.v2|safe}}
  if (v1 {{c.op}} v2 != {{c.res}}) {
    return 1;
  }
  return 0;
}
{% endfor %}
