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
desc: To verify that the parent class member variable (type boolean[]) is correctly initialized.
---*/

class Parent {
  public value: boolean[];

  constructor(value: boolean[]) {
    this.value = value;
  }
}

class Child extends Parent {
  constructor(value: boolean[]) {
    super(value);
  }
}

function main(): void {
  const myChild = new Child([true, false, true]);
  assertEQ(myChild.value[0], true);
  assertEQ(myChild.value[1], false);
  assertEQ(myChild.value[2], true);
}