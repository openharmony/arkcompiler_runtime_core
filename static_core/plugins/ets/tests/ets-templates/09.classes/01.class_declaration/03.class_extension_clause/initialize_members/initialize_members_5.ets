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
desc: The subclass initializes member variables in the constructor, with the modifier being public and the type being boolean[].
---*/

class Parent {
  public constructorPublicVar: boolean[];

  constructor(initialValue: boolean[]) {
    this.constructorPublicVar = initialValue;
  }
}

class Child extends Parent {
  constructor(childValue: boolean[]) {
    super(childValue);
  }
}

function main(): void {
  const y : boolean[] = [true, false, true];
  let child = new Child(y);
  let x: boolean[] = child.constructorPublicVar;
  assertEQ(x[0], true);
}