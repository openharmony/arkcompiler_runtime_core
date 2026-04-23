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
desc: Bridge methods for generic overrides - Tuple return with three generic parameters [T, U, V]
---*/

class A<T, U, V> {
  foo(a: T, b: U, c: V): [T, U, V] {
    return [a, b, c];
  }
}

class B extends A<string, number, boolean> {
  override foo(a: string, b: number, c: boolean): [string, number, boolean] {
    return [a + '!', b * 2, !c];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleReturnThreeParams');

  suite.addTest('Three params via parent ref', () => {
    let a: A<string, number, boolean> = new B();
    let r = a.foo('go', 3, true);
    arktest.assertEQ(r[0], 'go!');
    arktest.assertEQ(r[1], 6);
    arktest.assertEQ(r[2], false);
  });

  return suite.run();
}
