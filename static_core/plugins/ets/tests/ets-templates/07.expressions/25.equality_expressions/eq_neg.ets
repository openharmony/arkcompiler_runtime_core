/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

{%- for op in operators %}
    {%- for index1 in range( types|length ) %}
        {%- for index2 in range( (index1 + 1), types|length ) %}
            {%- for t1 in types[index1].types %}
                {%- for t2 in types[index2].types %}

/*---
desc: |-
    Negative cases for equality expressions.
    Comparison of {{types[index1].name}} vs. {{types[index2].name}}
tags: [compile-only, negative]
---*/

function compare(a: {{t1}}, b: {{t2}}) {
    return a {{op}} b
}

// Auxiliary stuff
class C {}
interface I {}

enum EN { a, b, c }
enum ES { a = "a" , b = "b", c = "c" }

                {%- endfor %}
            {%- endfor %}
        {%- endfor %}
    {%- endfor %}
{%- endfor %}
