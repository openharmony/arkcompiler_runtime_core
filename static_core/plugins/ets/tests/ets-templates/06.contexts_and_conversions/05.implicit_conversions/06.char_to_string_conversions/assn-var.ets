/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

/*---
desc: >-
    Character to string conversion converts a value of type char to type
    string. The resultant new string has the length equal to 1.
    Assignment context. Variable assignment.
---*/
{%- for v in chars %}
let v{{loop.index}}: char = {{v.char}}
{%- endfor %}

{%- for v in chars %}
const c{{loop.index}}: char = {{v.char}}
{%- endfor %}

let s: string

function main() {
    // from char literals:

{%- for v in chars %}
    s = {{v.char}}
    assertEQ( s, {{v.str}} )
{%- endfor %}

    // from char variables:
{%- for v in chars %}
    s = v{{loop.index}}
    assertEQ( s, {{v.str}} )
{%- endfor %}

    // from char constants:
{%- for v in chars %}
    s = c{{loop.index}}
    assertEQ( s, {{v.str}} )
{%- endfor %}
}
