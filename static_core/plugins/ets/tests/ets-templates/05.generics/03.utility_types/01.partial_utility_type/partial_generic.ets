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
desc: Using Partial Utility type as a parameter for generic class
---*/

interface I {
  i: number;
}

class A implements I {
  j: string;
}

type Z = Partial<A>

class B <T> { }

function foo(i: Partial<A>) {
  let a: int = 0;
  if(i.i != null) {
    a += 1;
  }
  if(i.j != null) {
    a += 2;
  }
}

function main() {
  let f: B<Z> = {}
  f = {i: 2}
  f = {j: "some str"}
  f = {i: 2, j: "some str"}
  let g: B<Partial<A>> = {}
  g = {i: 2}
  g = {j: "some str"}
  g = {i: 2, j: "some str"}
}
