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
desc: Test whether the instance of class e can obtain the overridden value of the variable _public_var inherited from class a by overriding the multi-layer inheritance class d.
---*/

class A {
  public _publicVar: boolean;
  constructor() {
    this._publicVar =  true;
  }
}

class B extends A {

}

class C extends B {

}

class D extends C {
  constructor() {
    this._publicVar = false;
  }
}

class E extends D {

}

function main(): void {
  let test: E = new E();
  assertEQ(test._publicVar, false);
}
