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
desc: Verify that a child class can access an Int32Array constant from an interface.
---*/

interface Int32ArrayConstantInterface {
  readonly CONST: Int32Array;
}

class Int32ArrayConstantChild implements Int32ArrayConstantInterface {
  readonly CONST: Int32Array = new Int32Array([-2147483648, 0, 2147483647]);
}

function main(): void {
  const instance = new Int32ArrayConstantChild();
  assertEQ(instance.CONST[0], -2147483648);
  assertEQ(instance.CONST[1], 0);
  assertEQ(instance.CONST[2], 2147483647);
}