/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Classes, interfaces, and type aliases with type parameters are
    generic types. Named types without type parameters are non-generic types.
---*/

class NonGenericClass {
  x: int = 5
}

class GenericClass<T> {
  value: T
  constructor(v: T) {
    this.value = v
  }
}

interface NonGenericIface {
  id(): int
}

interface GenericIface<T> {
  get(): T
}

class ImplNonGeneric implements NonGenericIface {
  override id(): int { return 7 }
}

class ImplGeneric implements GenericIface<string> {
  override get(): string { return "abc" }
}

type NonGenericAlias = int

type GenericAlias<T> = T[]

function main(): int {
  let suite = new arktest.ArkTestsuite("Generic and Non-Generic Named Types");

  suite.addTest("Non-generic class is a named type", () => {
    let obj = new NonGenericClass()
    arktest.assertTrue(obj instanceof NonGenericClass)
    arktest.assertEQ(obj.x, 5)
  });

  suite.addTest("Generic class is a named type", () => {
    let obj = new GenericClass<int>(36)
    arktest.assertTrue(obj instanceof GenericClass)
    arktest.assertEQ(obj.value, 36)
  });

  suite.addTest("Non-generic interface is a named type", () => {
    let obj: NonGenericIface = new ImplNonGeneric()
    arktest.assertEQ(obj.id(), 7)
  });

  suite.addTest("Generic interface is a named type", () => {
    let obj: GenericIface<string> = new ImplGeneric()
    arktest.assertEQ(obj.get(), "abc")
  });

  suite.addTest("Non-generic type alias is a named type", () => {
    let v: NonGenericAlias = 15
    arktest.assertEQ(v, 15)
  });

  suite.addTest("Generic type alias is a named type", () => {
    let arr: GenericAlias<int> = [10, 20, 30]
    arktest.assertEQ(arr.length, 3)
    arktest.assertEQ(arr[0], 10)
    arktest.assertEQ(arr[2], 30)
  });

  return suite.run();
}
