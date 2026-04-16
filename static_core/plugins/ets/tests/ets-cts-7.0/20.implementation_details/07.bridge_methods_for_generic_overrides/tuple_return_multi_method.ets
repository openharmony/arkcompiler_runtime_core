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
desc: Bridge methods for generic overrides - Multiple methods with different tuple return types in one class
---*/

class A<T, U> {
  foo(a: T, b: U): [T, U] {
    return [a, b];
  }
  bar(a: T): [T, T] {
    return [a, a];
  }
  baz(b: U): [U, T] {
    return [b, this.a_default()];
  }
  a_default(): T {
    return null!;
  }
}

class B extends A<string, number> {
  override foo(a: string, b: number): [string, number] {
    return [a.toUpperCase(), b * 3];
  }
  override bar(a: string): [string, string] {
    return [a, a + a];
  }
  override baz(b: number): [number, string] {
    return [b, b.toString()];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleReturnMultiMethod');

  suite.addTest('foo via parent ref', () => {
    let a: A<string, number> = new B();
    let r = a.foo('hi', 2);
    arktest.assertEQ(r[0], 'HI');
    arktest.assertEQ(r[1], 6);
  });

  suite.addTest('bar via parent ref', () => {
    let a: A<string, number> = new B();
    let r = a.bar('ab');
    arktest.assertEQ(r[0], 'ab');
    arktest.assertEQ(r[1], 'abab');
  });

  suite.addTest('baz via parent ref', () => {
    let a: A<string, number> = new B();
    let r = a.baz(42);
    arktest.assertEQ(r[0], 42);
    arktest.assertEQ(r[1], '42');
  });

  return suite.run();
}
