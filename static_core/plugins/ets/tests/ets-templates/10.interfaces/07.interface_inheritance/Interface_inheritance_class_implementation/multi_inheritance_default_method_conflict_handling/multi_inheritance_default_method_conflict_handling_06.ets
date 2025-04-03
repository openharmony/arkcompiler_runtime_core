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
desc: Use static methods to avoid conflicts.
---*/

interface IBase1 {
  method(): string;
}

interface IBase2 {
  method(): string;
}

class Base1 {
  static method(): string {
    return "Base1 method";
  }
}

class Base2 {
  static method(): string {
    return "Base2 method";
  }
}

class Derived implements IBase1, IBase2 {
  method(): string {
    return "Derived method";
  }
}

function main(): void {
  const test = new Derived();
  const result = test.method();
  assertEQ(result, "Derived method");
}