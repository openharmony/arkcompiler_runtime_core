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
desc: >-
    A field in a derived class can override a superclass field of the same type and
    simultaneously implement an interface property of that type.
---*/

interface I {
  f: {{c.check_type}}
}

class Base {
  f: {{c.check_type}} = {{c.base_value}}
}

class Derived extends Base implements I {
  f: {{c.check_type}} = {{c.derived_value}}
}

function getProp(i: I): {{c.check_type}} {
  return i.f
}

function main(): void {
  let d: Derived = new Derived()
  // derived field value is used, not the superclass value
  arktest.assertEQ(d.f, {{c.derived_value}})
  arktest.assertEQ(getProp(d), {{c.derived_value}})
}

{% endfor %}
