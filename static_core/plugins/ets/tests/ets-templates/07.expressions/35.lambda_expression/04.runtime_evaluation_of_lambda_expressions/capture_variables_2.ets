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

{% for cc in cases %}
/*---
desc: Every time a lambda expression is evaluated, the outer variables referred to by
      the lambda expression are captured.
---*/

enum Color { Red, Green, Blue }
enum ByteEnum: byte { A = 0, B = 1, C = 3 }

const bv: byte = 12
const sv: short = 1234
const lv: long = 12345678900
const ln: number = 1.234

let outer = {{cc.outer}}

function foo() {
    let x = () => { {{cc.expression}} } // 'outer' is *captured*.
    x()
    arktest.assertEQ({{cc.expected}}, outer, 'outer capture failed' )
}

function main() {
    outer = {{cc.outer}}
    foo()
}

{% endfor %}
