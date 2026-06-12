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
    Runtime checking in cast expression must accept a function value whose
    actual type with optional parameters can be cast to the target function type.
    The cast itself must succeed without throwing ClassCastError.
---*/

function cast(a: Any) {
    return a as {{c.target_incompat}}
}

function main(): void {
    let reachedInnerCall: boolean = false
    let res = ((p: Any) => {
        ((p: {{c.target_incompat}}) => {
            ((p: {{c.target_upcast}}) => {
                reachedInnerCall = true
            })(p)
        })(cast(p))
    })({{c.expr}})
    arktest.assertTrue(reachedInnerCall)
}
{% endfor %}
