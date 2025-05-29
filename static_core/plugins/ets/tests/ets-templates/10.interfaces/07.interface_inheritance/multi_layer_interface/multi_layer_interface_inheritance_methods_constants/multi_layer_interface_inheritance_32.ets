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
desc: In multi-layer inheritance, the sub-interface inherits the methods of
      all upper-layer interfaces. The return data type of the method is enumerated.
---*/

enum ColorSet {
  Red = 0,
  Green = 1,
  Blue = 2
}

interface GrandParentInterface {
  grandparentMethod(): ColorSet {
    return ColorSet.Red;
  }
}

interface ParentInterface extends GrandParentInterface {
  parentMethod(): ColorSet {
    return ColorSet.Green;
  }
}

interface ChildInterface extends ParentInterface {

}

class ConcreteClass implements ChildInterface {

}

function main(): void {
  const obj: ChildInterface = new ConcreteClass();

  let tmp1: ColorSet = obj.grandparentMethod();
  assertEQ(tmp1.valueOf(), 0);

  let tmp2: ColorSet = obj.parentMethod();
  assertEQ(tmp2.valueOf(), 1);
}