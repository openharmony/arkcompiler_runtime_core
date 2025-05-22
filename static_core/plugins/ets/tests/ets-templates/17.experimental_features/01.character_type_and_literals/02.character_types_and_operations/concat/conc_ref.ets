/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
{%- for v in  vals %}
/*---
desc: >-
    The string concatenation operator '+' (see String Concatenation), which (if a string operand and character
    operand are available) converts the character operand to a string and then creates a concatenation of
    the two strings as a new string.
---*/
function main() {
    const v1 = new Char({{v.v1}})
    const v2 = new Char({{v.v2}})

    assertEQ("" + v1 + v2, {{v.r1}})
    assertEQ(v1 + "" + v2, {{v.r1}})
    assertEQ((v1 + v2) as Char + "", {{v.r2}})

    assertEQ("" + {{v.v1}} + v2, {{v.r1}})
    assertEQ({{v.v1}} + "" + v2, {{v.r1}})
    assertEQ(({{v.v1}} + v2) as Char + "", {{v.r2}})

    assertEQ("" + {{v.v1}} + {{v.v2}}, {{v.r1}})
    assertEQ({{v.v1}} + "" + {{v.v2}}, {{v.r1}})
    assertEQ(({{v.v1}} + {{v.v2}}) as Char + "", {{v.r2}})
}
{%- endfor %}
