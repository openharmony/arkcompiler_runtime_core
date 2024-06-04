/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
    A rest parameter allows to make a function or method that take an unbounded
    number of arguments. A rest parameter is marked with ... symbol before parameter
    name.
---*/

function {{c.sig}} {
    {{c.def|indent}}
}

function main(): int {
    {{c.call|indent}}
    return 1;
}
{% endfor %}
