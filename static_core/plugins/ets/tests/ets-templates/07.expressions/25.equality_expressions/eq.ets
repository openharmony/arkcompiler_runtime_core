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

{%- for c in cases %}

/*---
desc: >-
    The two forms have the same semantics regardless of the number of the symbols '=' used in the operator.
    The form of the operator '===' is identical to '==', and '!==' is identical to '!='.
---*/
{{c.decl}}

function main() {
    let a = {{c.a}}
    let b = {{c.b}}
    assertTrue( (a == b) == (a === b) )
    assertTrue( (a != b) == (a !== b) )
}
{%- endfor %}
