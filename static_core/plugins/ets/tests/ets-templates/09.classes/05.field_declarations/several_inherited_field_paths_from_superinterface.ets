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
desc: Several paths by which the same field declaration is inherited from an interface.
assert: There might be several paths by which the same field declaration is inherited from an interface. In such asituation, the field is considered to be inherited only once, and it may be referred to by its simple name without ambiguity.
---*/

interface A {
  x: int;
}

class C implements A {
    x: int;
}

class B extends C implements A {
}

function main(): void {
    let instance: B = new B();

    assertEQ( instance.x, 0 );
}
