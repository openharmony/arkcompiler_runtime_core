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

enum Enum_int { Red, Green, Blue }
enum Enum_byte: byte { Red = 1, Green = 2, Blue }
enum Enum_short: short { Red = 100, Green = 200, Blue }
enum Enum_long: long { Red = 1000, Green, Blue = 3000 }
enum Enum_float: float { Red = 0.1f, Green = 0.2f, Blue = 0.3f }
enum Enum_double: double { Red = 0.1, Green = 0.2, Blue = 0.3 }

let tmp: Any, isCCE = (e: Error) => e instanceof ClassCastError

function main(): int {
    let suite = new arktest.ArkTestsuite("Runtime checking in cast expression of value type to union with enum")

{% for c in cases %}
    suite.addTest("{{c.type}} as {{c.union_type_pos}}|Enum_{{c.type|lower}}", () => {
        ((p: Any) => { tmp = p as {{c.union_type_pos}}|Enum_{{c.type|lower}} })({{c.value}})
        arktest.assertTrue(tmp instanceof {{c.union_type_pos}})
        arktest.assertEQ(tmp, {{c.value}})
    })

    suite.addTest("{{c.type}} as {{c.union_type_neg}}|Enum_{{c.type|lower}}", () => {
        arktest.expectThrow(
            () => { ((p: Any) => { tmp = p as {{c.union_type_neg}}|Enum_{{c.type|lower}} })({{c.value}}) },
            isCCE
        )
    })
{% endfor %}
    return suite.run()
}
