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
desc: |-
  Type char is a subtype of Object, and per chapter 3.9 Type Object it inherits all methods
  of class Object
test_suffix: {{modifier}}
---*/

function main() {
    {{modifier}} a_char: char = c'a'
    arktest.assertTrue(a_char instanceof char, "expect a_char instance of char")
    arktest.assertTrue(a_char instanceof Object, "expect a_char instance of Object")

    // verify Object method toString
    arktest.assertTrue(a_char.toString() instanceof string, "expect instance of string")
    arktest.assertEQ(a_char.toString(), "a", "expect equal values of string")
}
{% endfor %}
