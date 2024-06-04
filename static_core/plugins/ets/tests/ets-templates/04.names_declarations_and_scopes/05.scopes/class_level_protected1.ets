/*---
Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: Class level scope, protected access
---*/

let x = -10

class C {
    protected x: int = 1;
}

class D extends C {
    public get():int {
        return this.x;
    }
    public put(p: int) {
        this.x = p;
    }
}

function main(): void {
    // access from D
    let d:D = new D()
    assert d.get() == 1
    d.put(42)
    assert d.get() == 42

    // module level scope is not affected
    assert x == -10
}
