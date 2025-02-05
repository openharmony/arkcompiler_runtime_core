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
type TPL = [boolean, string];
class Gen<T> {}

function foo<T>(i: Gen<T>): void {
  res = 1;
}

function foo(i: Number|undefined|null): void {
  res = 2;
}

function foo(i: TPL): void {
  res = 3;
}
function foo(i: Char[]): void {
  res = 4;
}

function main(): void {
  foo(new Gen<Number>());
  assertEQ(res, 1);
  foo(undefined);
  assertEQ(res, 2);
  foo(null);
  assertEQ(res, 2);
  let a: TPL = [true, "some str"];
  foo(a);
  assertEQ(res, 3);
  foo(new Char[2]);
  assertEQ(res, 4);
}
