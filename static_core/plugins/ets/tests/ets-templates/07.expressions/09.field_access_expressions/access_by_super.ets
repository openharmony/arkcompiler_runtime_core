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
desc: Access to field by keyword super.
assert: A field access expression may access a field of an object which is referenced by the value of either an expression or the special keyword super.
---*/

class A {
  private _x: int;

  get x():int {
    return this._x;
  }

}

final class B extends A {
  public bar(): int {
    return super.x;
  }
}

function main(): void {
    let instance: B = new B();

    assertEQ( instance.bar(), 0 );
}
