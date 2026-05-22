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

{% for modifier in ['', 'protected', 'public'] %}

/*---
desc: Implementing class has same field inherited from superclass.
test_suffix: "{{modifier}}"
---*/

class X {
    {{modifier}} code_: int = 1
}

interface I {
    code_: int
}

class MyCL extends X implements I {}

function main(): void {
    let instance: MyCL = new MyCL

    arktest.assertTrue(instance instanceof I, "expect instance of I")
    arktest.assertTrue(instance instanceof X, "expect instance of X")

    let i: I = instance
    arktest.assertEQ(i.code_, 1, "field via interface")
}
{% endfor%}
