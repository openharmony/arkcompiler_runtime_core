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
desc: Ex 4.3 Getter and setter from different interfaces

---*/

interface I1 {
    get n(): number
}
interface I2 {
    set n(value: number)
}

class X implements I1, I2 {
    private _n: number = 0;
    get n(): number {
        return this._n
    }
    set n(value: number) {
        this._n = value
    }
}

class Y extends X {
    foo(): number {
        return super.n;
    }
}

function main() {
    arktest.assertEQ(0, new Y().foo())
}
