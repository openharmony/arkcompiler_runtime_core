/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{% for c in cases %}

/*---
desc: Test of invariant no type parameter modifier. 
---*/

class A<T> {
  c: T
  constructor(p: T) { this.c = p }
  f(p: T): T { return p }
  f1(p: T): string { return "v = " + p }
  f2(): T { return this.c }
}

function main(): void {
  let arg: {{c.type}} = {{c.value}}
  let a: A<{{c.type}}> = new A<{{c.type}}>(arg)
  assertEQ(a.f(arg), {{c.value}})
  assertEQ(a.f1(arg), {{c.value1}})
  assertEQ(a.f2(), {{c.value}})
}

{% endfor %}
