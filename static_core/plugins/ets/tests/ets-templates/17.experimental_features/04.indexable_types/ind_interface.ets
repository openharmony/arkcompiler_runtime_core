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

{% for c in cases %}
/*---
desc: >-
    If a class or an interface declares one or two functions with names
    $_get and $_set, and signatures (index: Type1): Type2 and
    (index: Type1, value: Type2) respectively, then an indexing expression
    can be applied to variables of such types
tags: [{{c.tags}}]
---*/
{{c.decl}}

interface I {
    $_get(i: {{c.ind_type}}|undefined): {{c.val_type}}|undefined;
    $_set(i: {{c.ind_type}}, v: {{c.val_type}}): void;
}
class A implements I {
    ind: {{c.ind_type}}|undefined
    val: {{c.val_type}}|undefined
    $_get(i: {{c.ind_type}}|undefined): {{c.val_type}}|undefined {
        return i == this.ind ? this.val : undefined
    }
    $_set(i: {{c.ind_type}}, v: {{c.val_type}}) {
        this.ind = i
        this.val = v
    }
}

function main(): int {
    let i: I = new A
    {% for t in c['values'] %}
    if (i[{{t.ind}}] != undefined) return 1
    i[{{t.ind}}] = {{t.val}}
    if (i[{{t.ind}}] != {{t.val}}) return 1
    {% endfor %}
    return 0;
}
{% endfor %}
