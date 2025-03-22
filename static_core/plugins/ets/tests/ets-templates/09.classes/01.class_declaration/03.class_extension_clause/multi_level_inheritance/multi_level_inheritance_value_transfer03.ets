/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Test whether class c can correctly obtain the initial value of the public variable _publicVar in class a after indirectly inheriting class a from class b.
---*/

class A {
  protected _protectedVar: number;
  public _publicVar: number;

  constructor() {
    this._protectedVar = 2;
    this._publicVar = 3;
  }
}

class B extends A {
  constructor() {
    super();
  }
}

class C extends B {
  constructor() {
    super();
  }

  test() {
  }
}

function main(): void {
  let test: C = new C();
  assertEQ(test._publicVar, 3);
}