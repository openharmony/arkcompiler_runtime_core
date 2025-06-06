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
desc: Ensuring that method_one in subclasses does not allow contravariant parameter types with derived class.
tags: [compile-only, negative]
---*/

class Base {
  methodOne(p: Base): Base {
    return p;
  }
}

class Derived extends Base {
  override methodOne(p: Derived): Base {
    return p;
  }
}

function main(): void {
  let a = new Base();
  let b = new Derived();

  let resultA = a.methodOne(a);
  let resultB = b.methodOne(b);

  assertEQ(resultA, resultB);
}