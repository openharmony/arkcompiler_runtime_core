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
    Call context. Constructor.
---*/
{%- for v in chars %}
let c{{loop.index}}: char = {{v.char}}
{%- endfor %}

class A {
    fld: string;
    constructor(p: string) {
        this.fld = p
    }
}

function main() {
    // from char literals:
{%- for v in chars %}
    assertTrue( new A({{v.char}}).fld == {{v.str}} )
{%- endfor %}

    // from char variables:
{%- for v in chars %}
    assertTrue( new A(c{{loop.index}}).fld == {{v.str}} )
{%- endfor %}
}
