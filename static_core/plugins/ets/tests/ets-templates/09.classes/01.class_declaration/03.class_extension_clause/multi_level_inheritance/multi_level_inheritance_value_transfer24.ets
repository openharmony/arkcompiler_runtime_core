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
desc: Test whether class b can correctly access the public variable _publicVar defined in class a and obtain the value of its first element after inheriting class a.
---*/

class A {
  public _publicVar: number[];

  constructor() {
    this._publicVar = [1, 2, 3];
  }
}

class B extends A {
  constructor() {
    super();
  }

  test() {
  }
}

function main(): void {
  let test: B = new B();
  assertEQ(test._publicVar[0], 1);
}