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
desc: Verify that a child class can access a Float32Array constant from an interface.
---*/

interface Float32ArrayConstantInterface {
  readonly CONST: Float32Array;
}

let check_value: Float32Array = new Float32Array([0.1, 0.2]);

class Float32ArrayConstantChild implements Float32ArrayConstantInterface {
  readonly CONST: Float32Array = check_value;
}

function main(): void {
  const instance = new Float32ArrayConstantChild();
  assertEQ(instance.CONST, check_value);
}