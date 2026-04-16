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
desc: Bridge methods for generic overrides - Tuple as both parameter and return type
---*/

class A<T, U> {
  bar(input: [T, U]): [U, T] {
    return [input[1], input[0]];
  }
}

class B extends A<string, number> {
  override bar(input: [string, number]): [number, string] {
    return [input[1] + 100, input[0] + '!'];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleParamAndReturn');

  suite.addTest('Tuple param and return via parent ref', () => {
    let a: A<string, number> = new B();
    let r = a.bar(['hi', 5]);
    arktest.assertEQ(r[0], 105);
    arktest.assertEQ(r[1], 'hi!');
  });

  return suite.run();
}
