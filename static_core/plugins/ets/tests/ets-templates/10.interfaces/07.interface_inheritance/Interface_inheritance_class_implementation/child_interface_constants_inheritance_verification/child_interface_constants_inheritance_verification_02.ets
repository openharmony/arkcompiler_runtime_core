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
desc: Verify a child class can correctly access constants from multiple implemented interfaces.
---*/

interface ParentInterface2 {
  readonly CONSTANT1: string;
}

interface ParentInterface3 {
  readonly CONSTANT2: boolean;
}

class ChildClass2 implements ParentInterface2, ParentInterface3 {
  readonly CONSTANT1: string = 'test';
  readonly CONSTANT2: boolean = true;
}

function main(): void {
  const instance = new ChildClass2();
  assertEQ(instance.CONSTANT1, 'test');
  assertEQ(typeof instance.CONSTANT1, 'string');
  assertEQ(instance.CONSTANT2, true);
  assertEQ(typeof instance.CONSTANT2, 'boolean');
}