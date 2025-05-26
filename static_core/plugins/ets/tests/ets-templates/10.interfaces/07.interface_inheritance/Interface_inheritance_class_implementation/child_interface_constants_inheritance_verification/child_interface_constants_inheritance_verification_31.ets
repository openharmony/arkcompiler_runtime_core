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
desc: Verify that a child class can access an Int8Array constant from an interface.
---*/

interface Int8ArrayConstantInterface {
  readonly CONST: Int8Array;
}

class Int8ArrayConstantChild implements Int8ArrayConstantInterface {
  readonly CONST: Int8Array = new Int8Array([-128, 0, 127]);
}

function main(): void {
  const instance = new Int8ArrayConstantChild();
  assertEQ(instance.CONST[0], -128);
  assertEQ(instance.CONST[1], 0);
  assertEQ(instance.CONST[2], 127);
}