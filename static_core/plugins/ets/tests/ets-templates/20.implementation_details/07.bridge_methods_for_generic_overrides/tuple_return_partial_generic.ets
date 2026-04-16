/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/*---
desc: Bridge methods for generic overrides - Tuple return with mixed generic and concrete types [T, string]
---*/

class A<T> {
  foo(v: T): [T, string] {
    return [v, ''];
  }
}

class B extends A<number> {
  override foo(v: number): [number, string] {
    return [v * 10, v.toString()];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleReturnPartialGeneric');

  suite.addTest('Partial generic via parent ref', () => {
    let a: A<number> = new B();
    let r = a.foo(7);
    arktest.assertEQ(r[0], 70);
    arktest.assertEQ(r[1], '7');
  });

  return suite.run();
}
