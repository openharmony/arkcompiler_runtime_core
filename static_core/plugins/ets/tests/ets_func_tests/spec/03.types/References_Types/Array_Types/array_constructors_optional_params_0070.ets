/*
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: 3.8.6 Array Types. Constructor of element of array has optional parameters
tags: []
---*/

class E<T> {
    ee: T | undefined;

    constructor(ee? : T) {
        this.ee = ee;
    }
}

class F<T> {
    ff: T | undefined;

    constructor(ff? : T) {
        this.ff = ff;
    }
}

class C {
    private e: E<number> | undefined;
    private f: F<number> | undefined;
    constructor(e?: E<number>, f?: F<number>) {
        this.e = e;
        this.f = f;
    }

    public getE() {return this.e}
    public getF() {return this.f}
}

function main() {
    let c  = new C[10]
    for (let i: int = 0; i < 10; i++) {
        assertEQ( c[i].getE(), undefined)
        assertEQ( c[i].getF(), undefined)
    }
}
