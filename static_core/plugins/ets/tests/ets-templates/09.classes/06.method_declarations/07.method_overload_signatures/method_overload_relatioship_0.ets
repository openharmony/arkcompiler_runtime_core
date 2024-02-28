/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Overload two methods with different return types.
assert: There is no required relationship between the return types or between the throws or rethrows clauses of two methods with the same name, unless their signatures are override-equivalent.
---*/

class A {
  public foo(): int {
    return 1;
  }

  public foo(a: int): boolean {
    return true;
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo() == 1;
  assert instance.foo(2) == true;
}
