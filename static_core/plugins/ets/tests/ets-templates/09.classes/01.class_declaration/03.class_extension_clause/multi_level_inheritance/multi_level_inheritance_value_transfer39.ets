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
desc: Used to verify the concept of class inheritance in object-oriented programming and different access levels such as protected and public.
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
    assertEQ(this._publicVar, 3);
    assertEQ(this._protectedVar, 2);
  }
}

function main(): void {
  let test: C = new C();
  test.test();
}