/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}

/*---
desc: >-
    All direct superinterfaces of the parameterized interface type I <T1 ,..., Tn >
    are types J <U1 𝜃 ,..., Uk 𝜃>, if:
    • Ti (1 ≤ 𝑖 ≤ 𝑛) is the type of a generic interface declaration I <F1 ,..., Fn > (𝑛 > 0);
    • J <U1 ,..., Uk > is a direct superinterface of I <F1 ,..., Fn >; and
    • 𝜃 is a substitution [F1 := T1 ,..., Fn := Tn ].
    {{c.doc}}
tags: [compile-only, negative]
expected_error: "{{c.error}}"
---*/

{{c.decl}}

{% endfor %}
