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
desc: Bridge methods for generic overrides - Multi-level inheritance with tuple return type
---*/

class A<T, U> {
  foo(a: T, b: U): [T, U] {
    return [a, b];
  }
}

class B<T> extends A<T, number> {
  override foo(a: T, b: number): [T, number] {
    return super.foo(a, b * 2);
  }
}

class C extends B<string> {
  override foo(a: string, b: number): [string, number] {
    return [a.toUpperCase(), b + 100];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleReturnMultiLevel');

  suite.addTest('Grandparent ref to grandchild', () => {
    let a: A<string, number> = new C();
    let r = a.foo('hi', 5);
    arktest.assertEQ(r[0], 'HI');
    arktest.assertEQ(r[1], 105);
  });

  suite.addTest('Parent ref to grandchild', () => {
    let b: B<string> = new C();
    let r = b.foo('xx', 3);
    arktest.assertEQ(r[0], 'XX');
    arktest.assertEQ(r[1], 103);
  });

  return suite.run();
}
