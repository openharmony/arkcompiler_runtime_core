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

{% yaml "implicit_type_resolution.data.yaml" as yaml_file %}

{% for c in yaml_file.cases %}
/*---
desc: Implicit type resolution for functions with receiver and mixed calls
---*/

type X = Array<{{c.type}}>

function foo<T>(this: X, el: Int): T {
    return this[el] as T
}

function bar<T>(this: X, el: Int): T {
    let x1 : T = this.foo<T>(el)
    return x1 
}

function main() {
    let x: X = {{c.value}}
    arktest.assertEQ(x.bar<{{c.type}}>(0), {{c.expected}})
}

{%- endfor %}