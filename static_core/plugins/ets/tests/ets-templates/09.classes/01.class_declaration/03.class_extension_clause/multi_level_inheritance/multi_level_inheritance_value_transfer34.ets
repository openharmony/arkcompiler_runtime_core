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
desc: Modifiers enhance method functionality and ensure the correctness of member variables in the inheritance chain.
---*/

class A {
  protected _publicVar: boolean;

  constructor() {
    this._publicVar = true;
  }
}

class B extends A {
  constructor() {
    super();
  }

  test() {
    assertEQ(this._publicVar, true);
  }
}

function main(): void {
  let b: B = new B();
  b.test();
}