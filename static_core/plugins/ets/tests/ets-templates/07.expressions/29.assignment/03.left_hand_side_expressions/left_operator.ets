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
desc: Left-hand expression testing - the chaining operator. In non-left-hand side scenarios.
---*/

class PropClass {
  value: number = 0;
  constructor(value: number) {
    this.value = value;
  }
}

class MyClass {
  prop?: PropClass;
}

function main(): void {
  const objNormal = new MyClass();
  objNormal.prop = new PropClass(0);
  const resultNormal = objNormal?.prop?.value;
  assertEQ(resultNormal, 0);
}