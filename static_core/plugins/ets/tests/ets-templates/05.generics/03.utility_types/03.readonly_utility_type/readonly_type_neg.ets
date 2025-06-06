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

{% for c in cases %}
{% for p in properties %}

/*---
desc: >-
    Type Readonly<T> constructs a type with all properties of T set to readonly.
    It means that the properties of the constructed value cannot be reassigned.
    Using union as T for Readonly<T>.
tags: [negative, compile-only]
---*/

interface I {
  i: number;
  j: string;
}

class A implements I {
  i = 0;
  j = "";
}

class C {
  i: number = 0;
  j: string = "";
}

function foo(i: Readonly<A|I|C>) {
  i.{{p.edit_prop}} = {{p.new_val}}
}

function main() {
  foo( {{c.argument}} )
}

{% endfor %}
{% endfor %}
