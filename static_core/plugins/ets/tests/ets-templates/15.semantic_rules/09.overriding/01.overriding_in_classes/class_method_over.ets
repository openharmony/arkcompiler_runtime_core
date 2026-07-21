/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    If an instance method is defined or inherited by a subclass with the same name as the instance method in a superclass
    or superinterface, then the following semantic rules are applied:
    - If signatures are not override-compatible (see Override-Compatible Signatures), and if signatures formed by
    using effective signature types of original signatures are override-compatible after type erasure, then a compile-
    time error occurs.
    - If signatures are override-compatible (see Override-Compatible Signatures), then the method of subinterface
    overrides the method of superinterface in the subinterface.

    {{c.doc|indent}}
---*/

let res: number = 0;

{{c.decl}}

function main(): void {
    res = 0
    {{c.use|indent}}
}
{% endfor %}
