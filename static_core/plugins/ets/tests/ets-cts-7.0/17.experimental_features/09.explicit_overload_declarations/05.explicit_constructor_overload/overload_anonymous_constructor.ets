/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: If a class has an anonymous constructor, then it is implicitly placed 
      at the first position in the list of the overloaded constructors.
---*/

let f = 10

class C {
    constructor() { f = 0 }
    constructor fromString(s: string) { f = 1 }

    overload constructor { fromString }
}

function main() {
    new C()
    arktest.assertEQ(f, 0, "anonymous constructor is called")

    new C("abc")
    arktest.assertEQ(f, 1, "constructor fromString is called")
}
