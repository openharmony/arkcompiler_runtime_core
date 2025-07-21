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
desc: In multi-layer inheritance, the sub-interface inherits the constants of all upper-layer interfaces.
      The constant data type is enumerated.
---*/

enum ColorSet {
  Red = 0,
  Green = 1,
  Blue = 2
}

interface GrandParentInterface {
  readonly GRANDPARENT_CONST: ColorSet;
}

interface ParentInterface extends GrandParentInterface {
  readonly PARENT_CONST: ColorSet;
}

interface ChildInterface extends ParentInterface {

}

class ConcreteClass implements ChildInterface {
  readonly GRANDPARENT_CONST: ColorSet = ColorSet.Red;
  readonly PARENT_CONST: ColorSet = ColorSet.Green;
}

function main(): void {
  const obj: ChildInterface = new ConcreteClass();
  assertEQ(obj.GRANDPARENT_CONST.valueOf(), 0);
  assertEQ(obj.PARENT_CONST.valueOf(), 1);
}