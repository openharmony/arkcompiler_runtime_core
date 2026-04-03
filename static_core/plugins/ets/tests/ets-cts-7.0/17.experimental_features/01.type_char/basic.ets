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
  Values of char type are 16-bit Unicode code units. Type char is a subtype of Object,
  and it can be used at any place where a class name is expected
test_suffix: {{modifier}}
---*/

function main() {
    {{modifier}} a_char: char = c'a'
    arktest.assertTrue(a_char instanceof char, "expect a_char instance of char")
    arktest.assertTrue(a_char instanceof Object, "expect a_char instance of Object")

    // verify assignability
    {{modifier}} o: Object = a_char
    arktest.assertTrue(o instanceof Object, "expect a_char assignable to Object")

    // range limits
    {{modifier}} min: char = c'\u0000'
    arktest.assertTrue(min instanceof char, "expect min instance of char")
    {{modifier}} max: char = c'\uFFFF'
    arktest.assertTrue(max instanceof char, "expect max instance of char")

    // default value per ch 3.22
    {{modifier}} default_char: char = new char
    arktest.assertEQ(default_char, min, "expect default_char value U+0000")
    {{modifier}} default_char1: char = new char()
    arktest.assertEQ(default_char1, min, "expect default_char1 value U+0000")

    // constructor with non-default value
    {{modifier}} value_0001: char = new char(c'\u0001')
    arktest.assertEQ(value_0001, c'\u0001', "expect value U+0001")

    // array
    {{modifier}} array: char[] = []
    arktest.assertTrue(array instanceof Array, "expect instance of Array")
}
{% endfor %}
