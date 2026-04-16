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
desc: Bridge methods for generic overrides - Getter with tuple return type
---*/

class A<T, U> {
  private v1: T;
  private v2: U;
  constructor(a: T, b: U) {
    this.v1 = a;
    this.v2 = b;
  }
  get pair(): [T, U] {
    return [this.v1, this.v2];
  }
}

class B extends A<string, number> {
  constructor(a: string, b: number) {
    super(a, b);
  }
  override get pair(): [string, number] {
    let p = super.pair;
    return [p[0].toUpperCase(), p[1] * 10];
  }
}

function main(): int {
  let suite = new arktest.ArkTestsuite('TupleReturnGetter');

  suite.addTest('Getter via parent ref', () => {
    let a: A<string, number> = new B('go', 4);
    let r = a.pair;
    arktest.assertEQ(r[0], 'GO');
    arktest.assertEQ(r[1], 40);
  });

  return suite.run();
}
