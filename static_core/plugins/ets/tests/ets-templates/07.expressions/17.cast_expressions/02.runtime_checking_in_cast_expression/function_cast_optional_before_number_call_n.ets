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

{% for c in cases %}
/*---
test_suffix: "{{c.test_suffix}}"
desc: >-
    Runtime checking in cast expression must reject a function value whose
    actual type is not a subtype of the target function type. The cast itself
    must throw ClassCastError before the result can be observed.The nested
    number-argument call below must never be reached.
---*/

function cast(a: Any) {
    return a as {{c.target_incompat}}
}

function main(): void {
    let reachedInnerCall: boolean = false
    arktest.expectThrow(() => {
        let res = ((p: Any) => {
            ((p: {{c.target_incompat}}) => {
                ((p: {{c.target_upcast}}) => {
                    reachedInnerCall = true
                })(p)
            })(cast(p))
        })({{c.expr}})
        // below prevents optimising unused return value
        arktest.assertTrue(res instanceof object, "this assert uses return value")
        arktest.assertTrue(false, "this code is not expected to run")
    }, (e: Error): boolean => e instanceof ClassCastError)
    arktest.assertFalse(reachedInnerCall, "not expected to reach inner call")
}
{% endfor %}
