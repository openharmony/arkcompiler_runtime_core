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
   Subinterface inherits both overloads. 
   Overload resolution selects based on the argument type.
tags: []
---*/

interface I1 { m(s: string): string { 
    return "I1" 
    } 
} 

interface I2 { 
    m(n: int): string { 
        return "I2" 
    } 
}

interface I3 extends I1, I2 {}

class C implements I3 {}

function main(){
    let i3: I3 = new C() 
    arktest.assertEQ(i3.m("x"), "I1") 
    arktest.assertEQ(i3.m(42), "I2")
}
