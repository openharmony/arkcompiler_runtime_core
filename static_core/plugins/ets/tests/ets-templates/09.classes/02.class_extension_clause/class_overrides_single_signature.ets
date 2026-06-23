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
   Class overrides one signature, leaving the default for the other.
---*/

interface I1 { 
    m(s: string): string { 
        return "I1" 
    }
} 

interface I2 { 
    m(n: int): string { 
        return "I2" 
    }
} 

class C implements I1, I2 { 
    m(s: string): string {
        return "C.str" 
    }
} 

function main(){
    let i1: I1 = new C() 
    let i2: I2 = new C() 
    arktest.assertEQ(i1.m("x"), "C.str", "via I1")
    arktest.assertEQ(i2.m(42), "I2", "via I2")
    let c: C = new C()
    arktest.assertEQ(c.m("x"), "C.str", "via C - string")
    arktest.assertEQ(c.m(42), "I2", "via C - int")
}
