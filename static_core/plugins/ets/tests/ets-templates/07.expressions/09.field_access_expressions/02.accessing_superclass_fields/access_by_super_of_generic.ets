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
desc: Access to field by keyword super.
assert: The form T.super.identifier refers to the field named identifier of the lexically enclosing instance corresponding to T, but with that instance viewed as an instance of the superclass of T.
---*/

class A {
  public x: int;
}

class B extends A{
  public y: int;
}

final class C<T> extends A {
  public bar(): int {
    return T.super.x;
  }
}

function main(): void {
    let instance: C<B> = new C<B>();

    assert instance.bar() == 1;
}
