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
desc: Static properties need to be defined directly in the class.
---*/

interface MyInterface {
  instanceProperty: string; 
}

class MyClass implements MyInterface {
  instanceProperty: string;

  constructor(value: string) {
    this.instanceProperty = value;
  }

  static staticProperty: string = "This is a static property.";
}

function main(): void {
  const instance = new MyClass("This is an instance property.");
  assertEQ(instance.instanceProperty, "This is an instance property.");
  assertEQ(MyClass.staticProperty, "This is a static property.");
}