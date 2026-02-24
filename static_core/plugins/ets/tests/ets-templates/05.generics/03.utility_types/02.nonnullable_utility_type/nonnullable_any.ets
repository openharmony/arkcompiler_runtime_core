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

{% for case in cases %}

/*---
desc: Type NonNullable<T> constructs a type by excluding null and undefined types -
      NonNullable<Any> = Any - null - undefined
tags: [compile-only, negative]
expected_out: "TODO: throw CTE for null or undefined to NonNullable<Any> assignment"
---*/

type NNANY = NonNullable<Any>
type NNNNANY = NonNullable<NNANY>

function main() {

    let u: undefined = undefined
    let n: null = null

    let ynn: NNANY = new Object
    let ynnnn: NNNNANY = new Object

    // should fail since NonNullable<Any> = Any - null - undefined
    {{ case }}
}
{% endfor %}