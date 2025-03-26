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
desc: Class overrides multiple default methods inherited by the subinterface, The default method returns the data type number.
---*/

interface Parent{
  speak(): number {
    return 123;
  }

  move(): number {
    return 456;
  }
}

interface Child extends Parent{}

class parentClass implements Child {
  speak(): number {
    return 456;
  }

  move(): number {
    return 123;
  }
}

function main(): void {
  const parent = new parentClass();
  assertEQ(parent.speak(), 456);
  assertEQ(parent.move(), 123);
}