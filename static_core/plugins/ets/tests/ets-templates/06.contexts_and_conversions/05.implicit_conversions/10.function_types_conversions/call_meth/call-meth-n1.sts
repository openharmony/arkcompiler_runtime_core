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
    A compile-time error occurs if a throwing function value is assigned
    to a non-throwing function type variable.
    Call context, method, function type conversion, implicit
tags: [compile-only, negative]
---*/
{{c.decl}}

class Z {
    meth(p: {{c.type}}): {{c.type}} {
        return p
    }
}

function main() {
    let z: Z = new Z()
    let x = z.meth(foo)
}
{%- endfor %}
