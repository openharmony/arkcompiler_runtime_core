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
desc: Bridge methods for generic overrides - Tuple return type from generic interface implementation
---*/

interface I<T, U> {
  foo(a: T, b: U): [T, U];
}

class A implements I<string, number> {
  foo(a: string, b: number): [string, number] {
    return [a.repeat(2), b + 1];
  }
}

class B implements I<boolean, string> {
  foo(a: boolean, b: string): [boolean, string] {
    return [!a, b.toUpperCase()];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleReturnInterface');

  suite.addTest('Interface ref - string/number', () => {
    let i: I<string, number> = new A();
    let r = i.foo('ab', 9);
    arktest.assertEQ(r[0], 'abab');
    arktest.assertEQ(r[1], 10);
  });

  suite.addTest('Interface ref - boolean/string', () => {
    let i: I<boolean, string> = new B();
    let r = i.foo(true, 'ok');
    arktest.assertEQ(r[0], false);
    arktest.assertEQ(r[1], 'OK');
  });

  return suite.run();
}
