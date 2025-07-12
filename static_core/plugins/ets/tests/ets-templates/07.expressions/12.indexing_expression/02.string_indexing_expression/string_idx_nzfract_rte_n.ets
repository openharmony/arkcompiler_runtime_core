/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    If an index expression is of type number or other floating-point type,
    and the fractional part differs from 0, then errors occur as follows:
    A runtime error, if the situation is identified during program execution; and
    A compile-time error, if the situation is detected during compilation.
---*/

let str: string = "abcdefg"

function foo (index: {{c['type']}}) {
  let val = str[index]
}

function main(): int {
  try {
    foo({{c['index']}})
  } catch (e) {
    return 0
  }
  return 1
}

{% endfor %}
