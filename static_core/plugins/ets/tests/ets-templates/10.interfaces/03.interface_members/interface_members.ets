/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: The body of an interface may declare members of the interface, that is, fields and methods.
---*/

interface A {
  f: int
  of ?: int
  readonly rf: int
  m1(): int { return 5; }
  m2(): int
  m3(value?: number)
  m4(value: number| undefined)
  get i(): int
  get j(): int
  set j(f: int)
}

let res: number|undefined = 0

function main() {
  let i: A = { f: 1, rf: 2, i: 3, j: 4,
                m2() { return 6;},
                m3(value: number|undefined) { res = value },
                m4(value?: number) { res = value}
             }
  arktest.assertEQ(i.f, 1)
  arktest.assertEQ(i.of, undefined)
  arktest.assertEQ(i.rf, 2)
  arktest.assertEQ(i.i, 3)
  arktest.assertEQ(i.j, 4)
  arktest.assertEQ(i.m1(), 5)
  arktest.assertEQ(i.m2(), 6)
  i.m3(7)
  arktest.assertEQ(res, 7)
  i.m3()
  arktest.assertEQ(res, undefined)
  i.m4(8)
  arktest.assertEQ(res, 8)
  i.m3()
  arktest.assertEQ(res, undefined)
}
