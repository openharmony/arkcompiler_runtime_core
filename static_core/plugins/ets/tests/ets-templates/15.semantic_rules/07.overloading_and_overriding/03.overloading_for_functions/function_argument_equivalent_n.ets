/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

/*---
desc: >-
    The correctness check for functions overloading is performed if two or more functions with the same name are acces-
    sible (see Accessible) in a scope (see Scopes).
---*/

let res: number = 0;

type FUN = () => boolean;

{% for c in cases %}
function foo({{c.arg}}): void {
    res = {{c.res}};
}
{% endfor %}

function main(): void {
    {% for d in cases %}
    foo({{d.call}});
    assertEQ(res, {{d.res}});
    {% endfor %}
}
