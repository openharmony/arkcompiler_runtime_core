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
desc: >-
    Unary numeric promotion is performed on expressions in the following situation:
    The operand of a unary minus operator.
params: {{c.type}} cannot be promoted
tags: [compile-only, negative]
---*/

function foo(a: Object): int {
  return 0;
}

function foo(a: boolean): int {
  return 0;
}

function foo(a: int): int {
  return 0;
}

function foo(a: double): int {
  return 0;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  return foo(-v);
}
{% endfor %}
