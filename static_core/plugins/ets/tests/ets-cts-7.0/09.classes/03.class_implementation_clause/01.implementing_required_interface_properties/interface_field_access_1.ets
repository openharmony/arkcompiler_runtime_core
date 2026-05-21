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
    Ex 3.1 Field write via class-ref. Implicit getter read via interface ref

---*/

interface I {
    n: number
}

class C implements I {
    n: number = 1
}

function main() {
    let c: C = new C();
    let i: I = c
    c.n = 10
    arktest.assertEQ(10, i.n)
}
