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
desc: Test whether the constant name is correct when the subclass interface inherits the parent class interface constant.
---*/

interface ParentInterface {
  readonly PARENT_ANT: string;
}

interface ChildInterface extends ParentInterface {
  readonly CHILD_ANT: string;
}

class ChildClass implements ChildInterface {
  readonly PARENT_ANT = "Parent ant Value";
  readonly CHILD_ANT = "Child ant Value";
}

function main(): void {
  let childInstance = new ChildClass();
  let tmp1 = childInstance.CHILD_ANT;
  let tmp2 = childInstance.PARENT_ANT;
  assertEQ(tmp1, "Child ant Value");
  assertEQ(tmp2, "Parent ant Value");
}