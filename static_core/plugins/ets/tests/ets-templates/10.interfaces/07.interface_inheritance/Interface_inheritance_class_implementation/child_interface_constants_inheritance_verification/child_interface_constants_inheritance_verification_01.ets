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
desc: Verify a child class can directly access a constant from a single implemented interface.
---*/

interface ParentInterface1 {
  VALUE: number;
}

class ChildClass1 implements ParentInterface1 {
  VALUE: number = 100;
}

function main(): void {
  const instance = new ChildClass1();
  assertEQ(instance.VALUE, 100);
  assertEQ(typeof instance.VALUE, 'number');
}