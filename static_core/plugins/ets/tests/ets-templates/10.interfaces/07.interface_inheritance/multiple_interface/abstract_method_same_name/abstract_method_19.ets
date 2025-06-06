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
desc: The method with the same name has the same type and number of parameters, but a method has a generic type.
---*/

interface Parent1 {
  process<T>(data: T): T;
}

interface Parent2 {
  process(data: string): string;
}

interface Child extends Parent1, Parent2 {
  process<T>(data: T): T;
  process(data: string): string;
}

class ChildClass implements Child {
  process<T>(data: T): T {
    return data;
  }

  process(data: string): string {
    return data;
  }
}

function main(): void {
  const child = new ChildClass();
  assertEQ(child.process("Hello"), "Hello");
  assertEQ(child.process(42), 42);
}