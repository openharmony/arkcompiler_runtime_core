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
desc: The child class calls the chained method of the parent class.
---*/

class Parent {
  public step1(): string {
    return "Step 1 from Parent";
  }

  public step2(): string {
    return this.step1() + " -> Step 2 from Parent";
  }
}

class Child extends Parent {
  public callParentChain(): string {
    return this.step2();
  }
}

function main(): void {
  const child = new Child();
  assertEQ(child.callParentChain(), "Step 1 from Parent -> Step 2 from Parent");
}