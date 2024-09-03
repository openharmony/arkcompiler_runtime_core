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
/*---
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    The index expression in an array access expression.
params: {{c.type}} cannot be promoted to int
tags: [compile-only, negative]
---*/

function main(): int {
  let v: {{c.type}} = {{c.val}};
  let arr: {{c.type}}[] = new {{c.type}}[{{c.len}} + 1];
  arr[v] = v;
  return 0;
}
{% endfor %}
