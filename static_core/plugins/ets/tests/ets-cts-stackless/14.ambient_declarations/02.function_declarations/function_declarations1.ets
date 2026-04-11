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

/*---
desc: Ambient Function declarations - implementation.
tags: [compile-only, not-a-test]
---*/

export function fun(x: int): string { return x.toString()}

export function foo(x: number, y: string, z: Object[]): double {
  return (x * y.length * z.length).toDouble();
}

export function bar<U, V, R>(x: U, y: V): R {
  return JSON.stringify([x, y]) as R
}

export function baz(y: string, x?: int): string {
  return x ? x.toString() : y;
}
