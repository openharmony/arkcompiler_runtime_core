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
desc: Using generic interfaces to handle conflicts.
---*/

interface IBase<T> {
  method(): string;
}

interface IBase1 extends IBase<IBase1> {
  method(): string;
}

interface IBase2 extends IBase<IBase2> {
  method(): string;
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