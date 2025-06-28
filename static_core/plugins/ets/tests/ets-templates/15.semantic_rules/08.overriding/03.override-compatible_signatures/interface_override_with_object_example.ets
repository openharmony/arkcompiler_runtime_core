/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
          The example below illustrates override/overload-compatibility with Object.
          If signatures are override-compatible (see Override-Compatible
          Signatures), then overriding is used. Otherwise, overloading is
          used.
tags: [compile-only]
---*/

enum E1 {E1, E2}

interface Base {
    foo<T extends Derived, U extends Base>(i: {{c.base}})
}

interface Derived extends Base {
    foo<T extends Derived, U extends Base>(i: {{c.derived}})
}

{% endfor %}
