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
    Function types conversion is the conversion of one function type to another.
    Call context, method, function type conversion, implicit
---*/
{{c.decl}}

class Z {
{%- for t in c['to'] %}
    meth{{loop.index}}(p: {{t.type}}): {{t.type}} {
        return p
    }
{%- endfor %}
}

function main() {
    let z: Z = new Z()
{% for t in c['to'] %}
    assert z.meth{{loop.index}}(foo){{t.call}}
{% endfor %}
}
{%- endfor %}
