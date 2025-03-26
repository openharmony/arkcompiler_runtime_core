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
desc: Verify that a child class can access constants of different types from multiple interfaces.
---*/

interface MultiInterface1 {
  readonly CONST1: number;
}

interface MultiInterface2 {
  readonly CONST2: string;
}

class MultiInterfaceChild implements MultiInterface1, MultiInterface2 {
  readonly CONST1: number = 2;
  readonly CONST2: string = "multi";
}

function main(): void {
  const instance = new MultiInterfaceChild();
  assertEQ(instance.CONST1, 2);
  assertEQ(instance.CONST2, "multi")
}