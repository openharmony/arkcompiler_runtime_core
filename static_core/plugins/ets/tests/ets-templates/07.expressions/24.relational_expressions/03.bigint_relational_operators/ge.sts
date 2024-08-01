/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

{%- for c in cases %}

/*---
desc: >-
    The type of each operand in a bigint relational operator must be bigint.
    Otherwise, a compile-time error occurs.
    All bigint relational operators compare bigint values.
---*/

let ba: bigint[] = [
    {%- for v in c['values'] %}
    {{v}}{{ ", " if not loop.last }}
    {%- endfor %}
]

function main(): int {
    for (let i: int = 0; i < ba.length - 1; i++) {
        for (let j: int = i + 1; j < ba.length; j++) {
            let a = ba[i]
            let b = ba[j]

            if (!(a >= b)) {
                return 1
            }
            if (!(b >= a)) {
                return 1
            }
            if (a - 1n >= b) {
                return 1
            }
            if (b - 1n >= a) {
                return 1
            }
        }
    }
    return 0
}
{%- endfor %}
