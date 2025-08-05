/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: exponentiation operator with other operators
---*/

class IntKeepper {
    x: int = 2;
    y: int = 3;
}

function main(): int {
    let x: int = 0;
    let a: int[] = [2, 3];
    // grouping
    x = (1 + 2)**(2 + 1);
    if (x != 27) return 1;
    // member access
    let v = new IntKeepper();
    x = v.x ** v.y;
    if (x != 8) return 2;
    // index
    x = a[1] ** a[0];
    if (x != 9) return 3;
    // pre/post-fix increment and decrement
    x = 1;
    x = ++x ** ++x;
    if (x != 8) return 4;
    x = 3;
    x = x-- ** x--;
    if (x != 9) return 5;
    // unary -/+
    x = -3;
    x = x ** -x;
    if (x != -27) return 6;
    // ** associativity: right-to-left (4**(3**2))
    x = 4 ** 3 ** 2;
    if (x != 262144) return 7;
    // multiplication has lower precedence
    x = 2*3**2;
    if (x != 18) return 8;
    x = 11+3**2-19;
    if (x != 1) return 9;
    return 0;
}

