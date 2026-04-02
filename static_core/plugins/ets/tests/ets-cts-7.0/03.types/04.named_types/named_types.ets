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
    Named types are classes, interfaces, enumerations, aliases,
    type parameters, and predefined types.
---*/

class MyClass {
  value: int = 10
}

interface MyInterface {
  greet(): string
}

class MyImpl implements MyInterface {
  override greet(): string {
    return "hello"
  }
}

enum Color { Red, Green, Blue }

type MyAlias = string

class Container<T> {
  item: T
  constructor(item: T) {
    this.item = item
  }
  getItem(): T {
    return this.item
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("Named Types");

  suite.addTest("Class declaration introduces a named type", () => {
    let obj = new MyClass()
    arktest.assertTrue(obj instanceof MyClass)
    arktest.assertEQ(obj.value, 10)
  });

  suite.addTest("Interface declaration introduces a named type", () => {
    let obj: MyInterface = new MyImpl()
    arktest.assertTrue(obj instanceof MyInterface)
    arktest.assertEQ(obj.greet(), "hello")
  });

  suite.addTest("Enumeration declaration introduces a named type", () => {
    let c: Color = Color.Green
    arktest.assertEQ(c, Color.Green)
    arktest.assertTrue(c != Color.Red)
  });

  suite.addTest("Type alias declaration introduces a named type", () => {
    let s: MyAlias = "test"
    arktest.assertEQ(s, "test")
    arktest.assertEQ(s.length, 4)
  });

  suite.addTest("Type parameter declaration introduces a named type", () => {
    let c = new Container<int>(35)
    arktest.assertEQ(c.getItem(), 35)
    let cs = new Container<string>("abc")
    arktest.assertEQ(cs.getItem(), "abc")
  });

  return suite.run();
}
