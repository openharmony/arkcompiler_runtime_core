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
desc: The subclass initializes member variables in the constructor, with the modifier being public and the type being string.
---*/

class Parent {
  public constructorPublicVar: string;

  constructor(initialValue: string) {
    this.constructorPublicVar = initialValue;
  }
}

class Child extends Parent {
  constructor() {
    super('abc');
  }
}

function main(): void {
  let child = new Child();
  let x: string = child.constructorPublicVar;
  assertEQ(x, 'abc');
}