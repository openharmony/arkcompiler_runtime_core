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

{% for p in cases %}
/*---
desc: >-
    Names and aliases of the Predefined Types are hard keywords. They
    cannot be used as identifiers. Type aliases are identifiers.
tags: [negative, compile-only]
expected_out: |-
    {{p.expected or ("Syntax error ESY0295: " + p.type + " is a predefined type, cannot be used as an identifier")}}
---*/

class A {}

type {{p.type}} = Array<A>;

function main() {}
{%- endfor %}
