/*
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
*/

/*---
desc: >-
    Ex 3.4 calls implicit setter

---*/

interface I {
    n: number
}

class C implements I {
    n: number = 1
    bar() {
        (this as I).n = 99
    }
    foo(): number {
        return this.n
    }
}

function main() {
    let c = new C();
    c.bar()
    arktest.assertEQ(99, c.foo())
}
