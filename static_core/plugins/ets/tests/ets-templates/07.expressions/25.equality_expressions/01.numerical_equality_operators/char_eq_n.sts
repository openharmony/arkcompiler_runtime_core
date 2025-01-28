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
    The operation is a character equality if: both operands of a value equality operator are of type char;
    or one operand is of type char and the other is of type Char.
    The character equality operators are associative.
tags: [compile-only, negative]
---*/

function main(): void {
  let c1 = {{c.c1}}
  let c2 = {{c.c2}}
  assertTrue( c1 {{c.op}} c2 )
}
{% endfor %}
