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
desc: Refer to inherits field with the same name from its direct superclass and superinterfaces.
assert: However, any attempt within the body of the class to refer to any such field by its simple name will result in a compile-time error, because the reference is ambiguous.
tags: [negative, compile-only]
---*/

class A {
  x: int = 5;
}

interface C {
  x: int = 1;
}

interface D {
  x: int = 1;
}

class B extends A implements C, D{
  public increment(): void {
    this.x++;
  }
}
