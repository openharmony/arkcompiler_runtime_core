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
desc: Make sure that the public member variable _publicVar is properly initialized in base class a and that the value remains unchanged in the lowest subclass c.
---*/

class A {
  public _publicVar: boolean;

  constructor() {
    this._publicVar = true;
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
    assertEQ(this._publicVar, true);
  }
}

function main(): void {
  let c: C = new C();
  c.test();
}