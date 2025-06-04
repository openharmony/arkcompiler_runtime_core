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
desc: An upper-layer interface has multiple methods. In multi-layer inheritance,
      the sub-interface inherits the methods of all upper-layer interfaces.
---*/

interface GrandParentInterface {
  grandparentMethod(): string {
    return "grandparentMethod";
  }

  grandparentMethod_1(): number {
    return 1;
  }
}

interface ParentInterface extends GrandParentInterface {
  parentMethod(): string {
    return "parentMethod";
  }

  parentMethod_1(): number {
    return 2;
  }
}

interface ChildInterface extends ParentInterface {

}

class ConcreteClass implements ChildInterface {

}

function main(): void {
  const obj: ChildInterface = new ConcreteClass();

  let tmp1: string = obj.grandparentMethod();
  assertEQ(tmp1, "grandparentMethod");

  let tmp2: string = obj.parentMethod();
  assertEQ(tmp2, "parentMethod");

  let tmp3: number = obj.grandparentMethod_1();
  assertEQ(tmp3, 1);

  let tmp4: number = obj.parentMethod_1();
  assertEQ(tmp4, 2);
}