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
desc: Bridge methods for generic overrides - Tuple return with same type parameter repeated [T, T]
---*/

class A<T> {
  foo(v: T): [T, T] {
    return [v, v];
  }
}

class B extends A<string> {
  override foo(v: string): [string, string] {
    return [v.toUpperCase(), v.toLowerCase()];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleReturnSameParam');

  suite.addTest('Same param repeated via parent ref', () => {
    let a: A<string> = new B();
    let r = a.foo('Hi');
    arktest.assertEQ(r[0], 'HI');
    arktest.assertEQ(r[1], 'hi');
  });

  return suite.run();
}
