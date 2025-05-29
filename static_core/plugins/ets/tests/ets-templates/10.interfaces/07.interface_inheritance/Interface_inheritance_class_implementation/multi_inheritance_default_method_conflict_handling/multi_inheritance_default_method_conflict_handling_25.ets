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
desc: Use static methods to avoid conflicts(number type).
---*/

interface IBase1 {
  method(): number;
}

interface IBase2 {
  method(): number;
}

class Base1 {
  static method() {
    return 1;
  }
}

class Base2 {
  static method() {
    return 2;
  }
}

class Derived implements IBase1, IBase2 {
  method(): number {
    return 3;
  }
}

function main(): void {
  const test = new Derived();
  const result = test.method();
  assertEQ(result, 3);
}