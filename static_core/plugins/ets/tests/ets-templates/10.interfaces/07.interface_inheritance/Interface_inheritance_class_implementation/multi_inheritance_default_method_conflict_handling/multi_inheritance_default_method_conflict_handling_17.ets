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
desc: Overriding the default method(null type).
---*/

interface IBase1 {
  method(): null;
}

interface IBase2 {
  method(): null;
}

class Base1 implements IBase1 {
  method(): null {
    return null;
  }
}

class Base2 implements IBase2 {
  method(): null {
    return null;
  }
}

class Derived extends Base1 implements IBase2 {
  method(): null {
    return null;
  }
}

function main(): void {
  const test = new Derived();
  const result = test.method();
  assertEQ(result, null);
}