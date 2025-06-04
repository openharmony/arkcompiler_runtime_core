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
          Signatures S1 with n parameters, and S2 with m parameters are overload-equivalent if:
          • n = m;
          • Parameter type at some position in S1 is a type parameter (see Type Parameters), and a parameter type at the
          same position in S2 is any reference type or type parameter;
          • Parameter type at some position in S1 is a generic type G <T1, ..., Tn>, and a parameter type at the same
          position in S2 is also G with any list of type arguments;
          • All other parameter types in S1 are equal to parameter types in the same positions in S2.
          Parameter names and return types do not influence overload-equivalence.
          The following signatures are overload-equivalent.
tags: [negative, compile-only]
---*/

let res: int = 0;

class A<T> {
  foo(x: T): void {
    res = 1;
  }

  foo(y: Number): number {
    res = 2;
    return y;
  }
}

function main(): void {
  new A<number>().foo(1);
  assertTrue(res == 2);
}
