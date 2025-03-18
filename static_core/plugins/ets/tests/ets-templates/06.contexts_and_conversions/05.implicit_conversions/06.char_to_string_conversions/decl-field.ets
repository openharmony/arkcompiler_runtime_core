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
    Declaration context. Instance field declaration.
---*/
{%- for v in chars %}
let c{{loop.index}}: char = {{v.char}};
{%- endfor %}

class A {
    // from char literals:
{%- for v in chars %}
    fld{{loop.index}}: string = {{v.char}}
{%- endfor %}
}

class B {
    // from char variables:
{%- for v in chars %}
    fld{{loop.index}}: string = c{{loop.index}}
{%- endfor %}
}

function main() {
    let a: A = new A()
{%- for v in chars %}
    assertTrue( a.fld{{loop.index}} == {{v.str}} )
{%- endfor %}

    let b: B = new B()
{%- for v in chars %}
    assertTrue( b.fld{{loop.index}} == {{v.str}} )
{%- endfor %}
}
