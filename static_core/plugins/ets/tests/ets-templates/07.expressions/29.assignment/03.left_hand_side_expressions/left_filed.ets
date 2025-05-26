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
desc: Left-hand expression testing - field access testing.
---*/

class MyClass {
  myField: number;
   constructor() {
    this.myField = 0;
  }
  set mySetter(value: number) {
    this.myField = value;
  }
}

function main(): void {
  const myObject = new MyClass();
  myObject.myField = 20;
  assertEQ(myObject.myField = 20, 20);
}