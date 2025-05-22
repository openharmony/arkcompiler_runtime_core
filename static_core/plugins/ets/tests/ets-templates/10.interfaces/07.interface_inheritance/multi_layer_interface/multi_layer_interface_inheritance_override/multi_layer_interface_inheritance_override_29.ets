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
desc: In multi-layer inheritance, sub-interfaces cannot overwrite all the constants of upper-layer interfaces.
      The data type is bigint.
---*/

interface GrandParentInterface {
  readonly GRANDPARENT_CONST: bigint;
}

interface ParentInterface extends GrandParentInterface {
  readonly PARENT_CONST: bigint;
}

interface ChildInterface extends ParentInterface {
  readonly GRANDPARENT_CONST: bigint;
  readonly PARENT_CONST: bigint;
}

class ConcreteClass implements ChildInterface {
  readonly GRANDPARENT_CONST: bigint = 100n;
  readonly PARENT_CONST: bigint = 20n;
}

function main(): void {
  const obj: ChildInterface = new ConcreteClass();
  assertEQ(obj.GRANDPARENT_CONST, 100n);
  assertEQ(obj.PARENT_CONST, 20n);
}