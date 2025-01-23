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
desc: >-
    If two functions declared in the same declaration space
    have the same name but different signatures that are not
    override-equivalent, then the function name is said to be overloaded.
params: {{c.type}}
---*/

interface I {}

class A {}
class B extends A implements I {}
class C extends B {}

function foo(a: A): int {
  return 1;
}
function foo(a: B): int {
  return 2;
}
function foo(a: I): int {
  return 3;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  if (foo(v) == {{c.res}}) return 0;
  return 1;
}
{% endfor %}
