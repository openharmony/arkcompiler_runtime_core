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
    Character operators that produce a value of type char.
---*/

function minus(p: Char): Char {
    // Widening to int happens
    return -p as Char
}

function plus(p: Char): Char {
    // Widening to int happens
    return +p as Char
}

function main() {
    const v1 = new Char({{v.v1}})

    assertEQ( ({{v.exp}}), {{v.r}} )
}
{%- endfor %}
