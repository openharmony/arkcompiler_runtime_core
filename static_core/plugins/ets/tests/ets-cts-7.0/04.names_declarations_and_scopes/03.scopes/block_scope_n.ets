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

{% for modifier in ["let", "const"] %}

/*---
desc: >-
  The scope of a name declared inside a block is the body of the block from the point
  of the name declaration and up to the end of the block (block scope).
tags: [negative, compile-only]
expected_out: "Semantic error ESE0143: Unresolved reference x"
test_suffix: "{{modifier}}"
---*/

function  foo(x: Any): void {}

function main(): void {
  {
    {{modifier}} x = 0;
  }
  {
    {{modifier}} x = 0;
  }
  foo(x)
}
{%- endfor %}
