/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Non-static field initialization with keyword 'super'.
assert: It there is an initializer in a non-static field declaration, then the following rules apply to the initializer. The initializer may refer to the current object using the keyword this or the keyword super.
---*/

class A {
  private _y: int = 5;

  public get y(): int{
    return this._y;
  }
}

class B extends A {
  public x: int = super.y
}

function main(): void {
    let instance: B = new B();

    assertEQ( instance.x, 5 );
}
