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
desc: Change the protected property _public_var inherited from class a to'aa' in class d, and verify that the property value is'aa' in the test method of class e.
---*/

class A {
  protected _publicVar: string;

  constructor() {
    this._publicVar = 'Multi-level inheritance';
  }
}

class B extends A {

}

class C extends B {

}

class D extends C {
  constructor() {
    this._publicVar = 'aaa';
  }
}

class E extends D {
  test() {
    assertEQ(this._publicVar, 'aaa');
  }
}

function main(): void {
  let test: E = new E;
  test.test();
}