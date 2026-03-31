/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for modifier in ["let", "const"] %}

/*---
desc: >-
  The scope of a name declared inside a block is the body of the block from the point
  of the name declaration and up to the end of the block (block scope).
test_suffix: "{{modifier}}"
---*/

function main(): void {
  {{modifier}} x = 0;
  arktest.assertEQ(x, 0, "out of block scope x 0 expected")
  {
    {{modifier}} x = 1;
    arktest.assertEQ(x, 1, "block scope x 1 expected")
    {{modifier}} y = 1;
    arktest.assertEQ(y, 1, "block scope y 1 expected")
  }
  {
    {{modifier}} y = 2;
    arktest.assertEQ(y, 2, "block scope y 2 expected")
  }
  {{modifier}} y = 0;
  arktest.assertEQ(y, 0, "out of block scope y 0 expected")
}
{%- endfor %}
