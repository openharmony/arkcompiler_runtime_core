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
desc: Subclasses accessing the initialization function of parent class members.
---*/

class Base {
  public value: int;
  constructor() {
    this.value = 1;
  }

  protected changeMembers(): void {
    this.value = 2;
  }
}

class Sub extends Base {
  constructor() {
    super();
  }

  public checkAccess(): void {
    this.changeMembers();
  }
}

function main(): void {
  const instanceB = new Sub();
  instanceB.checkAccess();
  assertEQ(instanceB.value, 2);
}