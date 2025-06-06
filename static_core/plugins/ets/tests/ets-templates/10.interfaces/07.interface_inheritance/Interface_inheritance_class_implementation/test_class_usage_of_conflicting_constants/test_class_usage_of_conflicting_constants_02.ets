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
desc: Verify the handling of constant conflicts with type assertions.
---*/

interface InterfaceA {
  readonly CONFLICT_CONSTANT: number;
}

interface InterfaceB {
  readonly CONFLICT_CONSTANT: string;
}

class ClassA implements InterfaceA {
  readonly CONFLICT_CONSTANT: number = 123;
}

class ClassB implements InterfaceB {
  readonly CONFLICT_CONSTANT: string = "abc";
}

function main(): void {
  const instanceA: InterfaceA = new ClassA();
  const instanceB: InterfaceB = new ClassB();

  assertEQ(typeof instanceA.CONFLICT_CONSTANT, 'number');
  assertEQ(instanceA.CONFLICT_CONSTANT, 123);
  assertEQ(typeof instanceB.CONFLICT_CONSTANT, 'string');
  assertEQ(instanceB.CONFLICT_CONSTANT, "abc");
}