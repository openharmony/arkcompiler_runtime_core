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
desc: >-
    The result of an as expression is the result of the evaluated expr
    if the actual type of expr is a subtype of target (see Subtyping).
    Otherwise, ClassCastError is thrown.
expected_out: |-
    Warning W0326: Attempting to cast type 'Enum1' to 'Enum2' can throw 'CastClassError' exception at runtime
---*/

{{c.decl}}

let isCCE = (e: Error) => e instanceof ClassCastError
let tmp: Any

function main() {
    let v: Enum1 = Enum1.Green
    arktest.expectThrow(() => { tmp = v as Enum2 }, isCCE)
}
{% endfor %}
