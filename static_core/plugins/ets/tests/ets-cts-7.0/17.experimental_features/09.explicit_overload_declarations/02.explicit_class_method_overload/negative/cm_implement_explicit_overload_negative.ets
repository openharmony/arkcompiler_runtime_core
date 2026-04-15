/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: If a class implements some interfaces with an explicit overload of the same name,
      then a new explicit overload must include all overloaded methods
tags: [compile-only, negative]
expected_out: "{{c.expected_out}}"
---*/

interface I1 {
    f1() {}
    f2() {}
    overload foo {f1, f2}
}

interface I2 {
    f3() {}
    f4() {}
    overload foo {f3, f4}
}

class C implements I1, I2 {
    {{c.expl_over}}
}

{% endfor %}
