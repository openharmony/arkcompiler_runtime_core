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
desc: Overload two methods, two inherited.
assert: If two methods of a class (whether both declared in the same class, or both inherited by a class, or one declared and one inherited) have the same name but signatures that are not override-equivalent, then the method name is said to be overloaded. This fact causes no difficulty and never of itself results in a compile-time error.
---*/

class A {
  public foo(): int {
    return 1;
  }

  public foo(a: int): int {
    return a;
  }
}

class B extends A{

}

function main(): void {
  let instance: B = new B();

  assertEQ( instance.foo(), 1 );
  assertEQ( instance.foo(2), 2 );
}
