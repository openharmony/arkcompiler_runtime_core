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
desc: >-
    Type references refer to named types by specifying their type names
    and (where applicable) type arguments to be substituted for the
    type parameters of a named type.
---*/

class A<T> {
  val: T
  constructor(v: T) { this.val = v }
}

class C<T> {
  field1: A<T>
  field2: A<number>
  constructor(f1: A<T>, f2: A<number>) {
    this.field1 = f1
    this.field2 = f2
  }
  foo(p: T): T { return p }
}

type MyType<T> = A<T>[]

function main(): int {
  let suite = new arktest.ArkTestsuite("Named Types - Type References");

  suite.addTest("Class type reference with type parameter", () => {
    let a: A<int> = new A<int>(37)
    arktest.assertEQ(a.val, 37)
  });

  suite.addTest("Class type reference with concrete type argument", () => {
    let a: A<number> = new A<number>(3.14)
    arktest.assertEQ(a.val, 3.14)
  });

  suite.addTest("Type reference in class fields", () => {
    let f1 = new A<string>("hello")
    let f2 = new A<number>(2.5)
    let c = new C<string>(f1, f2)
    arktest.assertEQ(c.field1.val, "hello")
    arktest.assertEQ(c.field2.val, 2.5)
  });

  suite.addTest("Type parameter as type reference in method", () => {
    let f1 = new A<string>("x")
    let f2 = new A<number>(1.0)
    let c = new C<string>(f1, f2)
    arktest.assertEQ(c.foo("result"), "result")
  });

  suite.addTest("Type alias reference with type argument", () => {
    let x: MyType<number> = [new A<number>(1.0), new A<number>(2.0)]
    arktest.assertEQ(x.length, 2)
    arktest.assertEQ(x[0].val, 1.0)
    arktest.assertEQ(x[1].val, 2.0)
  });

  return suite.run();
}
