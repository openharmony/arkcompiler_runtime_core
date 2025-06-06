/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Class overrides abstract methods from superinterface.
assert: Unless the class being declared is abstract, all the abstract member methods of each direct superinterface must be implemented either by a declaration in this class or by an existing method declaration inherited from the direct superclass or a direct superinterface, because a class that is not abstract is not permitted to have abstract methods.
---*/

final class A implements C, B {
  override foo(): int {
    return 5;
  }

  override bar(): int {
    return 10;
  }
}

interface B {
  bar(): int;
}

interface C {
  foo(): int;
}

function main(): void {
  let c: A = new A();
  let b: A = new A();

  assertEQ( c.foo(), 5 );
  assertEQ( b.bar(), 10 );
}
