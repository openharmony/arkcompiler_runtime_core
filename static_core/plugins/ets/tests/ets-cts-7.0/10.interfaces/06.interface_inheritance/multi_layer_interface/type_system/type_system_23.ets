/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Multiple methods overriding
---*/

interface A {
    draw(x?: string): object;
}

interface B {
    draw(x?: number): string;
}

interface C {
    draw(x: boolean): object;
}

/* more specific return type (covariance)
*  wider parameter type (contravariance)
*  same parameters quantity
*/
class D implements A, B, C {
    draw(x: Any): string {
        if (x === undefined) return 'A'
        else if (x instanceof number) return 'B'
             else if (x instanceof boolean) return 'C'
        
        return 'unknown'
    }
}

function main() {
    let c: D = new D()
    //draw signatures from interfaces
    arktest.assertEQ(c.draw(), "A")
    arktest.assertEQ(c.draw(1.0), "B")
    arktest.assertEQ(c.draw(true), "C")
    //wider parameters range from class D
    arktest.assertEQ(c.draw(1), "unknown")
}