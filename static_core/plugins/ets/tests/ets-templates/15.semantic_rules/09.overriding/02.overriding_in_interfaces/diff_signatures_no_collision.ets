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
   Different signatures -> no collision. 
   Each interface reference invokes its own default.
tags: []
---*/

interface I1 { 
    m(s: string): string { 
        return "I1:" + s 
    }
}

interface I2 { 
    m(n: int): string { 
        return "I2:" + n.toString() 
    }
} 

class C implements I1, I2 {} 

function main(): void {
    let i1: I1 = new C(); 
    let i2: I2 = new C();
    let cc: C = new C();
    arktest.assertEQ("I1:x", cc.m("x"))
    arktest.assertEQ("I2:42", cc.m(42))
    arktest.assertEQ(i1.m("x"), "I1:x");
    arktest.assertEQ(i2.m(42), "I2:42");
}
