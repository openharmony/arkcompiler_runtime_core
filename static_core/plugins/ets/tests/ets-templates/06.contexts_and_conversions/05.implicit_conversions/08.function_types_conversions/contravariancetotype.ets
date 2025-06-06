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
desc: Casting conversions Contravariance to type.
tags: [compile-only, positive]
---*/

class Animal {
  name: string;
  constructor(name: string) {
    this.name = name;
  }
}

class Dog extends Animal {
  breed: string;
  constructor(name: string, breed: string) {
    super(name);
    this.breed = breed;
  }
}

function createDog(): Dog {
  return new Dog("Buddy", "Golden Retriever");
}

function main(): void {
  type AnimalCreator = () => Animal;
  let createAnimal: AnimalCreator = createDog;
  let animal: Animal = createAnimal();

  if (animal instanceof Dog) {
    console.log("Test passed! Function with covariant return type works as expected.");
  } else {
    console.error("Test failed! Expected an instance of Dog, but got something else.");
  }
}