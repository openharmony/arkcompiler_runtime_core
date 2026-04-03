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
desc: Explicit overload for constructors is used in the same manner as a normal constructor is.
---*/

let f = 0

class BigFloat {
    constructor fromNumber(n: number) { f = 1 }
    constructor fromString(s: string) { f = 2 }

    overload constructor { fromNumber, fromString }
}

function main() {
    new BigFloat(1)
    arktest.assertEQ(f, 1, "first constructor is called")

    new BigFloat("3.14")
    arktest.assertEQ(f, 2, "second constructor is called")
}
