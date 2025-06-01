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
desc: Verify that a child class can access an array constant from a single interface.
---*/

interface ArrayConstantInterface {
  readonly CONST: number[];
}

class ArrayConstantChild implements ArrayConstantInterface {
  readonly CONST: number[] = [1, 2, 3];
}

function main(): void {
  const instance = new ArrayConstantChild();
  assertEQ(instance.CONST.length, 3);
  assertEQ(instance.CONST[0], 1);
}