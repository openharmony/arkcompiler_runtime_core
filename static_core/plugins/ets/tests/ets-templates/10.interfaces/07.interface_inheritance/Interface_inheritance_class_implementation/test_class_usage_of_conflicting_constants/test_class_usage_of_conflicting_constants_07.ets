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
desc: Verify that a class can handle the conflict of the same - named constants between an interface and a class property by overriding in a class method.
---*/

interface InterfacePropConflictInterface {
  readonly CONST: number;
}

class InterfacePropConflictClass {
  readonly CONST: number = 12;
}

class InterfacePropConflictChild extends InterfacePropConflictClass implements InterfacePropConflictInterface {
  get CONST(): number {
    return 13;
  }
}

function main(): void {
  const instance = new InterfacePropConflictChild();
  assertEQ(instance.CONST, 13);
}