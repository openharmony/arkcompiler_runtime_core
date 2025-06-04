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
{%- for v in  vals %}
/*---
desc: >-
    ArkTS provides a number of operators to act on character values:
    Conditional operator ‘?:’ (see Conditional Expressions);
---*/

class A {
    static b: boolean = {{v.b}}
    static v1: char = {{v.v1}}
    static v2: char = {{v.v2}}
    static r: char = {{v.r}}
}

function main() {
    assertEQ( (A.b ? A.v1 : A.v2), A.r )
}
{%- endfor %}
