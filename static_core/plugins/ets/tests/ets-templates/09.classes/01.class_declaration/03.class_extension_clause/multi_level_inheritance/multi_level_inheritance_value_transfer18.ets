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
desc: Test whether class f can correctly obtain the initial null value of the public variable _publicVar in class a after inheriting from class a through multiple layers.
---*/

class A {
  public _publicVar: null;

  constructor() {
    this._publicVar = null;
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

}

function main(): void {
  let test: F = new F();
  assertEQ(test._publicVar, null);
}