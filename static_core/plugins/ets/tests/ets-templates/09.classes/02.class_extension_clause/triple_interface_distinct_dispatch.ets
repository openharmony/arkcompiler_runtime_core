/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: >-
   Three interfaces, three different signatures. 
   Each dispatches to its own default.
---*/

interface I1 { 
    m(a: int): string { 
        return "int" 
    }
} 

interface I2 { 
    m(a: double): string { 
        return "double" 
    }
}

interface I3 { 
    m(a: string): string { 
        return "string" 
    }
} 

class C implements I1, I2, I3 {}

function main(): void {
    let i1: I1 = new C() 
    let i2: I2 = new C() 
    let i3: I3 = new C() 
    arktest.assertEQ(i1.m(1), "int", "via I1")
    arktest.assertEQ(i2.m(1.0), "double", "via I2")
    arktest.assertEQ(i3.m("x"), "string", "via I3")
    let c: C = new C()
    arktest.assertEQ(c.m(1), "int", "C via I1")
    arktest.assertEQ(c.m(1.0), "double", "C via I2")
    arktest.assertEQ(c.m("x"), "string", "C via I3")
}
