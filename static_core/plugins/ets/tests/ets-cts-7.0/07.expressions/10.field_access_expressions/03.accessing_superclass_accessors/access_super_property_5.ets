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
    Ex. 2.5
    Class implements an interface property using explicit accessor. super.prop in a subclass denotes accessor not a field
---*/

interface I {
    n: number
}

abstract class A implements I {
    abstract get n(): number
    abstract set n(value: number)
}

class X extends A {
    private _n: number = 5;
    get n(): number { return this._n }
    set n(value: number) { this._n = value }
}

class Y extends X {
    foo(): number { return super.n }
}

function main() {
    arktest.assertEQ(5, new Y().foo())
}
