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

let tmp: Any, isCCE = (e: Error) => e instanceof ClassCastError

function main(): int {
    let suite = new arktest.ArkTestsuite("Runtime checking in cast expression of value type to union")

{% for c in cases %}
    suite.addTest("{{c.type}} as {{c.union_type_pos}}", () => {
        ((p: Any) => { tmp = p as {{c.union_type_pos}} })({{c.value}})
        arktest.assertTrue(tmp instanceof {{c.union_type_pos}})
        arktest.assertEQ(tmp, {{c.value}})
    })

    suite.addTest("{{c.type}} as {{c.union_type_neg}}", () => {
        arktest.expectThrow(
            () => { ((p: Any) => { tmp = p as {{c.union_type_neg}} })({{c.value}}) },
            isCCE
        )
    })
{% endfor %}
    return suite.run()
}
