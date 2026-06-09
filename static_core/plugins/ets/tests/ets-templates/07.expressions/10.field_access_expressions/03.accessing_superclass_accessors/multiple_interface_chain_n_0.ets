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
desc: Ex 4.1 Interaction of super access with properties from multiple interfaces
tags: [negative,compile-only]
expected_out: "Semantic error ESE0377: Class field 'n' defined by the parent class is not accessible in the child class via super."
---*/

interface I1 {
    n: number;
}
interface I2 {
    n: number;
}
class X implements I1, I2 {
    n: number = 1;
}

class Y extends X {
    foo(): number {
        return super.n;
    }
}
