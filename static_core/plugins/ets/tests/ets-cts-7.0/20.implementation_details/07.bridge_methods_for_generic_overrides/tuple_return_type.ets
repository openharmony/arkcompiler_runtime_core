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
desc: Bridge methods for generic overrides - Tuple return type with multiple generic parameters
---*/

class Parent<T, U> {
  process(a: T, b: U): [T, U] {
    return [a, b];
  }
}

class Child extends Parent<string, number> {
  override process(a: string, b: number): [string, number] {
    return [a.toUpperCase(), b * 2];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite("BridgeMethodsTupleReturn");

  suite.addTest("Tuple return with two generic parameters", () => {
    let child: Parent<string, number> = new Child();
    let result = child.process("hi", 5);
    arktest.assertEQ(result[0], "HI");
    arktest.assertEQ(result[1], 10);
  });

  return suite.run();
}
