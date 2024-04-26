/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
    Declaration context, variable declaration.
---*/
{%- for v in chars %}
let c{{loop.index}}: char = {{v.char}}
{%- endfor %}

function main() {
    // from char literals:
{%- for v in chars %}
    let s{{loop.index}}: string = {{v.char}}
    assert s{{loop.index}} == {{v.str}}
{%- endfor %}
    // from char variables:
{%- for v in chars %}
    let q{{loop.index}}: string = c{{loop.index}}
    assert q{{loop.index}} == {{v.str}}
{%- endfor %}
}
