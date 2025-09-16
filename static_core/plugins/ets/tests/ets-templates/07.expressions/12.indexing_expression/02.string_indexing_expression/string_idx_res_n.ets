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
    The result of a string indexing expression is a value of string type.
    Index expression for array indexing must be of a numeric type (see Numeric Types).
    The same rules apply as those for Array Indexing Expression.
---*/

let str: string = "\x17\x2b\xef\0\u1f48\u2331\u2b03\u2eec \n \u{CAFE}\n\uBABF生日快乐!С Днём рождения!"
let vals: string[] = ["\x17", "\x2b", "\xef", "\0", "\u1f48", "\u2331", "\u2b03", "\u2eec"," ", "\n",
" ", "\u{CAFE}", "\n", "\uBABF", "生", "日", "快", "乐", "!", "С", " ", "Д", "н", "ё", "м", " ", "р",
"о", "ж", "д", "е", "н", "и", "я", "!"]

function main(): void {
  let id: {{c['type']}}

  id = 0{{c['add_val']}}
  arktest.assertEQ(str[id], "\x17")

  id = 3{{c['add_val']}}
  arktest.assertEQ(str[id], "\u1f48")

  id = 8{{c['add_val']}}
  arktest.assertEQ(str[id], "\n")

  id = 10{{c['add_val']}}
  arktest.assertEQ(str[id], "\u{CAFE}")

  id = 13{{c['add_val']}}
  arktest.assertEQ(str[id], "生")

  id = 18{{c['add_val']}}
  arktest.assertEQ(str[id], "С")

  arktest.assertEQ(str.length, vals.length)
  for (let i: {{c['type']}} = 0{{c['add_val']}}; i < str.length; i += 1{{c['add_val']}}) {
    arktest.assertEQ(str[i], vals[i])
  }
  arktest.assertEQ(str.length, 35)
}
{% endfor %}
