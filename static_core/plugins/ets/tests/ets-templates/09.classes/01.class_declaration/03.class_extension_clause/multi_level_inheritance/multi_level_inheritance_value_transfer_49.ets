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
desc: Verify that the protected member variable _public_var inherited from class a to class f is correctly initialized to the string'multi-level inheritance' after multi-level inheritance.
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

}

class E extends D {

}

class F extends E {
  test() {
    assertEQ(this._publicVar, 'Multi-level inheritance');
  }
}

function main(): void {
  let test: F = new F;
  test.test();
}