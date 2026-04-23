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

/*---
desc: >-
    The result of an as expression is the result of the evaluated expr
    if the actual type of expr is a subtype of target (see Subtyping).
    Otherwise, ClassCastError is thrown.
---*/

enum Enum_int_1 { Red, Green, Blue }
enum Enum_int_2 { Red, Green, Blue }
enum Enum_int_3: int { Red = 10, Green = 20, Blue }
enum Enum_int_4: int { Red = 10, Green = 20, Blue }

enum Enum_string_1 { Red = 'r', Green = 'g', Blue = 'b' }
enum Enum_string_2 { Red = 'r', Green = 'g', Blue = 'b' }
enum Enum_string_3: string { Red = 'R', Green = 'G', Blue = 'B' }
enum Enum_string_4: string { Red = 'R', Green = 'G', Blue = 'B' }

enum Enum_byte_1: byte { Red = 1, Green = 2, Blue }
enum Enum_byte_2: byte { Red = 1, Green = 2, Blue }

enum Enum_short_1: short { Red = 100, Green = 200, Blue }
enum Enum_short_2: short { Red = 100, Green = 200, Blue }

enum Enum_long_1: long { Red = 1000, Green, Blue = 3000 }
enum Enum_long_2: long { Red = 1000, Green, Blue = 3000 }

enum Enum_float_1: float { Red = 0.1f, Green = 0.2f, Blue = 0.3f }
enum Enum_float_2: float { Red = 0.1f, Green = 0.2f, Blue = 0.3f }

enum Enum_double_1: double { Red = 0.1, Green = 0.2, Blue = 0.3 }
enum Enum_double_2: double { Red = 0.1, Green = 0.2, Blue = 0.3 }

let tmp: Any, isCCE = (e: Error) => e instanceof ClassCastError

function main(): int {
    let suite = new arktest.ArkTestsuite("Runtime checking in cast expression of enum type to enum type")

{% for c1 in cases %}
{%- for c2 in cases %}
    suite.addTest("{{c2.type}} as {{c1.type}}", () => {
        {%- if c1.type.upper() == c2.type.upper() %}
        ((p: Any) => { tmp = p as {{c1.type}} })({{c2.value}})
        arktest.assertTrue(tmp instanceof {{c1.type}})
        arktest.assertEQ(tmp, {{c2.value}})
        {%- else %}
        arktest.expectThrow(
            () => { ((p: Any) => { tmp = p as {{c1.type}} })({{c2.value}}) },
            isCCE
        )
        {%- endif %}
    })
{%- endfor %}
{% endfor %}
    return suite.run()
}
