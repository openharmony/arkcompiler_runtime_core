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
desc: Multi-level inheritance verifies that the public member variable _publicVar in class b is correctly initialized to a specific string value in subclass d.
---*/

class A {
}

class B extends A {
  public _publicVar: string;

  constructor() {
    this._publicVar = 'Multi-level inheritance';
  }
}

class C extends B {

}

class D extends C {
  test() {
  }
}

function main(): void {
  let test: D = new D();
  assertEQ(test._publicVar, 'Multi-level inheritance');
}