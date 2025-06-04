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
desc: Verify that a child class can access a Uint16Array constant from an interface.
---*/

interface Uint16ArrayConstantInterface {
  readonly CONST: Uint16Array;
}

class Uint16ArrayConstantChild implements Uint16ArrayConstantInterface {
  readonly CONST: Uint16Array = new Uint16Array([0, 65535]);
}

function main(): void {
  const instance = new Uint16ArrayConstantChild();
  assertEQ(instance.CONST[0], 0);
  assertEQ(instance.CONST[1], 65535);
}