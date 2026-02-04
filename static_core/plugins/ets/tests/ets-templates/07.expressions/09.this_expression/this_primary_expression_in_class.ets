/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: This reference on object.
assert: The keyword this used as a primary expression denotes a value that is a reference to the object for which the instance method was called or to the object being constructed.
---*/

class A {
  public x: int;

  public foo(): A {
    return bar(this);
  }
}

function bar(a: A): A {
  return a;
}

function main(): int {
  let a = new A();
  a.x = 10;
  let result = a.foo();
  arktest.assertEQ(result, a);
  arktest.assertEQ(result.x, 10);
  return 0;
}
