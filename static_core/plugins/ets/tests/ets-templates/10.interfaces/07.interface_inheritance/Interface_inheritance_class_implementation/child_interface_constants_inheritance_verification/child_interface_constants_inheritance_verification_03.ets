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
desc: Verify a child class can access constants from multi - level inherited interfaces.
---*/

interface GrandParentInterface {
  readonly GRAND_CONSTANT: number;
}

interface ParentInterface4 extends GrandParentInterface {
  readonly PARENT_CONSTANT: string;
}

class ChildClass3 implements ParentInterface4 {
  readonly GRAND_CONSTANT: number = 200;
  readonly PARENT_CONSTANT: string = 'inheritance';
}

function main(): void {
  const instance = new ChildClass3();

  assertEQ(instance.GRAND_CONSTANT, 200);
  assertEQ(typeof instance.GRAND_CONSTANT, 'number');
  assertEQ(instance.PARENT_CONSTANT, 'inheritance');
  assertEQ(typeof instance.PARENT_CONSTANT, 'string');
}