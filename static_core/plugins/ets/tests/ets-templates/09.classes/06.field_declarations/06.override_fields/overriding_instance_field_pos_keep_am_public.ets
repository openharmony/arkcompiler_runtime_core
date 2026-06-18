/*
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
*/

{% yaml "overriding_instance_field_pos_keep_am.params.yaml" as data %}

{% for c in data.cases %}

/*---
desc: >-
    A compile-time error occurs if: Access Modifiers of the overriding field
    and of the overridden field are different.
params: keep public access modifier
---*/

type UT = String | Boolean
type FT = () => number
enum Color { White, Black, Red, Blue, Yellow }
type RT = Record<string, number>

const f1: FT = () => 12
const f2: FT = () => 24

interface IPerson {
    name: String;
    surname: String;
    age: int
}

const bob: IPerson = {name: "Bob", surname: "Doe", age: 25}
const liz: IPerson = {name: "Liz", surname: "Doe", age: 16}

const r1: RT = { "John": 25, "Mary": 21 }
const r2: RT = { "Bob": 15, "Jane": 31 }

class Base {
  field: {{c.check_type}} = {{c.base_value}}
  get_field() {
    return this.field
  }
}

class A extends Base {
  field: {{c.check_type}} = {{c.override_value}}
}

function get_field(a: Base): {{c.check_type}} {
  return a.get_field()
}

function main(): void {
  let a: A = new A()
  let b: Base = new Base()
  arktest.assertTrue(a.field instanceof {{c.check_type}})
  arktest.assertEQ(a.field, {{c.override_value}})
  arktest.assertEQ(a.get_field(), {{c.override_value}})
  arktest.assertEQ(get_field(a), {{c.override_value}})
  arktest.assertTrue(b.field instanceof {{c.check_type}})
  arktest.assertEQ(b.field, {{c.base_value}})
  arktest.assertEQ(b.get_field(), {{c.base_value}})
  arktest.assertEQ(get_field(b), {{c.base_value}})
}

{% endfor %}
