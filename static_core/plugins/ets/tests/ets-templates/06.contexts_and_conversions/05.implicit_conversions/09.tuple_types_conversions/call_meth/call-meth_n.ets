/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    Tuple types conversion is the conversion of one tuple type to another.
    Call context, instance method, tuple type conversion, implicit
tags: [compile-only, negative]
---*/
{{c.decl}}

let s: [{% for t in c['from'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}] = [{% for t in c['from'] %}{{t.expr|safe}}{% if not loop.last %}, {% endif %}{% endfor %}];

class Z {
    meth(p: [{% for t in c['to'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}]): [{% for t in c['to'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}] {
        return p
    }
}

function main() {
    let res1: [{% for t in c['to'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}] = new Z().meth([{% for t in c['from'] %}{{t.expr|safe}}{% if not loop.last %}, {% endif %}{% endfor %}])
    let res2: [{% for t in c['to'] %}{{t.type}}{% if not loop.last %}, {% endif %}{% endfor %}] = new Z().meth(s)
}
{%- endfor %}
