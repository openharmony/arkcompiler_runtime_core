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
desc: Null interfaces and class static methods.
---*/

interface ParentInterface {
  id: number;
}

interface EmptyInterface {}

class MyClass implements ParentInterface, EmptyInterface {
  id: number;

  constructor(id: number) {
    this.id = id;
  }

  static createInstance(id: number): MyClass {
    return new MyClass(id);
  }
}

function main(): void {
  const instance = MyClass.createInstance(100);  
  assertEQ(instance.id, 100);
}