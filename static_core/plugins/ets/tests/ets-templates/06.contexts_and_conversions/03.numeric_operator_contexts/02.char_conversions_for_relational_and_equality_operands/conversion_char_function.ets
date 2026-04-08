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

{% for t in types %}
{% for op in operators %}
/*---
desc: >-
    A char type operand is widened to one of the following:
        - Type int if the other operand is of types byte, char or int; or
        - Type of the other operand.
    char {{op[0]}} {{t[0]}}
---*/
function foo(a: char, b: {{t[0]}}, c: boolean): void {
    arktest.assertEQ((a {{op[0]}} b), c)
    arktest.assertEQ((b {{op[0]}} a), c)
}

function main(): void {
  let a: char = c'\u0001'
  let b: {{t[0]}} = {{t[1]}}
  foo(a, b, {{op[1]}})
}

{% endfor %}
{% endfor %}
