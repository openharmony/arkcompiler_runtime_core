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
desc: Verify that a child class can access a tuple constant from an interface.
---*/

interface TupleConstantInterface {
  readonly CONST: [string, boolean];
}

class TupleConstantChild implements TupleConstantInterface {
  readonly CONST: [string, boolean] = ["hello", true];
}

function main(): void {
  const instance = new TupleConstantChild();
  assertEQ(instance.CONST[0], 'hello');
  assertEQ(instance.CONST[1], true);
}