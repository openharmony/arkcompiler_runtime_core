/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Type arguments
params: method
---*/

interface I {}

class X implements I {}

final class Y<T> extends X {}

enum Color { Red, Green, Blue }

class A <V> {
    private v: V

    public meth <T{{c.bound}}> (v: V, t: T): T {
        this.v = v
        let tmp: T = t
        return tmp;
    }
}

{{c.decl}}

function main(): int {
    let a: A<X> = new A<X>()
    {{c.use|indent}}
    return 0;
}
{% endfor %}
