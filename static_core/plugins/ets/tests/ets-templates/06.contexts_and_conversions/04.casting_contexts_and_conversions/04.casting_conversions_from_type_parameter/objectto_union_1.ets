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
desc: Convert to obj-union type.
---*/

class Cat {
  name: string = 'cat';
}

class Dog {
  name: string = 'dog';
}

class Frog {
  name: string = 'frog';
}

type Animal = Cat | Dog | Frog | number;
let animal: Animal = 5;

function main(): void {
  let obj1: object = animal;
  assertTrue(obj1 == animal);
}