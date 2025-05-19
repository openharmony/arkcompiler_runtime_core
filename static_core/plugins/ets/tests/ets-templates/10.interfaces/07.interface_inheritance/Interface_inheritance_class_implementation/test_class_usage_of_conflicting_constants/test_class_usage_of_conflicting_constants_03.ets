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
desc: Verify that a class cannot directly implement two interfaces with the same - named constants due to conflicts, mainly focusing on type conflicts.
---*/

interface InterfaceOne {
  readonly SAME_NAME_CONSTANT: number;
}

interface InterfaceTwo {
  readonly SAME_NAME_CONSTANT: string;
}

class ClassForOne implements InterfaceOne {
  readonly SAME_NAME_CONSTANT: number = 123;
}

class ClassForTwo implements InterfaceTwo {
  readonly SAME_NAME_CONSTANT: string = 'abc';
}

function main() {
  const instanceOne = new ClassForOne();
  const instanceTwo = new ClassForTwo();
  assertEQ(typeof instanceOne.SAME_NAME_CONSTANT, 'number');
  assertEQ(typeof instanceTwo.SAME_NAME_CONSTANT, 'string');
}