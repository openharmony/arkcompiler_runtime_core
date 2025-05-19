/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Module contains statements and variables which should be initialized
tags: [compile-only, not-a-test]
---*/

// Variables
export let i: int;
export let arr: int[] = [1, 2, 3, 4, 5];
export let str: String = "new str";
export let n: number = 0.1;

// Constants
export const cn: number = 2.5;

// Statements
{
    i = 1;
}

{
    if(i != 0) {
        i++;
    } else {
        assertTrue(false);
    }
    assertEQ( i, 2 );
}

{
    for(let j = 0; j < 5; j++) {
        n += 1;
    }
    assertEQ( n, 5.1 );
}

{
    let j = 0;
    while(j < 5) {
        n += 1;
        j++;
    }
    assertEQ( n, 10.1 );
}

{
    let j = 0;
    do j++; while (j < 5) {
        n += 1;
    }
    assertEQ( n, 11.1 );
}

{
    for(let z of arr) i += z;
    assertEQ( i, 17 );
}

{
    try {
        if( 1 != 2)
            throw new Error();
        assertTrue(false);
    } catch(e) {
        assertEQ( i, 17 );
    }
}

{
    str = "updated str";
}
