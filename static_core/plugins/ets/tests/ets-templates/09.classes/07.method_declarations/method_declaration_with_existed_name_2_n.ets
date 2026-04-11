/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: >-
    Method declaration with existed name in class body declaration.
    It is a compile-time error for the body of a class declaration to declare a method with the name
    that is already used for field or a member type in this declaration.
tags: [compile-only, negative]
expected_out: >-
    Semantic error ESE0216: Class Derive cannot extend class Base
    because the overriding declaration type for method run is not compatible with the declaration type required by the base class.
---*/

class Base {
    private run: int;
}

final class Derive extends Base {
    public run(): void {}
}
