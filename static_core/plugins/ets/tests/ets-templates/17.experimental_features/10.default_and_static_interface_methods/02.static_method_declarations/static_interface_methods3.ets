/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Static interface method calls refer to no particular object.
---*/

interface I {
    static inc(p: number): number {
        return p + 1
    }
}
type TI = I

interface J extends TI {
    static inc(p: number): number {
        return p + 2
    }
}
type TJ = J

class A implements TI {
    static inc(p: number): number {
        return p + 3
    }
}
type TA = A

class B implements TJ {
    static inc(p: number): number {
        return p + 4
    }
}
type TB = B

class C extends TA {}
type TC = C

class D implements TI {}
type TD = D

class F implements TJ {}
type TF = F

function main(): void {
    assert TA.inc(10) == 13
    assert TB.inc(10) == 14
    assert TC.inc(10) == 13
    assert TD.inc(10) == 11
    assert TF.inc(10) == 12

    assert TI.inc(20) == 21
    assert TJ.inc(20) == 22
}
